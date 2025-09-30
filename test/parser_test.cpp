#include <gtest/gtest.h>
#include "stub_parser.h"

using namespace stub_index;

TEST(ParserTest, ParseSimpleClass) {
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

    EXPECT_GE(result.size(), 1);

    bool found_class = false;
    for (const auto& entry : result.getEntries()) {
        if (entry->getType() == StubType::CLASS && entry->getName() == "MyClass") {
            found_class = true;
            break;
        }
    }
    EXPECT_TRUE(found_class);
}

TEST(ParserTest, ParseStruct) {
    StubParser parser;
    std::string code = R"(
        struct Point {
            int x;
            int y;
        };
    )";

    auto result = parser.parseCode(code);

    EXPECT_GE(result.size(), 1);

    bool found_struct = false;
    for (const auto& entry : result.getEntries()) {
        if (entry->getType() == StubType::CLASS && entry->getName() == "Point") {
            auto class_entry = std::dynamic_pointer_cast<ClassStub>(entry);
            ASSERT_NE(class_entry, nullptr);
            EXPECT_TRUE(class_entry->isStruct());
            found_struct = true;
            break;
        }
    }
    EXPECT_TRUE(found_struct);
}

TEST(ParserTest, ParseFunctionDeclaration) {
    StubParser parser;
    std::string code = R"(
        int add(int a, int b) {
            return a + b;
        }

        void printMessage(const std::string& message);
    )";

    auto result = parser.parseCode(code);

    EXPECT_GE(result.size(), 1);

    bool found_add = false;
    for (const auto& entry : result.getEntries()) {
        if (entry->getName() == "add" && entry->getType() == StubType::FUNCTION) {
            found_add = true;
            auto func_entry = std::dynamic_pointer_cast<FunctionStub>(entry);
            ASSERT_NE(func_entry, nullptr);
            EXPECT_EQ(func_entry->getReturnType(), "int");
            EXPECT_EQ(func_entry->getParameters().size(), 2);
            EXPECT_EQ(func_entry->getParameters()[0].type, "int");
            EXPECT_EQ(func_entry->getParameters()[0].name, "a");
            break;
        }
    }
    EXPECT_TRUE(found_add);
}

TEST(ParserTest, ParseVariableDeclaration) {
    StubParser parser;
    std::string code = R"(
        const int MAX_SIZE = 100;
        static int counter = 0;
        int global_var;
    )";

    auto result = parser.parseCode(code);

    EXPECT_GE(result.size(), 2);

    bool found_const = false;
    bool found_static = false;

    for (const auto& entry : result.getEntries()) {
        if (entry->getType() == StubType::VARIABLE) {
            auto var_entry = std::dynamic_pointer_cast<VariableStub>(entry);
            ASSERT_NE(var_entry, nullptr);

            if (var_entry->getName() == "MAX_SIZE") {
                found_const = true;
                EXPECT_TRUE(var_entry->isConst());
                EXPECT_EQ(var_entry->getVariableType(), "int");
            } else if (var_entry->getName() == "counter") {
                found_static = true;
                EXPECT_TRUE(var_entry->isStatic());
                EXPECT_EQ(var_entry->getVariableType(), "int");
            }
        }
    }

    EXPECT_TRUE(found_const);
    EXPECT_TRUE(found_static);
}

TEST(ParserTest, ParseComplexCode) {
    StubParser parser;
    std::string code = R"(
        #include <iostream>
        #include <string>

        class Database {
        public:
            static const int MAX_CONNECTIONS = 10;

            bool connect(const std::string& url);
            void disconnect();

        private:
            std::string connection_url_;
            bool is_connected_;
        };

        bool Database::connect(const std::string& url) {
            connection_url_ = url;
            is_connected_ = true;
            return true;
        }

        void Database::disconnect() {
            is_connected_ = false;
        }
    )";

    auto result = parser.parseCode(code);

    EXPECT_GE(result.size(), 4);

    bool found_class = false;
    bool found_const_var = false;
    bool found_method = false;

    for (const auto& entry : result.getEntries()) {
        if (entry->getType() == StubType::CLASS && entry->getName() == "Database") {
            found_class = true;
        } else if (entry->getType() == StubType::VARIABLE && entry->getName() == "MAX_CONNECTIONS") {
            found_const_var = true;
            auto var_entry = std::dynamic_pointer_cast<VariableStub>(entry);
            EXPECT_TRUE(var_entry->isStatic());
            EXPECT_TRUE(var_entry->isConst());
        } else if (entry->getType() == StubType::FUNCTION) {
            found_method = true;
        }
    }

    EXPECT_TRUE(found_class);
    EXPECT_TRUE(found_const_var);
    EXPECT_TRUE(found_method);
}

TEST(ParserTest, ParseWithFileSource) {
    StubParser parser;
    std::string code = R"(
        class TestClass {
        public:
            void testMethod();
        };
    )";

    auto result = parser.parseCode(code, "test.cpp");

    EXPECT_GE(result.size(), 1);

    bool found_class = false;
    for (const auto& entry : result.getEntries()) {
        if (entry->getType() == StubType::CLASS && entry->getName() == "TestClass") {
            EXPECT_EQ(entry->getLocation().file_path, "test.cpp");
            found_class = true;
            break;
        }
    }
    EXPECT_TRUE(found_class);
}