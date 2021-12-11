#include "assembler.h"

void label_map_tp::AddLabel(const std::string& str, const value_tp& val) {
    labels_.insert(std::make_pair(str, val));
}

value_tp label_map_tp::GetValue(const std::string& str) const {
    // User (vAddress, -1) to represent the error case
    if (labels_.find(str) == labels_.end()) {
        // not found
        return value_tp(vAddress, -1);
    }
    else {
        return labels_.at(str);//at方法返回值
    }
}

std::ostream& operator<<(std::ostream& os, const StringType& item) {
    switch (item) {
    case sComment:
        os << "Comment ";
        break;
    case sLabel:
        os << "Label";
        break;
    case sValue:
        os << "Value";
        break;
    case sOpcode:
        os << "Opcode";
        break;
    case sOprand:
        os << "Oprand";
        break;
    default:
        os << "Error";
        break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const ValueType& val) {
    switch (val) {
    case vAddress:
        os << "Address";
        break;
    case vValue:
        os << "Value";
        break;
    default:
        os << "Error";
        break;
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const value_tp& value) {
    if (value.type_ == vValue) {
        os << "[ " << value.type_ << " -- " << value.val_ << " ]";
    }
    else {
        os << "[ " << value.type_ << " -- " << std::hex << "0x" << value.val_ << " ]";
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const label_map_tp& label_map) {
    for (auto item : label_map.labels_) {
        os << "Name: " << item.first << " " << item.second << std::endl;
    }
    return os;
}
//翻译立即数字符串
int RecognizeNumberValue(std::string s) {
    int num = 0;
    if (s[0] == 'x' || s[0] == 'X') {
        s.erase(0, 1);
        while (s.size() < 4) {
            s = "0" + s;
        }
        for (int i = 0; i < 4; i++) {
            num += CharToDec(s[i]) * pow(16.0, 3 - i);
        }
    }
    else if (s[0] == '#') {
        s.erase(0, 1);
        num = stoi(s);
    }
    else if (s[0] == 'B' || s[0] == 'b') {
        s.erase(0, 1);
        if (s[0] == '-') {
            s.erase(0, 1);
            s = ConvertBin2Hex(s);
            s = "X" + s;
            num = -1 * RecognizeNumberValue(s);
        }
        else {
            s = ConvertBin2Hex(s);
            s = "X" + s;
            num = RecognizeNumberValue(s);
        }
    }
    else {
        num = stoi(s);
    }
    return num;
}
//将数字转换为16位二进制字符串
std::string NumberToAssemble(const int& number) {
    int num = number;
    std::string result = "";
    /*
    while (num != 0) {
        result = std::to_string(num % 2) + result;
        num = num / 2;
    }
    while (result.size() < 16) {
        result = "0" + result;
    }*/
    result = buma(num, 16);
    return result;
}
//将数字转换为16位二进制字符串
std::string NumberToAssemble(const std::string& number) {
    // You might use `RecognizeNumberValue` in this function
    int num = RecognizeNumberValue(number);
    /*
    std::string result = "";
    while (num != 0) {
        result = std::to_string(num % 2) + result;
        num = num / 2;
    }
    while (result.size() < 16) {
        result = "0" + result;
    }*/
    std::string result = "";
    result = buma(num, 16);
    return result;
}
//将二进制字符串转换为十六进制字符串
std::string ConvertBin2Hex(std::string bin) {
    while (bin.size() < 16) {//将bin补位16位字符窜
        bin = "0" + bin;
    }
    int a[4] = {};
    for (int i = 0; i < 4; i++) {
        a[i] = 0;
    }
    for (int i = 0; i < 4; i++) {
        for (int j = 4 * i; j < 4 * i + 4; j++) {
            a[i] += (bin[j] - '0') * pow(2, 4 * i + 3 - j);
        }
    }
    std::string result = "";
    for (int i = 0; i < 4; i++) {
        result += DecToChar(a[i]);
    }
    return result;
}
//翻译操作数，包括寄存器和标签等
std::string assembler::TranslateOprand(int current_address, std::string str, int opcode_length) {
	// Translate the oprand
	str = Trim(str);
	auto item = label_map.GetValue(str);
	std::string result = "";
	if (!(item.getType() == vAddress && item.getVal() == -1)) {//如果是标签
        if (item.getType() == vAddress || item.getType() == vValue ){//要改吧……？？？
            int address_minus = item.getVal() - current_address - 1;
            result = buma(address_minus, opcode_length);
            return result;
        }
	}
	if (str[0] == 'R') {//如果是寄存器
		// str is a register
		// TO BE DONE
		int r_num = str[1] - '0';
		while (r_num != 0) {
			result = std::to_string(r_num % 2) + result;
			r_num = r_num / 2;
		}
		while (result.size() < opcode_length) {
			result = "0" + result;
		}
	}
	else {//如果是立即数,将其转为二进制补码
        int num = 0;
        num = RecognizeNumberValue(str);
        result = buma(num, opcode_length);
	}
	return result;
}

// TODO: add error line index
int assembler::assemble(std::string input_filename, std::string output_filename) {
    //存储原始字符串
    std::vector<std::string> file_content;//保存文件的代码内容
    std::vector<std::string> origin_file;//保存原有文件
    // store the tag for line
    std::vector<LineStatusType> file_tag;//保存每一行的tag
    std::vector<std::string> file_comment;//保存注释
    std::vector<int> file_address;//记录每一行代码在翻译后的文件里的起始地址
    int orig_address = -1;//.ORIG后的起始地址
    std::string line;

    std::ifstream input_file(input_filename);

    if (input_file.is_open()) {
        // Scan #0:
        // Read file
        // Store comments
        //第一次扫描，读取文件，并存储注释、代码和原始文件
        while (std::getline(input_file, line)) {
            // 删除这一行开头和结尾的空白
            line = Trim(line);
            if (line.size() == 0) {//如果是空行，重新寻找
                continue;
            }
            std::string origin_line = line;//保存原有行
            //把line转为大写状态
            line = LowToUp(line);
            // 保存注释
            auto comment_position = line.find(";");
            //如果这一行没有注释
            if (comment_position == std::string::npos) {
                file_content.push_back(line);//直接加入这一行的代码内容
                origin_file.push_back(origin_line);//加入原有行
                file_tag.push_back(lPending);//表明 只有代码,标记为待处理
                file_comment.push_back("");//这一行无注释
                file_address.push_back(-1);
                continue;
            }
            else {
                //分割代码内容和字符串
                std::string comment_str = "";
                std::string content_str = "";
                content_str = line.substr(0, comment_position);
                comment_str = line.substr(comment_position + 1);
                //删除空格
                comment_str = Trim(comment_str);
                content_str = Trim(content_str);
                std::cout << "content_str = " << content_str << std::endl;
                std::cout << "comment_str = " << comment_str << std::endl;
                //存储代码内容和注释
                file_content.push_back(content_str);
                origin_file.push_back(origin_line);
                file_comment.push_back(comment_str);
                if (content_str.size() == 0) {
                    //表明这一行是注释
                    file_tag.push_back(lComment);
                }
                else {//表明这一行既有代码又有注释
                    file_tag.push_back(lPending);
                }
                file_address.push_back(-1);
            }
        }
    }
    else {//文件打开失败
        std::cout << "Unable to open file" << std::endl;
        // @ Input file read error
        return -1;
    }

    // Scan #1:
    // Scan for the .ORIG & .END pseudo code
    // Scan for jump label, value label, line comments
    //再一次扫描，寻找.ORIG  .END伪代码，以及jump label, value label, line comments
    int line_address = -1;//这一行的内存地址
    for (int line_index = 0; line_index < file_content.size(); ++line_index) {//line_index为行号，从开始计数
        //表明这一行是注释，跳过处理
        if (file_tag[line_index] == lComment) {
            continue;
        }
        auto line = file_content[line_index];//获取这一行的代码部分

        // 如果是伪指令
        //寻找.ORIG & .END指令
        if (line[0] == '.') {
            file_tag[line_index] = lPseudo;//改变这一行的标签为伪指令
            auto line_stringstream = std::istringstream(line);//类用于执行C++风格的串流的输入操作，逐个输入字符串
            std::string pseudo_command;
            line_stringstream >> pseudo_command;

            if (pseudo_command == ".ORIG") {
                // .ORIG
                std::string orig_value;
                line_stringstream >> orig_value;//获取地址值
                std::cout << "orig_value = " << orig_value << std::endl;//输出地址（debug用
                orig_address = RecognizeNumberValue(orig_value);//将字符串转为数字
                std::cout << "orig_address = " << orig_address << std::endl;//输出地址（debug用
                if (orig_address == std::numeric_limits<int>::max()) {//错误的内存地址
                    return -2;
                }
                file_address[line_index] = -1;
                line_address = orig_address;
            }
            else if (pseudo_command == ".END") {//如果找到.END
                file_address[line_index] = -1;
            }
            else if (pseudo_command == ".STRINGZ") {
                file_address[line_index] = line_address;//记录其在文件中地址的开始
                std::string word;//读入STRINGZ后的字符串
                line_stringstream >> word;
                if (word[0] != '\"' || word[word.size() - 1] != '\"') {//错误的字符串格式,即缺少引号
                    // @ Error String format error
                    return -6;
                }
                std::cout << "word = " << word << std::endl;//输出word(debug
                auto num_temp = word.size() - 1;//获取字符串的大小
                line_address += num_temp;//开辟一段空间
            }
            else if (pseudo_command == ".FILL") {
                // TO BE DONE
                file_address[line_index] = line_address;//记录其在文件中地址的开始
                line_address++;
            }
            else if (pseudo_command == ".BLKW") {
                // TO BE DONE
                file_address[line_index] = line_address;
                std::string word;//读取之后的数字
                line_stringstream >> word;
                line_address += stoi(word, 0, 10);//将word转成数字
            }
            else {
                // @ Error Unknown Pseudo command
                return -100;
            }
            continue;
        }
        //如果是伪地址，已经返回了
        //程序不以.ORIG开头，错误！
        if (line_address == -1) {
            // @ Error Program begins before .ORIG
            return -3;
        }
        //如果不是伪指令，记录地址
        file_address[line_index] = line_address;
        line_address++;

        //获取第一个字
        auto line_stringstream = std::stringstream(line);
        std::string word;
        line_stringstream >> word;
        //如果是操作符
        if (IsLC3Command(word) != -1 || IsLC3TrapRoutine(word) != -1) {
            // * This is an operation line
            // TO BE DONE
            file_tag[line_index] = lOperation;//记录状态
            continue;
        }
        //操作完直接continue
        // 如果遇到标签
        //存储标签
        auto label_name = word;//获得标签
        line_stringstream >> word;//获取第二个单词
        if (IsLC3Command(word) != -1 || IsLC3TrapRoutine(word) != -1 || word == "") {
            // a label used for jump/branch
            // TO BE DONE
            file_tag[line_index] = lOperation;//记录状态
            label_map.AddLabel(label_name, value_tp(vAddress, line_address - 1));//存到map表中
        }
        //是伪指令
        else {
            file_tag[line_index] = lPseudo;
            if (word == ".FILL") {
                line_stringstream >> word;
                auto num_temp = RecognizeNumberValue(word);
                if (num_temp == std::numeric_limits<int>::max()) {
                    // @ Error Invalid Number input @ FILL
                    return -4;
                }
                if (num_temp > 65535 || num_temp < -65536) {
                    // @ Error Too large or too small value  @ FILL
                    return -5;
                }
                label_map.AddLabel(label_name, value_tp(vAddress, line_address - 1));
            }
            if (word == ".BLKW") {
                // modify label map
                // modify line address
                // TO BE DONE
                line_stringstream >> word;
                auto num_temp = stoi(word, 0, 10);
                label_map.AddLabel(label_name, value_tp(vValue, line_address - 1));
                line_address += num_temp - 1;
            }
            if (word == ".STRINGZ") {
                // modify label map
                // modify line address
                // TO BE DONE
                line_stringstream >> word;
                //错误的字符串格式,即缺少引号
                if (word[0] != '\"' || word[word.size() - 1] != '\"') {
                   // @ Error String format error
                    return -6;
                }
                label_map.AddLabel(label_name, value_tp(vValue, line_address - 1));
                auto num_temp = word.size() - 2;//获取字符串的大小
                line_address += num_temp;
            }
        }
    }
    //std::cout << "进入debug模式" << std::endl;
    if (gIsDebugMode) {
        // Some debug information
        std::cout << std::endl;
        std::cout << "Label Map: " << std::endl;
        std::cout << label_map << std::endl;

        for (auto index = 0; index < file_content.size(); ++index) {
            std::cout << std::hex << file_address[index] << " ";
            std::cout << file_content[index] << std::endl;
        }
    }

    // Scan #2:
    // Translate

    // Check output file
    //获取output_filename
    if (output_filename == "") {
        output_filename = input_filename;
        if (output_filename.find(".") == std::string::npos) {
            output_filename = output_filename + ".asm";
        }
        else {
            output_filename = output_filename.substr(0, output_filename.rfind("."));
            output_filename = output_filename + ".asm";
        }
    }

    std::ofstream output_file;
    // Create the output file
    output_file.open(output_filename);
    if (!output_file) {
        // @ Error at output file
        return -20;
    }

    for (int line_index = 0; line_index < file_content.size(); ++line_index) {
        //这一行不用翻译
        if (file_address[line_index] == -1 || file_tag[line_index] == lComment) {
            continue;
        }

        auto line = file_content[line_index];
        auto line_stringstream = std::stringstream(line);

        if (gIsDebugMode)
            output_file << std::hex << file_address[line_index] << ": ";
        //如果是伪指令
        if (file_tag[line_index] == lPseudo) {
            // Translate pseudo command
            std::string word;
            line_stringstream >> word;
            //说明有标签，重新取下一个数
            if (word[0] != '.') {
                // Fetch the second word
                // Eliminate the label
                line_stringstream >> word;
            }

            if (word == ".FILL") {
                std::string number_str;
                line_stringstream >> number_str;
                auto output_line = NumberToAssemble(number_str);
                if (gIsHexMode)
                    output_line = ConvertBin2Hex(output_line);
                output_file << output_line << std::endl;
            }
            else if (word == ".BLKW") {
                // Fill 0 here
                // TO BE DONE
                std::string number_str;
                line_stringstream >> number_str;
                std::string output_line = "0000000000000000";
                if(gIsHexMode)
                    output_line = ConvertBin2Hex(output_line);
                for (int i = 0; i < stoi(number_str, 0, 10); i++) {
                    output_file << output_line << std::endl;
                }
            }
            else if (word == ".STRINGZ") {
                // Fill string here
                // TO BE DONE
                int address_add = 1;
                std::string temp_str;
                line_stringstream >> temp_str;
                for (int i = 1; i < temp_str.size() - 1; i++) {
                    int ascll_code = (int)temp_str[i];
                    std::string temp_code = NumberToAssemble(ascll_code);
                    if (gIsHexMode)
                        temp_code = ConvertBin2Hex(temp_code);
                    output_file << temp_code << std::endl;
                }
                std::string output_line = "0000000000000000";
                if (gIsHexMode)
                    output_line = ConvertBin2Hex(output_line);
                output_file << output_line << std::endl;
            }

            continue;
        }
        //如果是操作指令
        if (file_tag[line_index] == lOperation) {
            std::string word;
            line_stringstream >> word;
            //说明有标签，再次读入下一个
            if (IsLC3Command(word) == -1 && IsLC3TrapRoutine(word) == -1) {
                // Eliminate the label
                line_stringstream >> word;
            }

            std::string result_line = "";
            auto command_tag = IsLC3Command(word);
            auto parameter_str = line.substr(line.find(word) + word.size());//获取操作数
            parameter_str = Trim(parameter_str);

            //将parameter_str中的逗号转成空格，便于分割
            for (int i = 0; i < parameter_str.size(); i++) {
                if (parameter_str[i] == ',') {
                    parameter_str[i] = ' ';
                }
            }

            auto current_address = file_address[line_index];

            std::vector<std::string> parameter_list;
            auto parameter_stream = std::stringstream(parameter_str);
            //输入操作符的所有参数
            while (parameter_stream >> word) {
                parameter_list.push_back(word);
            }
            auto parameter_list_size = parameter_list.size();
            if (command_tag != -1) {
                // This is a LC3 command
                switch (command_tag) {
                case 0:
                    // "ADD"
                    result_line += "0001";
                    if (parameter_list_size != 3) {
                        // @ Error parameter numbers
                        return -30;
                    }
                    result_line += TranslateOprand(current_address, parameter_list[0]);
                    result_line += TranslateOprand(current_address, parameter_list[1]);
                    //第三个参数是寄存器
                    if (parameter_list[2][0] == 'R') {
                        // The third parameter is a register
                        result_line += "000";
                        result_line += TranslateOprand(current_address, parameter_list[2]);
                    }
                    //如果是立即数
                    else {
                        // The third parameter is an immediate number
                        result_line += "1";
                        // std::cout << "hi " << parameter_list[2] << std::endl;
                        result_line += TranslateOprand(current_address, parameter_list[2], 5);
                    }
                    break;
                case 1:
                    // "AND"
                    result_line += "0101";
                    if (parameter_list_size != 3) {
                        // @ Error parameter numbers
                        return -30;
                    }
                    result_line += TranslateOprand(current_address, parameter_list[0]);
                    result_line += TranslateOprand(current_address, parameter_list[1]);
                    //第三个参数是寄存器
                    if (parameter_list[2][0] == 'R') {
                        // The third parameter is a register
                        result_line += "000";
                        result_line += TranslateOprand(current_address, parameter_list[2]);
                    }
                    //如果是立即数
                    else {
                        // The third parameter is an immediate number
                        result_line += "1";
                        // std::cout << "hi " << parameter_list[2] << std::endl;
                        result_line += TranslateOprand(current_address, parameter_list[2], 5);
                    }
                    break;
                case 2:
                    // "BR"
                    // TO BE DONE
                    result_line += "0000000";
                    if (parameter_list_size != 1) {
                        // @ Error parameter numbers
                        return -30;
                    }
                    result_line += TranslateOprand(current_address, parameter_list[0], 9);
                    break;
                case 3:
                    // "BRN"
                    // TO BE DONE
                    result_line += "0000100";
                    if (parameter_list_size != 1) {
                        // @ Error parameter numbers
                        return -30;
                    }
                    result_line += TranslateOprand(current_address, parameter_list[0], 9);
                    break;
                case 4:
                    // "BRZ"
                    // TO BE DONE
                    result_line += "0000010";
                    if (parameter_list_size != 1) {
                        // @ Error parameter numbers
                        return -30;
                    }
                    result_line += TranslateOprand(current_address, parameter_list[0], 9);
                    break;
                case 5:
                    // "BRP"
                    // TO BE DONE
                    result_line += "0000001";
                    if (parameter_list_size != 1) {
                        // @ Error parameter numbers
                        return -30;
                    }
                    result_line += TranslateOprand(current_address, parameter_list[0], 9);
                    break;
                case 6:
                    // "BRNZ"
                    // TO BE DONE
                    result_line += "0000110";
                    if (parameter_list_size != 1) {
                        // @ Error parameter numbers
                        return -30;
                    }
                    result_line += TranslateOprand(current_address, parameter_list[0], 9);
                    break;
                case 7:
                    // "BRNP"
                    // TO BE DONE
                    result_line += "0000101";
                    if (parameter_list_size != 1) {
                        // @ Error parameter numbers
                        return -30;
                    }
                    result_line += TranslateOprand(current_address, parameter_list[0], 9);
                    break;
                case 8:
                    // "BRZP"
                    // TO BE DONE
                    result_line += "0000011";
                    if (parameter_list_size != 1) {
                        // @ Error parameter numbers
                        return -30;
                    }
                    result_line += TranslateOprand(current_address, parameter_list[0], 9);
                    break;
                case 9:
                    // "BRNZP"
                    result_line += "0000111";
                    if (parameter_list_size != 1) {
                        // @ Error parameter numbers
                        return -30;
                    }
                    result_line += TranslateOprand(current_address, parameter_list[0], 9);
                    break;
                case 10:
                    // "JMP"
                    // TO BE DONE
                    result_line += "1100000";
                    if (parameter_list_size != 1) {
                        // @ Error parameter numbers
                        return -30;
                    }
                    result_line += TranslateOprand(current_address, parameter_list[0]);
                    result_line += "000000";
                    break;
                case 11:
                    // "JSR"
                    // TO BE DONE
                    result_line += "01001";
                    if (parameter_list_size != 1) {
                        // @ Error parameter numbers
                        return -30;
                    }
                    result_line += TranslateOprand(current_address, parameter_list[0],11);
                    break;
                case 12:
                    // "JSRR"
                    // TO BE DONE
                    result_line += "0100000";
                    if (parameter_list_size != 1) {
                        // @ Error parameter numbers
                        return -30;
                    }
                    result_line += TranslateOprand(current_address, parameter_list[0]);
                    result_line += "000000";
                    break;
                case 13:
                    // "LD"
                    result_line += "0010";
                    if (parameter_list_size != 2) {
                        // @ Error parameter numbers
                        return -30;
                    }
                    result_line += TranslateOprand(current_address, parameter_list[0]);
                    result_line += TranslateOprand(current_address, parameter_list[1],9);
                    break;
                case 14:
                    // "LDI"
                    result_line += "1010";
                    if (parameter_list_size != 2) {
                        // @ Error parameter numbers
                        return -30;
                    }
                    result_line += TranslateOprand(current_address, parameter_list[0]);
                    result_line += TranslateOprand(current_address, parameter_list[1], 9);
                    break;
                case 15:
                    // "LDR"
                    result_line += "0110";
                    if (parameter_list_size != 3) {
                        // @ Error parameter numbers
                        return -30;
                    }
                    result_line += TranslateOprand(current_address, parameter_list[0]);
                    result_line += TranslateOprand(current_address, parameter_list[1]);
                    result_line += TranslateOprand(current_address, parameter_list[2],6);
                    break;
                case 16:
                    // "LEA"
                    result_line += "1110";
                    if (parameter_list_size != 2) {
                        // @ Error parameter numbers
                        return -30;
                    }
                    result_line += TranslateOprand(current_address, parameter_list[0]);
                    result_line += TranslateOprand(current_address, parameter_list[1], 9);
                    break;
                case 17:
                    // "NOT"
                    result_line += "1001";
                    if (parameter_list_size != 2) {
                        // @ Error parameter numbers
                        return -30;
                    }
                    result_line += TranslateOprand(current_address, parameter_list[0]);
                    result_line += TranslateOprand(current_address, parameter_list[1]);
                    result_line += "111111";
                    break;
                case 18:
                    // RET
                    result_line += "1100000111000000";
                    if (parameter_list_size != 0) {
                        // @ Error parameter numbers
                        return -30;
                    }
                    break;
                case 19:
                    // RTI
                    // TO BE DONE
                    result_line += "1000000000000000";
                    if (parameter_list_size != 0) {
                        // @ Error parameter numbers
                        return -30;
                    }
                    break;
                case 20:
                    // ST
                    result_line += "0011";
                    if (parameter_list_size != 2) {
                        // @ Error parameter numbers
                        return -30;
                    }
                    result_line += TranslateOprand(current_address, parameter_list[0]);
                    result_line += TranslateOprand(current_address, parameter_list[1], 9);
                    break;
                case 21:
                    // STI
                    result_line += "1011";
                    if (parameter_list_size != 2) {
                        // @ Error parameter numbers
                        return -30;
                    }
                    result_line += TranslateOprand(current_address, parameter_list[0]);
                    result_line += TranslateOprand(current_address, parameter_list[1], 9);
                    break;
                case 22:
                    // STR
                    result_line += "0111";
                    if (parameter_list_size != 3) {
                        // @ Error parameter numbers
                        return -30;
                    }
                    result_line += TranslateOprand(current_address, parameter_list[0]);
                    result_line += TranslateOprand(current_address, parameter_list[1]);
                    result_line += TranslateOprand(current_address, parameter_list[2],6);
                    break;
                case 23:
                    // TRAP
                    result_line += "11110000";
                    if (parameter_list_size != 1) {
                        // @ Error parameter numbers
                        return -30;
                    }
                    result_line += TranslateOprand(current_address, parameter_list[0],8);
                    break;
                default:
                    // Unknown opcode
                    // @ Error
                    break;
                }
            }
            else {
                // This is a trap routine
                command_tag = IsLC3TrapRoutine(word);
                switch (command_tag) {
                case 0:
                    // x20
                    result_line += "1111000000100000";
                    break;
                case 1:
                    // x21
                    result_line += "1111000000100001";
                    break;
                case 2:
                    // x22
                    result_line += "1111000000100010";
                    break;
                case 3:
                    // x23
                    result_line += "1111000000100011";
                    break;
                case 4:
                    // x24
                    result_line += "1111000000100100";
                    break;
                case 5:
                    // x25
                    result_line += "1111000000100101";
                    break;
                default:
                    // @ Error Unknown command
                    return -50;
                }
            }

            if (gIsHexMode)
                result_line = ConvertBin2Hex(result_line);
            output_file << result_line << std::endl;
        }
    }

    // Close the output file
    output_file.close();
    // OK flag
    return 0;
}
