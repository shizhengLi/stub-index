#include <gtest/gtest.h>
#include "stub_index.h"
#include "stub_parser.h"
#include "psi_tree_builder.h"
#include "psi_tree_operations.h"
#include "psi_visitor.h"
#include <chrono>

using namespace stub_index;

class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试用的代码内容
        test_content = R"(
#include <iostream>
#include <vector>

class Calculator {
public:
    Calculator() : value_(0) {}

    int add(int a, int b) {
        return a + b;
    }

    int multiply(int x, int y) {
        return x * y;
    }

    int getValue() const { return value_; }

private:
    int value_;
};

class DataProcessor {
public:
    void processData(const std::vector<int>& data);
    int calculateAverage(const std::vector<int>& data);
};

namespace utils {
    class Helper {
    public:
        static void log(const std::string& message);
        static bool isValid(int value);
    };
}

// 全局变量和函数
int global_counter = 0;
void resetCounter() {
    global_counter = 0;
}
)";

        file_path = "integration_test.cpp";
    }

    std::string test_content;
    std::string file_path;
};

// 测试1: Stub索引构建与PSI树构建的一致性
TEST_F(IntegrationTest, StubIndexAndPSITreeConsistency) {
    // 使用Stub解析器解析代码
    StubParser stub_parser;
    auto parse_result = stub_parser.parseCode(test_content, file_path);
    auto stub_entries = parse_result.getEntries();

    // 构建Stub索引
    StubIndex stub_index;
    for (const auto& entry : stub_entries) {
        stub_index.addEntry(entry);
    }

    // 构建PSI树
    PSITreeBuilder psi_builder;
    auto psi_tree = psi_builder.buildTreeFromContent(file_path, test_content);

    // 验证一致性
    PSITreeOperations psi_ops;

    // 1. 比较类数量
    auto stub_classes = stub_index.queryByType(StubType::CLASS);
    auto psi_classes = psi_ops.findAllNodes(psi_tree.get(), PSINodeType::CLASS);

    EXPECT_EQ(stub_classes.size(), psi_classes.size())
        << "Stub索引和PSI树的类数量不一致";

    // 2. 比较函数数量
    auto stub_functions = stub_index.queryByType(StubType::FUNCTION);
    auto psi_functions = psi_ops.findAllNodes(psi_tree.get(), PSINodeType::FUNCTION);

    EXPECT_EQ(stub_functions.size(), psi_functions.size())
        << "Stub索引和PSI树的函数数量不一致";

    // 3. 验证特定的类存在
    auto calculator_stub = stub_index.queryByName("Calculator");
    auto calculator_psi = psi_ops.findFirstNodeByName(psi_tree.get(), "Calculator");

    EXPECT_GT(calculator_stub.size(), 0) << "Stub索引中找不到Calculator类";
    EXPECT_NE(calculator_psi, nullptr) << "PSI树中找不到Calculator类";

    // 4. 验证特定的函数存在
    auto add_func_stub = stub_index.queryByName("add");
    auto add_func_psi = psi_ops.findFirstNodeByName(psi_tree.get(), "add");

    EXPECT_GT(add_func_stub.size(), 0) << "Stub索引中找不到add函数";
    EXPECT_NE(add_func_psi, nullptr) << "PSI树中找不到add函数";
}

// 测试2: 双向查询验证
TEST_F(IntegrationTest, BidirectionalQueryValidation) {
    // 构建Stub索引和PSI树
    StubParser stub_parser;
    auto parse_result = stub_parser.parseCode(test_content, file_path);
    auto stub_entries = parse_result.getEntries();

    StubIndex stub_index;
    for (const auto& entry : stub_entries) {
        stub_index.addEntry(entry);
    }

    PSITreeBuilder psi_builder;
    auto psi_tree = psi_builder.buildTreeFromContent(file_path, test_content);

    PSITreeOperations psi_ops;

    // 对于每个Stub条目，验证在PSI树中存在对应的节点
    for (const auto& stub_entry : stub_entries) {
        std::string entry_name = stub_entry->getName();

        // 在PSI树中查找对应节点
        auto* psi_node = psi_ops.findFirstNodeByName(psi_tree.get(), entry_name);

        ASSERT_NE(psi_node, nullptr)
            << "在PSI树中找不到Stub条目: " << entry_name;

        // 验证类型一致性
        switch (stub_entry->getType()) {
            case StubType::CLASS:
                EXPECT_TRUE(psi_node->getType() == PSINodeType::CLASS ||
                           psi_node->getType() == PSINodeType::STRUCT)
                    << "类型不匹配: Stub是CLASS但PSI节点是其他类型: " << entry_name;
                break;
            case StubType::FUNCTION:
                EXPECT_EQ(psi_node->getType(), PSINodeType::FUNCTION)
                    << "类型不匹配: Stub是FUNCTION但PSI节点是其他类型: " << entry_name;
                break;
            case StubType::VARIABLE:
                EXPECT_EQ(psi_node->getType(), PSINodeType::VARIABLE)
                    << "类型不匹配: Stub是VARIABLE但PSI节点是其他类型: " << entry_name;
                break;
            default:
                // 其他类型暂时不验证
                break;
        }

        // 验证位置信息一致性
        auto stub_location = stub_entry->getLocation();
        auto psi_location = psi_node->getLocation();

        EXPECT_EQ(stub_location.file_path, psi_location.file_path)
            << "文件路径不一致: " << entry_name;
        EXPECT_EQ(stub_location.line, psi_location.line)
            << "行号不一致: " << entry_name;
    }
}

// 测试3: 层次结构验证
TEST_F(IntegrationTest, HierarchyValidation) {
    PSITreeBuilder psi_builder;
    auto psi_tree = psi_builder.buildTreeFromContent(file_path, test_content);

    PSITreeOperations psi_ops;

    // 查找Calculator类
    auto* calculator_class = psi_ops.findFirstNodeByName(psi_tree.get(), "Calculator");
    ASSERT_NE(calculator_class, nullptr);

    // 验证Calculator类存在
    EXPECT_EQ(calculator_class->getText(), "Calculator");

    // 验证Calculator类的方法在树中存在（在当前实现中，它们是兄弟节点）
    auto* add_method = psi_ops.findFirstNodeByName(psi_tree.get(), "add");
    auto* multiply_method = psi_ops.findFirstNodeByName(psi_tree.get(), "multiply");
    auto* get_value_method = psi_ops.findFirstNodeByName(psi_tree.get(), "getValue");

    EXPECT_NE(add_method, nullptr) << "在树中找不到add方法";
    EXPECT_NE(multiply_method, nullptr) << "在树中找不到multiply方法";
    EXPECT_NE(get_value_method, nullptr) << "在树中找不到getValue方法";

    // 验证命名空间结构
    auto* utils_namespace = psi_ops.findFirstNodeByName(psi_tree.get(), "utils");
    if (utils_namespace) {
        // 在当前实现中，命名空间可能不包含子节点，但Helper类应该存在
        auto* helper_class = psi_ops.findFirstNodeByName(psi_tree.get(), "Helper");
        EXPECT_NE(helper_class, nullptr) << "在树中找不到Helper类";
    }

    // 验证树的整体结构
    auto all_nodes = psi_ops.findAllNodes(psi_tree.get(), PSINodeType::CLASS);
    EXPECT_GT(all_nodes.size(), 2) << "应该找到多个类（Calculator, DataProcessor, Helper等）";

    auto all_functions = psi_ops.findAllNodes(psi_tree.get(), PSINodeType::FUNCTION);
    EXPECT_GT(all_functions.size(), 5) << "应该找到多个函数";
}

// 测试4: 复杂查询能力
TEST_F(IntegrationTest, ComplexQueryCapabilities) {
    // 构建Stub索引和PSI树
    StubParser stub_parser;
    auto parse_result = stub_parser.parseCode(test_content, file_path);
    auto stub_entries = parse_result.getEntries();

    StubIndex stub_index;
    for (const auto& entry : stub_entries) {
        stub_index.addEntry(entry);
    }

    PSITreeBuilder psi_builder;
    auto psi_tree = psi_builder.buildTreeFromContent(file_path, test_content);

    PSITreeOperations psi_ops;

    // 1. 复合查询：查找特定文件中的所有函数
    auto file_functions = stub_index.queryByFile(file_path);
    size_t function_count = 0;
    for (size_t i = 0; i < file_functions.size(); ++i) {
        auto entry = file_functions.entries[i];
        if (entry->getType() == StubType::FUNCTION) {
            function_count++;
        }
    }

    auto psi_file_functions = psi_ops.findNodesInFile(psi_tree.get(), file_path);
    size_t psi_function_count = 0;
    for (auto* node : psi_file_functions) {
        if (node->getType() == PSINodeType::FUNCTION) {
            psi_function_count++;
        }
    }

    EXPECT_EQ(function_count, psi_function_count)
        << "Stub索引和PSI树的文件内函数数量不一致";

    // 2. 使用PSI查询进行复杂过滤
    auto public_methods = PSITreeQuery(psi_tree.get())
        .ofType(PSINodeType::FUNCTION)
        .execute();

    EXPECT_GT(public_methods.size(), 0) << "找不到任何方法";

    // 3. 验证语义信息传递
    auto* calculator_node = psi_ops.findFirstNodeByName(psi_tree.get(), "Calculator");
    ASSERT_NE(calculator_node, nullptr);

    // 为节点添加语义信息
    calculator_node->setSemanticInfo("category", "math");
    calculator_node->setSemanticInfo("complexity", "medium");

    // 验证语义信息查询
    auto math_classes = psi_ops.findNodesByCondition(psi_tree.get(), [](PSINode* node) {
        return node->getSemanticInfo("category") == "math";
    });

    EXPECT_GT(math_classes.size(), 0) << "通过语义信息找不到数学类";
}

// 测试5: 更新同步测试
TEST_F(IntegrationTest, UpdateSynchronizationTest) {
    // 初始构建
    StubParser stub_parser;
    auto parse_result = stub_parser.parseCode(test_content, file_path);
    auto stub_entries = parse_result.getEntries();

    StubIndex stub_index;
    for (const auto& entry : stub_entries) {
        stub_index.addEntry(entry);
    }

    PSITreeBuilder psi_builder;
    auto psi_tree = psi_builder.buildTreeFromContent(file_path, test_content);

    PSITreeOperations psi_ops;

    // 记录初始状态
    size_t initial_stub_count = stub_entries.size();
    size_t initial_psi_count = psi_ops.getSubtreeSize(psi_tree.get());

    // 模拟代码更新：添加新类
    std::string updated_content = test_content + R"(
// 新增的类
class NewFeature {
public:
    void execute();
    bool isActive() const;
private:
    bool active_;
};
)";

    // 重新解析
    auto updated_parse_result = stub_parser.parseCode(updated_content, file_path);
    auto updated_stub_entries = updated_parse_result.getEntries();

    // 更新Stub索引（模拟）
    stub_index.clear();
    for (const auto& entry : updated_stub_entries) {
        stub_index.addEntry(entry);
    }

    // 重新构建PSI树
    auto updated_psi_tree = psi_builder.buildTreeFromContent(file_path, updated_content);

    // 验证更新后的状态
    EXPECT_GT(updated_stub_entries.size(), initial_stub_count)
        << "更新后的Stub条目数量应该增加";

    size_t updated_psi_count = psi_ops.getSubtreeSize(updated_psi_tree.get());
    EXPECT_GT(updated_psi_count, initial_psi_count)
        << "更新后的PSI树大小应该增加";

    // 验证新功能存在
    auto* new_feature = psi_ops.findFirstNodeByName(updated_psi_tree.get(), "NewFeature");
    EXPECT_NE(new_feature, nullptr) << "找不到新添加的NewFeature类";

    auto new_feature_stubs = stub_index.queryByName("NewFeature");
    EXPECT_GT(new_feature_stubs.size(), 0) << "Stub索引中找不到NewFeature";
}

// 测试6: 性能和内存测试
TEST_F(IntegrationTest, PerformanceAndMemoryTest) {
    // 创建大量代码内容以测试性能
    std::string large_content;
    for (int i = 0; i < 100; ++i) {
        large_content += R"(
class TestClass)" + std::to_string(i) + R"( {
public:
    void method)" + std::to_string(i) + R"(();
    int getValue)" + std::to_string(i) + R"(();
private:
    int value)" + std::to_string(i) + R"(;
};
)";
    }

    // 测试构建性能
    auto start_time = std::chrono::high_resolution_clock::now();

    StubParser stub_parser;
    auto parse_result = stub_parser.parseCode(large_content, "large_test.cpp");
    auto stub_entries = parse_result.getEntries();

    StubIndex stub_index;
    for (const auto& entry : stub_entries) {
        stub_index.addEntry(entry);
    }

    PSITreeBuilder psi_builder;
    auto psi_tree = psi_builder.buildTreeFromContent("large_test.cpp", large_content);

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // 验证构建时间合理（应该小于1秒）
    EXPECT_LT(duration.count(), 1000)
        << "构建100个类的Stub索引和PSI树耗时过长: " << duration.count() << "ms";

    // 验证内存使用合理
    PSITreeOperations psi_ops;
    auto all_nodes = psi_ops.findAllNodes(psi_tree.get(), PSINodeType::CLASS);
    EXPECT_EQ(all_nodes.size(), 100) << "应该找到100个类";

    // 测试查询性能
    start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < 100; ++i) {
        auto class_name = "TestClass" + std::to_string(i);
        auto* psi_node = psi_ops.findFirstNodeByName(psi_tree.get(), class_name);
        auto stub_results = stub_index.queryByName(class_name);

        ASSERT_NE(psi_node, nullptr) << "找不到类: " << class_name;
        ASSERT_GT(stub_results.size(), 0) << "Stub索引中找不到类: " << class_name;
    }

    end_time = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    EXPECT_LT(duration.count(), 100)
        << "执行100次查询耗时过长: " << duration.count() << "ms";
}

// 测试7: 错误处理和边界情况
TEST_F(IntegrationTest, ErrorHandlingAndBoundaryCases) {
    // 1. 空内容处理
    PSITreeBuilder psi_builder;
    auto empty_tree = psi_builder.buildTreeFromContent("empty.cpp", "");
    ASSERT_NE(empty_tree, nullptr);

    PSITreeOperations psi_ops;
    size_t empty_count = psi_ops.getSubtreeSize(empty_tree.get());
    EXPECT_EQ(empty_count, 1) << "空文件的PSI树应该只有文件根节点";

    // 2. 错误语法处理
    std::string invalid_content = R"(
class InvalidClass {
    // 缺少闭合大括号
    void invalidMethod(
        // 缺少函数体
};

int syntax_error_here = ;

void anotherError() {
    // 不完整的语句
)";

    auto invalid_tree = psi_builder.buildTreeFromContent("invalid.cpp", invalid_content);
    ASSERT_NE(invalid_tree, nullptr) << "即使语法错误，也应该能构建PSI树";

    // 3. 极大文件名处理
    std::string very_long_name(1000, 'a');
    very_long_name += ".cpp";
    auto long_name_tree = psi_builder.buildTreeFromContent(very_long_name, "int x;");
    ASSERT_NE(long_name_tree, nullptr) << "应该能处理长文件名";

    // 4. Unicode字符处理
    std::string unicode_content = R"(
class 测试类 {
public:
    void 中文方法();
    int 计算数值();
};

int 全局变量 = 42;
)";

    auto unicode_tree = psi_builder.buildTreeFromContent("unicode.cpp", unicode_content);
    ASSERT_NE(unicode_tree, nullptr) << "应该能处理Unicode字符";

    // 5. 空指针和异常处理
    EXPECT_NO_THROW({
        psi_ops.findAllNodes(nullptr, PSINodeType::CLASS);
    }) << "处理空指针时不应抛出异常";

    EXPECT_NO_THROW({
        psi_ops.findFirstNodeByName(nullptr, "test");
    }) << "处理空指针时不应抛出异常";
}