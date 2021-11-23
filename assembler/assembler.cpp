#include "assembler.h"

void label_map_tp::AddLabel(const std::string& str, const value_tp& val) {
    labels_.insert(std::make_pair(str, val));
}

value_tp label_map_tp::GetValue(const std::string& str) const {
    /**
    * @description: Get value of the label
    * @param {string} &str
    * @return {*}
    */
    // User (vAddress, -1) to represent the error case
    /* **************************************
    * ********** TO BE DONE *************** *
    *************************************** */
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

int RecognizeNumberValue(std::string s) {
    /**
    * @description: convert string s into a integer value
    * @param {string} s
    * @return {*}
    */

    /* **************************************
    * ********** TO BE DONE *************** *
    *************************************** */
}

std::string NumberToAssemble(const int& number) {
    /**
    * @description: convert the nubmer to 16-bit machine code
    * @param {int} &number
    * @return {*}
    */

    /* **************************************
    * ********** TO BE DONE *************** *
    *************************************** */
}

std::string NumberToAssemble(const std::string& number) {
    /**
    * @description: convert number to 16-bit machine code
    * @param {string} &number
    * @return {*}
    */

    /* **************************************
    * ********** TO BE DONE *************** *
    *************************************** */
}

std::string ConvertBin2Hex(std::string bin) {
    /**
    * @description: convert the binary string into hex representation
    * @param {string} bin
    * @return {*}
    */

    /* **************************************
    * ********** TO BE DONE *************** *
    *************************************** */
}

std::string assembler::TranslateOprand(int current_address, std::string str, int opcode_length) {
    /**
    * @description: convert oprand to machine code
    * @param {int} current_address : the address of current line
    * @param {string} str : the given string to represent oprand
    * @param {int} opcode_length : length limit of the current oprand
    * @return {*}
    */
    str = Trim(str);
    auto item = label_map.GetValue(str);
    if (!(item.getType() == vAddress && item.getVal() == -1)) {
        // str is a label

        /* **************************************
        * ********** TO BE DONE *************** *
        *************************************** */
    }
    if (str[0] == 'R') {
        // str is a register

        /* **************************************
        * ********** TO BE DONE *************** *
        *************************************** */
    }
    else {
        // str is an immediate number

        /* **************************************
        * ********** TO BE DONE *************** *
        *************************************** */
    }
}

int assembler::assemble(std::string input_filename, std::string output_filename) {
    // assemble main function
    // parse program

    // store the original string
    std::vector<std::string> file_content;
    std::vector<std::string> origin_file;
    // store the tag for line
    std::vector<LineStatusType> file_tag;
    std::vector<std::string> file_comment;
    std::vector<int> file_address;
    int orig_address = -1;
    std::string line;

    std::ifstream input_file(input_filename);

    if (input_file.is_open()) {
        // Scan #0:
        // Read file
        // Store comments
        while (std::getline(input_file, line)) {
            // Remove the leading and trailing whitespace
            line = Trim(line);
            if (line.size() == 0) {
                // Empty line
                continue;
            }
            std::string origin_line = line;
            // Convert line into upper case
            for (auto& c : line) {
                c = toupper(c);
            }
            // Store comment here
            auto comment_position = line.find(";");
            if (comment_position == std::string::npos) {
                // No comments here

                /* **************************************
                * ********** TO BE DONE *************** *
                *************************************** */
                continue;
            }
            else {
                auto comment_str = line.substr(comment_position);
                auto content_str = line.substr(0, comment_position);
                // Delete the leading whitespace and the trailing whitespace
                comment_str = Trim(comment_str);
                content_str = Trim(content_str);
                // Store content and comment separately
                file_content.push_back(content_str);
                origin_file.push_back(origin_line);
                file_comment.push_back(comment_str);
                if (content_str.size() == 0) {
                    // The whole line is a comment
                    file_tag.push_back(lComment);
                }
                else {
                    file_tag.push_back(lPending);
                }
                file_address.push_back(-1);
            }
        }
    }
    else {
        std::cout << "Unable to open file" << std::endl;
        // @ Input file read error
        return -1;
    }

    // Scan #1:
    // Scan for the .ORIG & .END pseudo code
    // Scan for jump label, value label, line comments
    int line_address = -1;
    for (int line_index = 0; line_index < file_content.size(); ++line_index) {
        if (file_tag[line_index] == lComment) {
            // This line is comment
            continue;
        }

        auto line = file_content[line_index];

        // * Pseudo Command
        if (line[0] == '.') {
            file_tag[line_index] = lPseudo;
            // This line is a pseudo instruction
            auto line_stringstream = std::istringstream(line);
            std::string pseudo_command;
            line_stringstream >> pseudo_command;

            if (pseudo_command == ".ORIG") {
                // .ORIG
                std::string orig_value;
                line_stringstream >> orig_value;
                orig_address = RecognizeNumberValue(orig_value);
                if (orig_address == std::numeric_limits<int>::max()) {
                    // @ Error address
                    return -2;
                }
                file_address[line_index] = -1;
                line_address = orig_address;
            }
            else if (pseudo_command == ".END") {
                // .END

                /* **************************************
                * ********** TO BE DONE *************** *
                *************************************** */
            }
            else if (pseudo_command == ".STRINGZ") {
                file_address[line_index] = line_address;
                std::string word;
                line_stringstream >> word;
                if (word[0] != '\"' || word[word.size() - 1] != '\"') {
                    // @ Error String format error
                    return -6;
                }
                auto num_temp = word.size() - 1;
                line_address += num_temp;
            }
            else if (pseudo_command == ".FILL") {
                /* **************************************
                * ********** TO BE DONE *************** *
                *************************************** */
            }
            else if (pseudo_command == ".BLKW") {
                std::string word;
                line_stringstream >> word;
                auto num_temp = RecognizeNumberValue(word);
                if (num_temp == std::numeric_limits<int>::max()) {
                    // @ Error Invalid Number input @ BLKW
                    return -6;
                }
                if (num_temp > 65535 || num_temp < -65536) {
                    // @ Error Too large or too small value @ BLKW
                    return -7;
                }
                line_address += num_temp - 1;
            }
            else {
                // @ Error Unknown Pseudo command
                return -100;
            }

            continue;
        }

        if (line_address == -1) {
            // @ Error Program begins before .ORIG
            return -3;
        }

        file_address[line_index] = line_address;
        line_address++;

        // Split the first word in the line
        auto line_stringstream = std::stringstream(line);
        std::string word;
        line_stringstream >> word;
        if (IsLC3Command(word) != -1 || IsLC3TrapRoutine(word) != -1) {
            // * This is an operation line
            file_tag[line_index] = lOperation;
            continue;
        }

        // * Label
        // Store the name of the label
        auto label_name = word;
        // Split the second word in the line
        line_stringstream >> word;
        if (IsLC3Command(word) != -1 || IsLC3TrapRoutine(word) != -1 || word == "") {
            file_tag[line_index] = lOperation;
            label_map.AddLabel(label_name, value_tp(vAddress, file_address[line_index]));
        }
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
                /* **************************************
                * ********** TO BE DONE *************** *
                *************************************** */
            }
            if (word == ".STRINGZ") {
                /* **************************************
                * ********** TO BE DONE *************** *
                *************************************** */
            }
        }
    }

    // Scan #2:
    // Translate

    // Check output file
    if (output_filename == "") {
        output_filename = input_filename;
        if (output_filename.find(".") == std::string::npos) {
            output_filename = output_filename + ".asm";
        }
        else {
            output_filename = output_filename.substr(0, output_filename.find("."));
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
        if (file_address[line_index] == -1 || file_tag[line_index] == lComment) {
            // * This line is not necessary to be translated
            continue;
        }

        auto line = file_content[line_index];
        auto line_stringstream = std::stringstream(line);

        if (gIsDebugMode)
            output_file << std::hex << file_address[line_index] << ": ";
        if (file_tag[line_index] == lPseudo) {
            // Translate pseudo command
            std::string word;
            line_stringstream >> word;
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
                /* **************************************
                * ********** TO BE DONE *************** *
                *************************************** */
            }
            else if (word == ".STRINGZ") {
                /* **************************************
                * ********** TO BE DONE *************** *
                *************************************** */
            }

            continue;
        }

        if (file_tag[line_index] == lOperation) {
            std::string word;
            line_stringstream >> word;
            if (IsLC3Command(word) == -1 && IsLC3TrapRoutine(word) == -1) {
                // Eliminate the label
                line_stringstream >> word;
            }

            std::string result_line = "";
            auto command_tag = IsLC3Command(word);
            auto parameter_str = line.substr(line.find(word) + word.size());
            parameter_str = Trim(parameter_str);
            for (auto& c : parameter_str) {
                if (c == ',')
                    c = ' ';
            }

            auto current_address = file_address[line_index];

            std::vector<std::string> parameter_list;
            auto parameter_stream = std::stringstream(parameter_str);
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
                    if (parameter_list[2][0] == 'R') {
                        // The third parameter is a register
                        result_line += "000";
                        result_line += TranslateOprand(current_address, parameter_list[2]);
                    }
                    else {
                        // The third parameter is an immediate number
                        result_line += "1";
                        result_line += TranslateOprand(current_address, parameter_list[2], 5);
                    }
                    break;
                case 1:
                    // "AND"

                    /* **************************************
                    * ********** TO BE DONE *************** *
                    *************************************** */
                case 2:
                    // "BR"

                    /* **************************************
                    * ********** TO BE DONE *************** *
                    *************************************** */
                case 3:
                    // "BRN"

                    /* **************************************
                    * ********** TO BE DONE *************** *
                    *************************************** */
                case 4:
                    // "BRZ"

                    /* **************************************
                    * ********** TO BE DONE *************** *
                    *************************************** */
                case 5:
                    // "BRP"

                    /* **************************************
                    * ********** TO BE DONE *************** *
                    *************************************** */
                case 6:
                    // "BRNZ"

                    /* **************************************
                    * ********** TO BE DONE *************** *
                    *************************************** */
                case 7:
                    // "BRNP"

                    /* **************************************
                    * ********** TO BE DONE *************** *
                    *************************************** */
                case 8:
                    // "BRZP"

                    /* **************************************
                    * ********** TO BE DONE *************** *
                    *************************************** */
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

                    /* **************************************
                    * ********** TO BE DONE *************** *
                    *************************************** */
                case 11:
                    // "JSR"
                    result_line += "01001";
                    if (parameter_list_size != 1) {
                        // @ Error parameter numbers
                        return -30;
                    }
                    result_line += TranslateOprand(current_address, parameter_list[0], 11);
                    break;
                case 12:
                    // "JSRR"

                    /* **************************************
                    * ********** TO BE DONE *************** *
                    *************************************** */
                case 13:
                    // "LD"

                    /* **************************************
                    * ********** TO BE DONE *************** *
                    *************************************** */
                case 14:
                    // "LDI"

                    /* **************************************
                    * ********** TO BE DONE *************** *
                    *************************************** */
                case 15:
                    // "LDR"
                    result_line += "0110";
                    if (parameter_list_size != 3) {
                        // @ Error parameter numbers
                        return -30;
                    }
                    result_line += TranslateOprand(current_address, parameter_list[0]);
                    result_line += TranslateOprand(current_address, parameter_list[1]);
                    result_line += TranslateOprand(current_address, parameter_list[2], 6);
                    break;
                case 16:
                    // "LEA"

                    /* **************************************
                    * ********** TO BE DONE *************** *
                    *************************************** */
                case 17:
                    // "NOT"

                    /* **************************************
                    * ********** TO BE DONE *************** *
                    *************************************** */
                case 18:
                    // RET

                    /* **************************************
                    * ********** TO BE DONE *************** *
                    *************************************** */
                case 19:
                    // RTI

                    /* **************************************
                    * ********** TO BE DONE *************** *
                    *************************************** */
                    break;
                case 20:
                    // ST

                    /* **************************************
                    * ********** TO BE DONE *************** *
                    *************************************** */
                case 21:
                    // STI

                    /* **************************************
                    * ********** TO BE DONE *************** *
                    *************************************** */
                case 22:
                    // STR

                    /* **************************************
                    * ********** TO BE DONE *************** *
                    *************************************** */
                case 23:
                    // TRAP

                    /* **************************************
                    * ********** TO BE DONE *************** *
                    *************************************** */
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

                    /* **************************************
                    * ********** TO BE DONE *************** *
                    *************************************** */
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
