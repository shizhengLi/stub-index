#include <gtest/gtest.h>
#include "stub_entry.h"

using namespace stub_index;

TEST(StubEntryTest, CreateClassStub) {
    SourceLocation loc("test.cpp", 10, 5);
    ClassStub class_stub("MyClass", loc);

    EXPECT_EQ(class_stub.getType(), StubType::CLASS);
    EXPECT_EQ(class_stub.getName(), "MyClass");
    EXPECT_EQ(class_stub.getLocation().file_path, "test.cpp");
    EXPECT_EQ(class_stub.getLocation().line, 10);
    EXPECT_EQ(class_stub.getLocation().column, 5);
    EXPECT_FALSE(class_stub.isStruct());
}

TEST(StubEntryTest, CreateStructStub) {
    SourceLocation loc("test.cpp", 15, 1);
    ClassStub struct_stub("MyStruct", loc, true);

    EXPECT_EQ(struct_stub.getType(), StubType::CLASS);
    EXPECT_EQ(struct_stub.getName(), "MyStruct");
    EXPECT_TRUE(struct_stub.isStruct());
}

TEST(StubEntryTest, ToString) {
    SourceLocation loc("test.cpp", 10, 5);
    ClassStub class_stub("MyClass", loc);

    std::string str = class_stub.toString();
    EXPECT_EQ(str, "Class MyClass at test.cpp:10");
}