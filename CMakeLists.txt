cmake_minimum_required(VERSION 3.20)
project(StubIndex)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 启用测试
enable_testing()

# 添加Google Test
find_package(GTest REQUIRED)

# 源文件
set(SOURCES
    src/stub_entry.h
    src/stub_index.h
    src/stub_index.cpp
    src/stub_parser.h
    src/stub_parser.cpp
    src/psi_node.h
    src/psi_node.cpp
    src/psi_visitor.h
    src/psi_tree_builder.h
    src/psi_tree_builder.cpp
    src/psi_tree_operations.h
    src/psi_tree_operations.cpp
    src/psi_tree_transformer.cpp
)

# 测试文件
set(TEST_SOURCES
    test/stub_entry_test.cpp
    test/function_stub_test.cpp
    test/variable_stub_test.cpp
    test/stub_index_test.cpp
    test/parser_test.cpp
    test/psi_node_test.cpp
    test/psi_tree_builder_test.cpp
    test/psi_tree_operations_test.cpp
    test/psi_tree_transformer_test.cpp
    test/integration_test.cpp
)

# 创建测试可执行文件
add_executable(stub_index_test ${TEST_SOURCES} ${SOURCES})
target_link_libraries(stub_index_test GTest::GTest GTest::Main)
target_include_directories(stub_index_test PRIVATE src)

# 添加测试
add_test(NAME StubIndexTest COMMAND stub_index_test)

# 创建调试程序
add_executable(debug_parser examples/debug_parser.cpp ${SOURCES})
target_link_libraries(debug_parser GTest::GTest GTest::Main)
target_include_directories(debug_parser PRIVATE src)

# 创建简单测试程序
add_executable(simple_test examples/simple_test.cpp ${SOURCES})
target_link_libraries(simple_test GTest::GTest GTest::Main)
target_include_directories(simple_test PRIVATE src)

# 创建演示程序
add_executable(demo examples/demo.cpp ${SOURCES})
target_link_libraries(demo GTest::GTest GTest::Main)
target_include_directories(demo PRIVATE src)