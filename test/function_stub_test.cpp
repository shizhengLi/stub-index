#include <gtest/gtest.h>
#include "stub_entry.h"

using namespace stub_index;

TEST(FunctionStubTest, CreateFunctionStub) {
    SourceLocation loc("test.cpp", 20, 10);
    FunctionStub func("myFunction", loc, "int");

    EXPECT_EQ(func.getType(), StubType::FUNCTION);
    EXPECT_EQ(func.getName(), "myFunction");
    EXPECT_EQ(func.getReturnType(), "int");
    EXPECT_EQ(func.getLocation().file_path, "test.cpp");
    EXPECT_EQ(func.getLocation().line, 20);
    EXPECT_TRUE(func.getParameters().empty());
}

TEST(FunctionStubTest, FunctionWithParameters) {
    SourceLocation loc("test.cpp", 25, 5);
    FunctionStub func("calculate", loc, "double");

    func.addParameter("int", "a");
    func.addParameter("float", "b");

    EXPECT_EQ(func.getParameters().size(), 2);
    EXPECT_EQ(func.getParameters()[0].type, "int");
    EXPECT_EQ(func.getParameters()[0].name, "a");
    EXPECT_EQ(func.getParameters()[1].type, "float");
    EXPECT_EQ(func.getParameters()[1].name, "b");
}

TEST(FunctionStubTest, FunctionToString) {
    SourceLocation loc("test.cpp", 30, 1);
    FunctionStub func("add", loc, "int");
    func.addParameter("int", "x");
    func.addParameter("int", "y");

    std::string str = func.toString();
    EXPECT_EQ(str, "Function int add(int x, int y) at test.cpp:30");
}