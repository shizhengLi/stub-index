#include <gtest/gtest.h>
#include "psi_tree_operations.h"
#include "psi_tree_builder.h"
#include "psi_visitor.h"

using namespace stub_index;

class PSITreeTransformerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 创建测试用的PSI树
        std::string content = R"(
class BaseClass {
public:
    void baseMethod();
};

class DerivedClass : public BaseClass {
public:
    void derivedMethod();
    void helperMethod();
};

namespace test {
    class NestedClass {
    public:
        void nestedMethod();
    };
}

void globalFunction();
int globalVar = 42;
)";

        PSITreeBuilder builder;
        tree = builder.buildTreeFromContent("test.cpp", content);
    }

    std::shared_ptr<PSIFileNode> tree;
};

TEST_F(PSITreeTransformerTest, TransformTreeBasic) {
    PSITreeTransformer transformer;

    // 创建简单的转换器：将所有节点名称转换为大写
    auto upper_transformer = [](PSINode* node) -> std::shared_ptr<PSINode> {
        std::string name = node->getText();
        std::transform(name.begin(), name.end(), name.begin(), ::toupper);
        return std::make_shared<PSINode>(node->getType(), name, node->getLocation());
    };

    auto transformed_tree = transformer.transformTree(tree.get(), upper_transformer);
    ASSERT_NE(transformed_tree, nullptr);

    // 验证转换结果
    PSITreeOperations ops;
    auto classes = ops.findAllNodes(transformed_tree.get(), PSINodeType::CLASS);

    EXPECT_GT(classes.size(), 0);
    for (auto* class_node : classes) {
        std::string name = class_node->getText();
        EXPECT_EQ(name, std::string(name.size(), std::toupper(name[0])));
    }
}

TEST_F(PSITreeTransformerTest, SimplifyTree) {
    PSITreeTransformer transformer;

    auto simplified_tree = transformer.simplifyTree(tree.get());
    ASSERT_NE(simplified_tree, nullptr);

    // 简化后的树应该保持基本结构
    PSITreeOperations ops;
    auto classes = ops.findAllNodes(simplified_tree.get(), PSINodeType::CLASS);
    auto functions = ops.findAllNodes(simplified_tree.get(), PSINodeType::FUNCTION);

    EXPECT_GT(classes.size(), 0);
    EXPECT_GT(functions.size(), 0);
}

TEST_F(PSITreeTransformerTest, RemoveNodesByType) {
    PSITreeTransformer transformer;

    // 移除所有变量节点
    auto tree_without_vars = transformer.removeNodesByType(tree.get(), PSINodeType::VARIABLE);
    ASSERT_NE(tree_without_vars, nullptr);

    // 验证变量节点被移除
    PSITreeOperations ops;
    auto variables = ops.findAllNodes(tree_without_vars.get(), PSINodeType::VARIABLE);
    EXPECT_EQ(variables.size(), 0);

    // 验证其他节点仍然存在
    auto classes = ops.findAllNodes(tree_without_vars.get(), PSINodeType::CLASS);
    EXPECT_GT(classes.size(), 0);
}

TEST_F(PSITreeTransformerTest, RemoveNodesByTypeMultiple) {
    PSITreeTransformer transformer;

    // 记录原始节点数量
    PSITreeOperations ops;
    auto original_classes = ops.findAllNodes(tree.get(), PSINodeType::CLASS);
    auto original_functions = ops.findAllNodes(tree.get(), PSINodeType::FUNCTION);
    auto original_variables = ops.findAllNodes(tree.get(), PSINodeType::VARIABLE);

    // 移除函数节点
    auto tree_without_functions = transformer.removeNodesByType(tree.get(), PSINodeType::FUNCTION);

    // 验证函数节点被移除
    auto remaining_functions = ops.findAllNodes(tree_without_functions.get(), PSINodeType::FUNCTION);
    EXPECT_EQ(remaining_functions.size(), 0);

    // 验证其他节点仍然存在
    auto remaining_classes = ops.findAllNodes(tree_without_functions.get(), PSINodeType::CLASS);
    auto remaining_variables = ops.findAllNodes(tree_without_functions.get(), PSINodeType::VARIABLE);

    EXPECT_EQ(remaining_classes.size(), original_classes.size());
    EXPECT_EQ(remaining_variables.size(), original_variables.size());
}

TEST_F(PSITreeTransformerTest, FlattenHierarchy) {
    PSITreeTransformer transformer;

    // 创建具有深层嵌套的树
    std::string nested_content = R"(
class Level1 {
public:
    class Level2 {
    public:
        class Level3 {
        public:
            void method();
        };
    };
};
)";

    PSITreeBuilder builder;
    auto nested_tree = builder.buildTreeFromContent("nested.cpp", nested_content);

    // 扁平化层次结构
    auto flattened_tree = transformer.flattenHierarchy(nested_tree.get(), 2);
    ASSERT_NE(flattened_tree, nullptr);

    // 验证深度不超过指定值
    PSITreeOperations ops;
    int depth = ops.getTreeDepth(flattened_tree.get());
    EXPECT_LE(depth, 2);
}

TEST_F(PSITreeTransformerTest, MergeTrees) {
    PSITreeTransformer transformer;

    // 创建第二个树
    std::string content2 = R"(
class AdditionalClass {
public:
    void additionalMethod();
};
)";

    PSITreeBuilder builder;
    auto tree2 = builder.buildTreeFromContent("test2.cpp", content2);

    // 合并两棵树
    std::vector<PSINode*> trees = {tree.get(), tree2.get()};
    auto merged_tree = transformer.mergeTrees(trees);
    ASSERT_NE(merged_tree, nullptr);

    // 验证合并后的树包含所有节点
    PSITreeOperations ops;
    auto classes = ops.findAllNodes(merged_tree.get(), PSINodeType::CLASS);
    EXPECT_GE(classes.size(), 2); // 至少包含来自两个树的类
}

TEST_F(PSITreeTransformerTest, OverlayTrees) {
    PSITreeTransformer transformer;

    // 创建基础树和覆盖树
    std::string base_content = R"(
class CommonClass {
public:
    void baseMethod();
};
)";

    std::string overlay_content = R"(
class CommonClass {
public:
    void overriddenMethod();
};
class OverlayClass {
public:
    void overlayMethod();
};
)";

    PSITreeBuilder builder;
    auto base_tree = builder.buildTreeFromContent("base.cpp", base_content);
    auto overlay_tree = builder.buildTreeFromContent("overlay.cpp", overlay_content);

    // 执行覆盖操作
    auto result_tree = transformer.overlayTrees(base_tree.get(), overlay_tree.get());
    ASSERT_NE(result_tree, nullptr);

    // 验证覆盖结果
    PSITreeOperations ops;
    auto common_classes = ops.findAllNodes(result_tree.get(), PSINodeType::CLASS);
    EXPECT_GT(common_classes.size(), 0);
}

TEST_F(PSITreeTransformerTest, TransformEmptyTree) {
    PSITreeTransformer transformer;

    // 创建空树
    PSITreeBuilder builder;
    auto empty_tree = builder.buildTreeFromContent("empty.cpp", "");

    // 测试对空树的转换
    auto identity_transformer = [](PSINode* node) -> std::shared_ptr<PSINode> {
        return std::make_shared<PSINode>(node->getType(), node->getText(), node->getLocation());
    };

    auto transformed_tree = transformer.transformTree(empty_tree.get(), identity_transformer);
    ASSERT_NE(transformed_tree, nullptr);

    // 空树转换后应该仍然有文件根节点
    PSITreeOperations ops;
    auto file_nodes = ops.findAllNodes(transformed_tree.get(), PSINodeType::FILE);
    EXPECT_EQ(file_nodes.size(), 1);
}

TEST_F(PSITreeTransformerTest, TransformWithNullTransformer) {
    PSITreeTransformer transformer;

    // 测试空转换器（应该返回nullptr）
    auto null_transformer = [](PSINode* node) -> std::shared_ptr<PSINode> {
        return nullptr;
    };

    auto transformed_tree = transformer.transformTree(tree.get(), null_transformer);
    EXPECT_EQ(transformed_tree, nullptr);
}

TEST_F(PSITreeTransformerTest, ReorganizeByNamespace) {
    PSITreeTransformer transformer;

    // 创建包含命名空间的树
    std::string namespaced_content = R"(
namespace myapp {
    class AppClass {
    public:
        void appMethod();
    };
}

namespace utils {
    class HelperClass {
    public:
        void helperMethod();
    };
}
)";

    PSITreeBuilder builder;
    auto namespaced_tree = builder.buildTreeFromContent("namespaced.cpp", namespaced_content);

    // 按命名空间重新组织
    auto reorganized_tree = transformer.reorganizeByNamespace(namespaced_tree.get());
    ASSERT_NE(reorganized_tree, nullptr);

    // 验证重新组织后的结构
    PSITreeOperations ops;
    auto nodes = ops.findAllNodes(reorganized_tree.get(), PSINodeType::CLASS);
    EXPECT_GT(nodes.size(), 0);
}

TEST_F(PSITreeTransformerTest, CloneNodePreservesStructure) {
    PSITreeTransformer transformer;

    // 获取原始树的子节点数量
    PSITreeOperations ops;
    auto original_classes = ops.findAllNodes(tree.get(), PSINodeType::CLASS);
    auto original_functions = ops.findAllNodes(tree.get(), PSINodeType::FUNCTION);
    auto original_variables = ops.findAllNodes(tree.get(), PSINodeType::VARIABLE);

    // 克隆树
    auto cloned_tree = transformer.transformTree(tree.get(), [](PSINode* node) -> std::shared_ptr<PSINode> {
        return std::make_shared<PSINode>(node->getType(), node->getText(), node->getLocation());
    });

    ASSERT_NE(cloned_tree, nullptr);

    // 验证克隆后的树保持相同的结构
    auto cloned_classes = ops.findAllNodes(cloned_tree.get(), PSINodeType::CLASS);
    auto cloned_functions = ops.findAllNodes(cloned_tree.get(), PSINodeType::FUNCTION);
    auto cloned_variables = ops.findAllNodes(cloned_tree.get(), PSINodeType::VARIABLE);

    EXPECT_EQ(cloned_classes.size(), original_classes.size());
    EXPECT_EQ(cloned_functions.size(), original_functions.size());
    EXPECT_EQ(cloned_variables.size(), original_variables.size());
}

TEST_F(PSITreeTransformerTest, TransformPreservesSemanticInfo) {
    PSITreeTransformer transformer;

    // 为原始树添加语义信息
    PSITreeOperations ops;
    auto* first_class = ops.findFirstNodeByName(tree.get(), "BaseClass");
    ASSERT_NE(first_class, nullptr);
    first_class->setSemanticInfo("test_key", "test_value");

    // 执行转换
    auto identity_transformer = [](PSINode* node) -> std::shared_ptr<PSINode> {
        auto clone = std::make_shared<PSINode>(node->getType(), node->getText(), node->getLocation());
        // 复制语义信息
        clone->setSemanticInfo("test_key", node->getSemanticInfo("test_key"));
        return clone;
    };

    auto transformed_tree = transformer.transformTree(tree.get(), identity_transformer);
    ASSERT_NE(transformed_tree, nullptr);

    // 验证语义信息被保留
    auto* transformed_class = ops.findFirstNodeByName(transformed_tree.get(), "BaseClass");
    ASSERT_NE(transformed_class, nullptr);
    EXPECT_EQ(transformed_class->getSemanticInfo("test_key"), "test_value");
}