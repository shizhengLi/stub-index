#include <gtest/gtest.h>
#include "stub_entry.h"

using namespace stub_index;

TEST(VariableStubTest, CreateVariableStub) {
    SourceLocation loc("test.cpp", 5, 15);
    VariableStub var("myVariable", loc, "int");

    EXPECT_EQ(var.getType(), StubType::VARIABLE);
    EXPECT_EQ(var.getName(), "myVariable");
    EXPECT_EQ(var.getVariableType(), "int");
    EXPECT_FALSE(var.isConst());
    EXPECT_FALSE(var.isStatic());
}

TEST(VariableStubTest, VariableWithDifferentTypes) {
    SourceLocation loc("test.cpp", 10, 1);
    VariableStub var1("counter", loc, "int", true);
    VariableStub var2("PI", loc, "double", true, true);

    EXPECT_TRUE(var1.isConst());
    EXPECT_FALSE(var1.isStatic());

    EXPECT_TRUE(var2.isConst());
    EXPECT_TRUE(var2.isStatic());
    EXPECT_EQ(var2.getVariableType(), "double");
}

TEST(VariableStubTest, VariableToString) {
    SourceLocation loc("test.cpp", 15, 5);
    VariableStub var("MAX_SIZE", loc, "size_t", true, true);

    std::string str = var.toString();
    EXPECT_EQ(str, "Variable const static size_t MAX_SIZE at test.cpp:15");
}