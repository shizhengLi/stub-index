#include <iostream>
#include <regex>
#include "stub_parser.h"

using namespace stub_index;

int main() {
    StubParser parser;
    std::string code = R"(
        class MyClass {
        public:
            void method();
        private:
            int value;
        };
    )";

    std::cout << "Debug code parsing..." << std::endl;
    std::cout << "Code length: " << code.length() << std::endl;
    std::cout << "Code content: " << std::endl;
    std::cout << code << std::endl;

    // 测试正则表达式
    std::regex class_pattern(
        R"(^(?:class|struct)\s+(\w+)\s*(?::\s*(?:public|private|protected)\s+\w+\s*)?\s*\{)",
        std::regex_constants::multiline | std::regex_constants::icase
    );

    std::cout << "\nTesting regex pattern..." << std::endl;

    std::sregex_iterator it(code.begin(), code.end(), class_pattern);
    std::sregex_iterator end;

    int count = 0;
    for (; it != end; ++it) {
        std::cout << "Match " << ++count << ": " << it->str() << std::endl;
        std::cout << "Position: " << it->position() << std::endl;
    }

    if (count == 0) {
        std::cout << "No matches found. Let's try a simpler pattern..." << std::endl;

        std::regex simple_pattern(R"(class\s+(\w+))", std::regex_constants::multiline);
        std::sregex_iterator it2(code.begin(), code.end(), simple_pattern);

        count = 0;
        for (; it2 != end; ++it2) {
            std::cout << "Simple match " << ++count << ": " << it2->str() << std::endl;
        }
    }

    // 测试解析器
    auto result = parser.parseCode(code);
    std::cout << "\nParser result size: " << result.size() << std::endl;

    return 0;
}