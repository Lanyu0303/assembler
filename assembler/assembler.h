#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

const int kLC3LineLength = 16;

extern bool gIsDebugMode;
extern bool gIsErrorLogMode;
extern bool gIsHexMode;
//LC3命令
const std::vector<std::string> kLC3Commands({
    "ADD",   // 00: "0001" + reg(line[1]) + reg(line[2]) + op(line[3])
    "AND",   // 01: "0101" + reg(line[1]) + reg(line[2]) + op(line[3])
    "BR",    // 02: "0000000" + pcoffset(line[1],9)
    "BRN",   // 03: "0000100" + pcoffset(line[1],9)
    "BRZ",   // 04: "0000010" + pcoffset(line[1],9)
    "BRP",   // 05: "0000001" + pcoffset(line[1],9)
    "BRNZ",  // 06: "0000110" + pcoffset(line[1],9)
    "BRNP",  // 07: "0000101" + pcoffset(line[1],9)
    "BRZP",  // 08: "0000011" + pcoffset(line[1],9)
    "BRNZP", // 09: "0000111" + pcoffset(line[1],9)
    "JMP",   // 10: "1100000" + reg(line[1]) + "000000"
    "JSR",   // 11: "01001" + pcoffset(line[1],11)
    "JSRR",  // 12: "0100000"+reg(line[1])+"000000"
    "LD",    // 13: "0010" + reg(line[1]) + pcoffset(line[2],9)
    "LDI",   // 14: "1010" + reg(line[1]) + pcoffset(line[2],9)
    "LDR",   // 15: "0110" + reg(line[1]) + reg(line[2]) + offset(line[3])
    "LEA",   // 16: "1110" + reg(line[1]) + pcoffset(line[2],9)
    "NOT",   // 17: "1001" + reg(line[1]) + reg(line[2]) + "111111"
    "RET",   // 18: "1100000111000000"
    "RTI",   // 19: "1000000000000000"
    "ST",    // 20: "0011" + reg(line[1]) + pcoffset(line[2],9)
    "STI",   // 21: "1011" + reg(line[1]) + pcoffset(line[2],9)
    "STR",   // 22: "0111" + reg(line[1]) + reg(line[2]) + offset(line[3])
    "TRAP"   // 23: "11110000" + h2b(line[1],8)
    });
//LC3跳转指令
const std::vector<std::string> kLC3TrapRoutine({
    "GETC",  // x20
    "OUT",   // x21
    "PUTS",  // x22
    "IN",    // x23
    "PUTSP", // x24
    "HALT"   // x25
    });

enum StringType { sComment, sLabel, sValue, sOpcode, sOprand, sError };
enum ValueType { vAddress, vValue };
enum LineStatusType { lPending = -1, lComment, lOperation, lPseudo };

typedef std::pair<std::string, StringType> token_tp;
inline void SetDebugMode(bool debug) {
    gIsDebugMode = debug;
}
inline void SetErrorLogMode(bool error) {
    gIsErrorLogMode = error;
}
inline void SetHexMode(bool hex) {
    gIsHexMode = hex;
}

class value_tp {
private:
    ValueType type_;
    int val_;

public:
    value_tp(ValueType type, int val) : type_(type), val_(val) {}
    value_tp(int val) : type_(vValue), val_(val) {}
    value_tp() : type_(ValueType::vValue), val_(0) {}
    ~value_tp() {}

    ValueType getType() const { return type_; }
    int getVal() const { return val_; }
    void setType(ValueType type) { type_ = type; }
    void setVal(int val) { val_ = val; }
    friend std::ostream& operator<<(std::ostream& os, const value_tp& value);
};
//标签的查找表
class label_map_tp {
private:
    std::map<std::string, value_tp> labels_;

public:
    ~label_map_tp() {}
    void AddLabel(const std::string& str, const value_tp& val);
    // void AddLabel(std::string str, std::string val);
    value_tp GetValue(const std::string& str) const;
    friend std::ostream& operator<<(std::ostream& os, const label_map_tp& label_map);
};

// trim from left
inline std::string& LeftTrim(std::string& s, const char* t = " \t\n\r\f\v") {
    // TO BE DONE
    s.erase(0, s.find_first_not_of(t));
    return s;
}

// trim from right
inline std::string& RightTrim(std::string& s, const char* t = " \t\n\r\f\v") {
    // TO BE DONE
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

// trim from left & right
inline std::string& Trim(std::string& s, const char* t = " \t\n\r\f\v") {
    return LeftTrim(RightTrim(s, t), t);
}
//把字符串转为大写
inline std::string& LowToUp(std::string& str) {
    int flag = 0;//用于判断是否是字符串，含有"Abc"这种，不能将小写变大写
    for (int i = 0; i < str.size(); i++) {
        if (str[i] == '"') {
            if (flag == 0) {
                flag = 1;
                continue;
            }
            else if (flag == 1) {
                flag = 0;
                continue;
            }
        }
        if (flag == 1) {
            continue;
        }
        if (str[i] >= 'a' && str[i] <= 'z') {
            str[i] = str[i] - 'a' + 'A';
        }
    }
    return str;
}
//把十进制int转为n位的二进制补码string
inline std::string buma(int num, int opcode_length) {
    std::string result = "";
    if (num >= 0) {
        while (num > 0) {
            result = std::to_string(num % 2) + result;
            num = num / 2;
        }
        while (result.size() < opcode_length) {
            result = "0" + result;
        }
        return result;
    }
    else if(num < 0){
        int ab_num = -1 * num;
        while (ab_num > 0) {
            result = std::to_string(ab_num % 2) + result;
            ab_num = ab_num / 2;
        }
        while (result.size() < opcode_length) {//高位补全
            result = "0" + result;
        }
        //取反
        for (int i = 0; i < result.size(); i++) {
            result[i] = '1' - result[i] + '0';
        }
        //加一
        result[result.size() - 1] = result[result.size() - 1] + '1' - '0';
        for (int i = result.size() - 1; i >= 0; i--) {
            if (i > 0) {
                if (result[i] == '2') {
                    result[i] = '0';
                    result[i - 1] = result[i - 1] + '1' - '0';
                }
            }
            else {
                if (result[i] == '2') {
                    result[i] = '0';
                }
            }
        }
        return result;
    }
}
inline int IsLC3Command(const std::string& str) {
    int index = 0;
    for (auto command : kLC3Commands) {
        if (str == command) {
            return index;
        }
        ++index;
    }
    return -1;
}

inline int IsLC3TrapRoutine(const std::string& str) {
    int index = 0;
    for (auto trap : kLC3TrapRoutine) {
        if (str == trap) {
            return index;
        }
        ++index;
    }
    return -1;
}

inline int CharToDec(const char& ch) {
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    if (ch >= 'A' && ch <= 'F') {
        return ch - 'A' + 10;
    }
    return -1;
}

inline char DecToChar(const int& num) {
    if (num <= 9) {
        return num + '0';
    }
    else {
        return num - 10 + 'A';
    }
    return -1;
}

int RecognizeNumberValue(std::string str);
std::string NumberToAssemble(const int& number);
std::string NumberToAssemble(const std::string& number);

std::string ConvertBin2Hex(std::string bin);

class assembler {
private:
    label_map_tp label_map;
    std::string TranslateOprand(int current_address, std::string str, int opcode_length = 3);//翻译操作数

public:
    int assemble(std::string input_filename, std::string output_filename);
};