#include <iostream>
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

    auto result = parser.parseCode(code);

    std::cout << "Parsed " << result.size() << " entries:" << std::endl;
    for (size_t i = 0; i < result.getEntries().size(); ++i) {
        const auto& entry = result.getEntries()[i];
        std::cout << i << ": " << entry->toString() << std::endl;
    }

    return 0;
}