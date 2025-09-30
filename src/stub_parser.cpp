#include "stub_parser.h"
#include <iostream>

namespace stub_index {

ParseResult StubParser::parseFile(const std::string& file_path, const std::string& content) {
    return parseCode(content, file_path);
}

ParseResult StubParser::parseCode(const std::string& code, const std::string& file_path) {
    ParseResult result;

    if (parse_classes_) {
        parseClass(code, file_path, result);
    }

    if (parse_functions_) {
        parseFunction(code, file_path, result);
    }

    if (parse_variables_) {
        parseVariable(code, file_path, result);
    }

    return result;
}

void StubParser::parseClass(const std::string& code, const std::string& file_path, ParseResult& result) {
    // 匹配类定义的正则表达式
    std::regex class_pattern(R"((class|struct)\s+(\w+)[^{]*\{)");

    std::sregex_iterator it(code.begin(), code.end(), class_pattern);
    std::sregex_iterator end;

    for (; it != end; ++it) {
        std::smatch match = *it;
        std::string match_str = match[0].str();

        std::string keyword = match[1].str();
        std::string class_name = match[2].str();

        // 确定是否是struct
        bool is_struct = (keyword == "struct");

        // 获取行号
        int line = getLineNumber(code, match.position());

        // 创建位置信息
        SourceLocation loc(file_path, line, 1);

        // 创建类Stub条目
        auto class_stub = std::make_shared<ClassStub>(class_name, loc, is_struct);
        result.addEntry(class_stub);
    }
}

void StubParser::parseFunction(const std::string& code, const std::string& file_path, ParseResult& result) {
    // 匹配函数定义的正则表达式 - 简化版本
    std::regex function_pattern(R"((\w+)\s+(\w+)\s*\(([^)]*)\)\s*(?:const\s*)?(?:override\s*)?(?:final\s*)?\s*[;{])");

    std::sregex_iterator it(code.begin(), code.end(), function_pattern);
    std::sregex_iterator end;

    for (; it != end; ++it) {
        std::smatch match = *it;
        std::string match_str = match[0].str();

        // 跳过类定义
        if (match_str.find("class") == 0 || match_str.find("struct") == 0) {
            continue;
        }

        std::string return_type = match[1].str();
        std::string func_name = match[2].str();
        std::string params = match[3].str();

        // 跳过构造函数和析构函数
        if (return_type == "return" || func_name == "return" || return_type == "if" || func_name == "if") {
            continue;
        }

        // 获取行号
        int line = getLineNumber(code, match.position());

        // 创建位置信息
        SourceLocation loc(file_path, line, 1);

        // 创建函数Stub条目
        auto func_stub = std::make_shared<FunctionStub>(func_name, loc, return_type);

        // 简单参数解析
        if (!params.empty()) {
            std::vector<std::string> param_list;
            size_t start = 0;
            size_t end = params.find(',');

            while (end != std::string::npos) {
                param_list.push_back(params.substr(start, end - start));
                start = end + 1;
                end = params.find(',', start);
            }
            param_list.push_back(params.substr(start));

            for (const auto& param : param_list) {
                std::string trimmed_param = param;
                // 去除前后空格
                trimmed_param.erase(0, trimmed_param.find_first_not_of(" \t\n\r"));
                trimmed_param.erase(trimmed_param.find_last_not_of(" \t\n\r") + 1);

                if (!trimmed_param.empty()) {
                    // 简单解析类型和名称
                    size_t space_pos = trimmed_param.find_last_of(" \t");
                    if (space_pos != std::string::npos) {
                        std::string param_type = trimmed_param.substr(0, space_pos);
                        std::string param_name = trimmed_param.substr(space_pos + 1);
                        func_stub->addParameter(param_type, param_name);
                    } else {
                        func_stub->addParameter(trimmed_param, "param");
                    }
                }
            }
        }

        result.addEntry(func_stub);
    }
}

void StubParser::parseVariable(const std::string& code, const std::string& file_path, ParseResult& result) {
    // 匹配变量声明的正则表达式 - 支持const和static组合
    std::regex variable_pattern(R"((const\s+static\s+|static\s+const\s+|const\s+|static\s+)?(\w+)\s+(\w+)\s*[=;])");

    std::sregex_iterator it(code.begin(), code.end(), variable_pattern);
    std::sregex_iterator end;

    for (; it != end; ++it) {
        std::smatch match = *it;
        std::string match_str = match[0].str();

        // 跳过函数定义和类定义
        if (match_str.find('(') != std::string::npos || match_str.find('{') != std::string::npos) {
            continue;
        }

        // 跳过关键字
        std::string keyword = match[1].str();
        std::string var_type = match[2].str();
        std::string var_name = match[3].str();

        // 跳过不是类型的标识符
        if (var_type == "return" || var_type == "if" || var_type == "else" || var_type == "for" || var_type == "while") {
            continue;
        }

        // 检查是否是const
        bool is_const = match_str.find("const") != std::string::npos;

        // 检查是否是static
        bool is_static = match_str.find("static") != std::string::npos;

        // 获取行号
        int line = getLineNumber(code, match.position());

        // 创建位置信息
        SourceLocation loc(file_path, line, 1);

        // 创建变量Stub条目
        auto var_stub = std::make_shared<VariableStub>(var_name, loc, var_type, is_const, is_static);
        result.addEntry(var_stub);
    }
}

std::vector<std::pair<size_t, size_t>> StubParser::findPatternMatches(
    const std::string& code, const std::regex& pattern) {

    std::vector<std::pair<size_t, size_t>> matches;
    std::sregex_iterator it(code.begin(), code.end(), pattern);
    std::sregex_iterator end;

    for (; it != end; ++it) {
        matches.emplace_back(it->position(), it->position() + it->length());
    }

    return matches;
}

int StubParser::getLineNumber(const std::string& code, size_t pos) {
    if (pos >= code.length()) {
        return 1;
    }

    int line = 1;
    for (size_t i = 0; i < pos && i < code.length(); ++i) {
        if (code[i] == '\n') {
            line++;
        }
    }

    return line;
}

}