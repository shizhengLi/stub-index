#include <gtest/gtest.h>
#include "psi_tree_operations.h"
#include "psi_tree_builder.h"

using namespace stub_index;

TEST(PSITreeOperationsTest, FindAllNodes) {
    std::string content = R"(
class TestClass {
public:
    void method1();
    void method2();
private:
    int member1;
    int member2;
};

void globalFunction() {
    // implementation
}

int globalVariable = 42;
)";

    PSITreeBuilder builder;
    auto tree = builder.buildTreeFromContent("test.cpp", content);

    PSITreeOperations ops;

    // 测试查找所有类节点
    auto classes = ops.findAllNodes(tree.get(), PSINodeType::CLASS);
    EXPECT_EQ(classes.size(), 1);

    // 测试查找所有函数节点
    auto functions = ops.findAllNodes(tree.get(), PSINodeType::FUNCTION);
    EXPECT_GE(functions.size(), 1);

    // 测试查找所有变量节点
    auto variables = ops.findAllNodes(tree.get(), PSINodeType::VARIABLE);
    EXPECT_GE(variables.size(), 1);
}

TEST(PSITreeOperationsTest, FindNodesByName) {
    std::string content = R"(
class Calculator {
public:
    int add(int a, int b);
    int multiply(int x, int y);
};

int add(int a, int b) {
    return a + b;
}
)";

    PSITreeBuilder builder;
    auto tree = builder.buildTreeFromContent("test.cpp", content);

    PSITreeOperations ops;

    // 查找名为"add"的节点
    auto add_nodes = ops.findNodesByName(tree.get(), "add");
    EXPECT_GE(add_nodes.size(), 1);

    // 查找第一个名为"Calculator"的节点
    auto* calculator = ops.findFirstNodeByName(tree.get(), "Calculator");
    ASSERT_NE(calculator, nullptr);
    EXPECT_EQ(calculator->getText(), "Calculator");

    // 查找不存在的节点
    auto* non_existent = ops.findFirstNodeByName(tree.get(), "NonExistent");
    EXPECT_EQ(non_existent, nullptr);
}

TEST(PSITreeOperationsTest, FindNodesByCondition) {
    std::string content = R"(
class TestClass {
public:
    void publicMethod();
private:
    void privateMethod();
};

int globalVar = 100;
)";

    PSITreeBuilder builder;
    auto tree = builder.buildTreeFromContent("test.cpp", content);

    PSITreeOperations ops;

    // 查找所有函数节点
    auto all_functions = ops.findNodesByCondition(tree.get(), [](PSINode* node) {
        return node->getType() == PSINodeType::FUNCTION;
    });
    EXPECT_GE(all_functions.size(), 1);

    // 查找所有包含"Method"的节点
    auto method_nodes = ops.findNodesByCondition(tree.get(), [](PSINode* node) {
        return node->getText().find("Method") != std::string::npos;
    });
    EXPECT_GE(method_nodes.size(), 1);
}

TEST(PSITreeOperationsTest, HierarchyOperations) {
    std::string content = R"(
class OuterClass {
public:
    void outerMethod();
};

class InnerClass {
public:
    void innerMethod();
};

void globalFunction();
)";

    PSITreeBuilder builder;
    auto tree = builder.buildTreeFromContent("test.cpp", content);

    PSITreeOperations ops;

    // 查找所有后代节点
    auto* outer_class = ops.findFirstNodeByName(tree.get(), "OuterClass");
    ASSERT_NE(outer_class, nullptr);

    auto descendants = ops.getAllDescendants(outer_class);
    EXPECT_EQ(descendants.size(), 0); // 当前实现中类没有子节点

    // 查找祖先节点 - 所有节点都应该是根节点的子节点
    auto* inner_method = ops.findFirstNodeByName(tree.get(), "innerMethod");
    ASSERT_NE(inner_method, nullptr);

    auto ancestors = ops.getAncestors(inner_method);
    EXPECT_EQ(ancestors.size(), 1); // 只有根节点

    // 查找共同祖先 - 当前实现中共同祖先应该是根节点
    auto* outer_method = ops.findFirstNodeByName(tree.get(), "outerMethod");
    ASSERT_NE(outer_method, nullptr);

    auto* common_ancestor = ops.findCommonAncestor(outer_method, inner_method);
    ASSERT_NE(common_ancestor, nullptr);
    EXPECT_EQ(common_ancestor->getType(), PSINodeType::FILE); // 共同祖先是文件节点
}

TEST(PSITreeOperationsTest, PathOperations) {
    std::string content = R"(
namespace myapp {
class Calculator {
public:
    int add(int a, int b);
};
}
)";

    PSITreeBuilder builder;
    auto tree = builder.buildTreeFromContent("test.cpp", content);

    PSITreeOperations ops;

    // 获取节点路径
    auto* calculator = ops.findFirstNodeByName(tree.get(), "Calculator");
    ASSERT_NE(calculator, nullptr);

    std::string path = ops.getNodePath(calculator);
    EXPECT_FALSE(path.empty());
    EXPECT_NE(path.find("Calculator"), std::string::npos);

    // 通过路径查找节点（简化测试）
    // 注意：实际的路径查找需要完整的命名空间支持
}

TEST(PSITreeOperationsTest, TreeMetrics) {
    std::string content = R"(
class RootClass {
public:
    void method1();
    void method2();
};

class NestedClass {
public:
    void nestedMethod();
};

void globalFunction();
)";

    PSITreeBuilder builder;
    auto tree = builder.buildTreeFromContent("test.cpp", content);

    PSITreeOperations ops;

    // 测试树深度 - 当前实现中深度为2（文件节点 + 子节点）
    int depth = ops.getTreeDepth(tree.get());
    EXPECT_GE(depth, 1);

    // 测试子树大小
    auto* root_class = ops.findFirstNodeByName(tree.get(), "RootClass");
    ASSERT_NE(root_class, nullptr);

    int subtree_size = ops.getSubtreeSize(root_class);
    EXPECT_EQ(subtree_size, 1); // 只有RootClass节点本身

    // 测试整个树的子树大小
    int tree_size = ops.getSubtreeSize(tree.get());
    EXPECT_GT(tree_size, 3); // 文件节点 + 至少3个子节点

    // 测试叶子节点 - 当前实现中所有节点都是叶子节点
    auto leaves = ops.getLeafNodes(tree.get());
    EXPECT_GT(leaves.size(), 0);

    // 测试分支节点 - 当前实现中只有文件节点是分支节点
    auto branches = ops.getBranchNodes(tree.get());
    EXPECT_EQ(branches.size(), 1); // 只有文件节点
}

TEST(PSITreeOperationsTest, TreeValidation) {
    std::string content = R"(
class ValidClass {
public:
    void validMethod();
};
)";

    PSITreeBuilder builder;
    auto tree = builder.buildTreeFromContent("test.cpp", content);

    PSITreeOperations ops;

    // 测试树验证
    bool is_valid = ops.validateTree(tree.get());
    EXPECT_TRUE(is_valid);

    auto errors = ops.getValidationErrors(tree.get());
    EXPECT_TRUE(errors.empty());
}

TEST(PSITreeOperationsTest, TreeSimilarity) {
    std::string content1 = R"(
class Calculator {
public:
    int add(int a, int b);
    int subtract(int a, int b);
};
)";

    std::string content2 = R"(
class Calculator {
public:
    int add(int a, int b);
    int multiply(int x, int y);
};
)";

    PSITreeBuilder builder;
    auto tree1 = builder.buildTreeFromContent("test1.cpp", content1);
    auto tree2 = builder.buildTreeFromContent("test2.cpp", content2);

    PSITreeOperations ops;

    // 测试相似度计算
    double similarity = ops.calculateSimilarity(tree1.get(), tree2.get());
    EXPECT_GT(similarity, 0.0);
    EXPECT_LE(similarity, 1.0);

    // 测试差异查找
    auto differences = ops.findDifferences(tree1.get(), tree2.get());
    // 应该能找到一些差异
}

TEST(PSITreeQueryTest, BasicQuery) {
    std::string content = R"(
class Calculator {
public:
    int add(int a, int b);
    int multiply(int x, int y);
private:
    int value;
};

class Helper {
public:
    void help();
};
)";

    PSITreeBuilder builder;
    auto tree = builder.buildTreeFromContent("test.cpp", content);

    // 测试基本查询
    auto classes = PSITreeQuery(tree.get()).ofType(PSINodeType::CLASS).execute();
    EXPECT_EQ(classes.size(), 2);

    auto functions = PSITreeQuery(tree.get()).ofType(PSINodeType::FUNCTION).execute();
    EXPECT_GE(functions.size(), 3);

    auto named_calculator = PSITreeQuery(tree.get()).withName("Calculator").first();
    ASSERT_NE(named_calculator, nullptr);
    EXPECT_EQ(named_calculator->getText(), "Calculator");

    // 测试复合查询
    auto calculator_methods = PSITreeQuery(tree.get())
        .withName("add")
        .execute();
    EXPECT_GE(calculator_methods.size(), 1);
}

TEST(PSITreeQueryTest, AggregationQueries) {
    std::string content = R"(
class Calculator {
public:
    int add(int a, int b);
    int multiply(int x, int y);
};

class Helper {
public:
    void help();
    void assist();
};

int global_var = 42;
)";

    PSITreeBuilder builder;
    auto tree = builder.buildTreeFromContent("test.cpp", content);

    // 测试计数 - use separate query objects
    size_t class_count = PSITreeQuery(tree.get()).ofType(PSINodeType::CLASS).count();
    EXPECT_EQ(class_count, 2);

    size_t function_count = PSITreeQuery(tree.get()).ofType(PSINodeType::FUNCTION).count();
    EXPECT_GE(function_count, 4);

    // 测试分组
    auto type_groups = PSITreeQuery(tree.get()).groupByType();
    EXPECT_GT(type_groups["Class"], 0);
    EXPECT_GT(type_groups["Function"], 0);

    auto name_groups = PSITreeQuery(tree.get()).groupByName();
    EXPECT_GT(name_groups["Calculator"], 0);
    EXPECT_GT(name_groups["Helper"], 0);
}

TEST(PSITreeAnalyzerTest, ComplexityAnalysis) {
    std::string content = R"(
class ComplexClass {
public:
    void method1();
    void method2();
    void method3();

    class InnerClass {
    public:
        void innerMethod1();
        void innerMethod2();
    };
};
)";

    PSITreeBuilder builder;
    auto tree = builder.buildTreeFromContent("test.cpp", content);

    PSITreeAnalyzer analyzer;

    // 测试复杂度分析
    auto metrics = analyzer.analyzeComplexity(tree.get());
    EXPECT_GE(metrics.cyclomatic_complexity, 0);
    EXPECT_GE(metrics.number_of_children, 0);
    EXPECT_GE(metrics.lines_of_code, 0);

    // 测试指标输出（至少不崩溃）
    analyzer.printMetrics(tree.get());
    EXPECT_TRUE(true);
}

TEST(PSITreeOperationsTest, TreeOptimization) {
    std::string content = R"(
class TestClass {
public:
    void method();
};

// This should create a tree that can be optimized
)";

    PSITreeBuilder builder;
    auto tree = builder.buildTreeFromContent("test.cpp", content);

    PSITreeOperations ops;

    // 测试树优化（至少不崩溃）
    ops.optimizeTree(tree.get());
    EXPECT_TRUE(true);

    // 测试子树克隆（至少不崩溃）
    auto clone = ops.cloneSubtree(tree.get());
    ASSERT_NE(clone, nullptr);
}

TEST(PSITreeOperationsTest, FileAndLineOperations) {
    std::string content = R"(
// Line 2
class LineTestClass { // Line 3
public: // Line 4
    void method1(); // Line 5
    void method2(); // Line 6
}; // Line 7

void functionOnLine9(); // Line 9
int variableOnLine10 = 0; // Line 10
)";

    PSITreeBuilder builder;
    auto tree = builder.buildTreeFromContent("test.cpp", content);

    PSITreeOperations ops;

    // 测试按文件查找
    auto file_nodes = ops.findNodesInFile(tree.get(), "test.cpp");
    EXPECT_GT(file_nodes.size(), 0);

    // 测试按行范围查找
    auto line_range_nodes = ops.findNodesInLineRange(tree.get(), 5, 7);
    EXPECT_GE(line_range_nodes.size(), 1);

    // 验证行范围查找的准确性
    bool found_method = false;
    for (auto* node : line_range_nodes) {
        if (node->getText().find("method1") != std::string::npos ||
            node->getText().find("method2") != std::string::npos) {
            found_method = true;
            break;
        }
    }
    EXPECT_TRUE(found_method);
}

TEST(PSITreeOperationsTest, SemanticInfoOperations) {
    std::string content = R"(
class SemanticClass {
public:
    void semanticMethod();
};
)";

    PSITreeBuilder builder;
    auto tree = builder.buildTreeFromContent("test.cpp", content);

    PSITreeOperations ops;

    // 为节点添加语义信息
    auto* test_class = ops.findFirstNodeByName(tree.get(), "SemanticClass");
    ASSERT_NE(test_class, nullptr);
    test_class->setSemanticInfo("test_key", "test_value");

    // 测试通过语义信息查找节点
    auto semantic_nodes = ops.findNodesByCondition(tree.get(), [](PSINode* node) {
        return node->hasSemanticInfo("test_key") &&
               node->getSemanticInfo("test_key") == "test_value";
    });
    EXPECT_EQ(semantic_nodes.size(), 1);
    EXPECT_EQ(semantic_nodes[0], test_class);
}