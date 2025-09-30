#include <gtest/gtest.h>
#include "stub_index.h"

using namespace stub_index;

TEST(StubIndexTest, CreateEmptyIndex) {
    StubIndex index;
    EXPECT_TRUE(index.empty());
    EXPECT_EQ(index.size(), 0);
}

TEST(StubIndexTest, AddClassEntry) {
    StubIndex index;
    SourceLocation loc("test.cpp", 1, 1);
    auto class_stub = std::make_shared<ClassStub>("TestClass", loc);

    index.addEntry(class_stub);

    EXPECT_FALSE(index.empty());
    EXPECT_EQ(index.size(), 1);

    auto results = index.queryByName("TestClass");
    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results.entries[0]->getName(), "TestClass");
}

TEST(StubIndexTest, QueryByType) {
    StubIndex index;
    SourceLocation loc("test.cpp", 1, 1);

    auto class_stub = std::make_shared<ClassStub>("MyClass", loc);
    auto func_stub = std::make_shared<FunctionStub>("myFunction", loc, "int");

    index.addEntry(class_stub);
    index.addEntry(func_stub);

    auto class_results = index.queryByType(StubType::CLASS);
    EXPECT_EQ(class_results.size(), 1);
    EXPECT_EQ(class_results.entries[0]->getName(), "MyClass");

    auto func_results = index.queryByType(StubType::FUNCTION);
    EXPECT_EQ(func_results.size(), 1);
    EXPECT_EQ(func_results.entries[0]->getName(), "myFunction");
}

TEST(StubIndexTest, QueryByFile) {
    StubIndex index;
    SourceLocation loc1("file1.cpp", 1, 1);
    SourceLocation loc2("file2.cpp", 1, 1);

    auto stub1 = std::make_shared<ClassStub>("Class1", loc1);
    auto stub2 = std::make_shared<ClassStub>("Class2", loc2);

    index.addEntry(stub1);
    index.addEntry(stub2);

    auto file1_results = index.queryByFile("file1.cpp");
    EXPECT_EQ(file1_results.size(), 1);
    EXPECT_EQ(file1_results.entries[0]->getName(), "Class1");

    auto file2_results = index.queryByFile("file2.cpp");
    EXPECT_EQ(file2_results.size(), 1);
    EXPECT_EQ(file2_results.entries[0]->getName(), "Class2");
}

TEST(StubIndexTest, ComplexQuery) {
    StubIndex index;
    SourceLocation loc("test.cpp", 1, 1);

    auto class_stub = std::make_shared<ClassStub>("Data", loc);
    auto func_stub = std::make_shared<FunctionStub>("Data", loc, "void");
    auto var_stub = std::make_shared<VariableStub>("config", loc, "int");

    index.addEntry(class_stub);
    index.addEntry(func_stub);
    index.addEntry(var_stub);

    QueryFilter filter(StubType::CLASS, "Data");
    auto results = index.query(filter);
    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results.entries[0]->getType(), StubType::CLASS);
}

TEST(StubIndexTest, ClearIndex) {
    StubIndex index;
    SourceLocation loc("test.cpp", 1, 1);
    auto stub = std::make_shared<ClassStub>("TestClass", loc);

    index.addEntry(stub);
    EXPECT_FALSE(index.empty());

    index.clear();
    EXPECT_TRUE(index.empty());
    EXPECT_EQ(index.size(), 0);
}