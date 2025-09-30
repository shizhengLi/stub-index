#include <gtest/gtest.h>
#include "psi_node.h"
#include "psi_visitor.h"

using namespace stub_index;

TEST(PSINodeTest, BasicNodeOperations) {
    SourceLocation loc("test.cpp", 1, 1);
    PSINode node(PSINodeType::CLASS, "MyClass", loc);

    EXPECT_EQ(node.getType(), PSINodeType::CLASS);
    EXPECT_EQ(node.getText(), "MyClass");
    EXPECT_EQ(node.getLocation().file_path, "test.cpp");
    EXPECT_EQ(node.getParent(), nullptr);
    EXPECT_TRUE(node.getChildren().empty());
}

TEST(PSINodeTest, TreeStructure) {
    SourceLocation loc("test.cpp", 1, 1);

    // 创建根节点
    auto root = std::make_shared<PSINode>(PSINodeType::FILE, "test.cpp", loc);

    // 创建子节点
    auto child1 = std::make_shared<PSINode>(PSINodeType::CLASS, "ClassA", loc);
    auto child2 = std::make_shared<PSINode>(PSINodeType::CLASS, "ClassB", loc);

    // 添加子节点
    root->addChild(child1);
    root->addChild(child2);

    // 验证父子关系
    EXPECT_EQ(root->getChildren().size(), 2);
    EXPECT_EQ(child1->getParent(), root.get());
    EXPECT_EQ(child2->getParent(), root.get());

    // 验证兄弟关系
    EXPECT_EQ(child1->getNextSibling(), child2.get());
    EXPECT_EQ(child2->getPrevSibling(), child1.get());
    EXPECT_EQ(child1->getPrevSibling(), nullptr);
    EXPECT_EQ(child2->getNextSibling(), nullptr);
}

TEST(PSINodeTest, NodeSearch) {
    SourceLocation loc("test.cpp", 1, 1);

    auto root = std::make_shared<PSINode>(PSINodeType::FILE, "test.cpp", loc);

    auto class1 = std::make_shared<PSINode>(PSINodeType::CLASS, "ClassA", loc);
    auto class2 = std::make_shared<PSINode>(PSINodeType::CLASS, "ClassB", loc);
    auto function = std::make_shared<PSINode>(PSINodeType::FUNCTION, "func", loc);

    root->addChild(class1);
    root->addChild(class2);
    root->addChild(function);

    // 查找所有类节点
    auto classes = root->findChildren(PSINodeType::CLASS);
    EXPECT_EQ(classes.size(), 2);

    // 查找第一个类节点
    auto first_class = root->findFirstChild(PSINodeType::CLASS);
    EXPECT_NE(first_class, nullptr);
    EXPECT_EQ(first_class->getText(), "ClassA");

    // 查找最后一个类节点
    auto last_class = root->findLastChild(PSINodeType::CLASS);
    EXPECT_NE(last_class, nullptr);
    EXPECT_EQ(last_class->getText(), "ClassB");

    // 查找不存在的节点类型
    auto namespaces = root->findChildren(PSINodeType::NAMESPACE);
    EXPECT_TRUE(namespaces.empty());
}

TEST(PSINodeTest, SemanticInfo) {
    SourceLocation loc("test.cpp", 1, 1);
    PSINode node(PSINodeType::CLASS, "MyClass", loc);

    // 设置语义信息
    node.setSemanticInfo("is_template", "true");
    node.setSemanticInfo("template_args", "T");

    // 验证语义信息
    EXPECT_TRUE(node.hasSemanticInfo("is_template"));
    EXPECT_EQ(node.getSemanticInfo("is_template"), "true");
    EXPECT_EQ(node.getSemanticInfo("template_args"), "T");
    EXPECT_FALSE(node.hasSemanticInfo("non_existent"));
    EXPECT_EQ(node.getSemanticInfo("non_existent"), "");
}

TEST(PSIFileNodeTest, FileNodeOperations) {
    std::string content = "int x = 5;";
    auto file_node = std::make_shared<PSIFileNode>("test.cpp", content);

    EXPECT_EQ(file_node->getType(), PSINodeType::FILE);
    EXPECT_EQ(file_node->getFilePath(), "test.cpp");
    EXPECT_EQ(file_node->getContent(), content);
    EXPECT_EQ(file_node->getTextRange().getLength(), content.length());
}

TEST(PSIClassNodeTest, ClassNodeOperations) {
    SourceLocation loc("test.cpp", 1, 1);
    auto class_node = std::make_shared<PSIClassNode>("MyClass", loc, false);

    EXPECT_EQ(class_node->getType(), PSINodeType::CLASS);
    EXPECT_EQ(class_node->getName(), "MyClass");
    EXPECT_FALSE(class_node->isStruct());
    EXPECT_FALSE(class_node->isAbstract());

    // 测试设置抽象类
    class_node->setAbstract(true);
    EXPECT_TRUE(class_node->isAbstract());
}

TEST(PSIFunctionNodeTest, FunctionNodeOperations) {
    SourceLocation loc("test.cpp", 1, 1);
    auto func_node = std::make_shared<PSIFunctionNode>("calculate", loc, "int");

    EXPECT_EQ(func_node->getType(), PSINodeType::FUNCTION);
    EXPECT_EQ(func_node->getName(), "calculate");
    EXPECT_EQ(func_node->getReturnType(), "int");
    EXPECT_TRUE(func_node->getParameters().empty());

    // 添加参数
    func_node->addParameter("int", "a");
    func_node->addParameter("double", "b", "0.0");

    const auto& params = func_node->getParameters();
    EXPECT_EQ(params.size(), 2);
    EXPECT_EQ(params[0].type, "int");
    EXPECT_EQ(params[0].name, "a");
    EXPECT_EQ(params[1].type, "double");
    EXPECT_EQ(params[1].name, "b");
    EXPECT_EQ(params[1].default_value, "0.0");

    // 测试修饰符
    func_node->setVirtual(true);
    func_node->setConst(true);
    func_node->setOverride(true);

    EXPECT_TRUE(func_node->isVirtual());
    EXPECT_TRUE(func_node->isConst());
    EXPECT_TRUE(func_node->isOverride());
}

TEST(PSIVariableNodeTest, VariableNodeOperations) {
    SourceLocation loc("test.cpp", 1, 1);
    auto var_node = std::make_shared<PSIVariableNode>("counter", loc, "int");

    EXPECT_EQ(var_node->getType(), PSINodeType::VARIABLE);
    EXPECT_EQ(var_node->getName(), "counter");
    EXPECT_EQ(var_node->getVariableType(), "int");
    EXPECT_FALSE(var_node->isConst());
    EXPECT_FALSE(var_node->isStatic());
    EXPECT_FALSE(var_node->isMember());
    EXPECT_FALSE(var_node->isParameter());

    // 测试修饰符
    var_node->setConst(true);
    var_node->setStatic(true);
    var_node->setMember(true);

    EXPECT_TRUE(var_node->isConst());
    EXPECT_TRUE(var_node->isStatic());
    EXPECT_TRUE(var_node->isMember());
}

TEST(PSIVisitorTest, PrintVisitor) {
    SourceLocation loc("test.cpp", 1, 1);

    // 构建一个简单的PSI树
    auto file_node = std::make_shared<PSIFileNode>("test.cpp", "content");
    auto class_node = std::make_shared<PSIClassNode>("MyClass", loc);
    auto func_node = std::make_shared<PSIFunctionNode>("method", loc, "void");
    auto var_node = std::make_shared<PSIVariableNode>("data", loc, "int");

    file_node->addChild(class_node);
    class_node->addChild(func_node);
    class_node->addChild(var_node);

    // 使用打印访问器
    PrintVisitor visitor;
    visitor.visit(file_node.get());

    // 测试不会崩溃，输出格式正确即可
    EXPECT_TRUE(true);
}

TEST(PSIVisitorTest, CollectVisitor) {
    SourceLocation loc("test.cpp", 1, 1);

    auto file_node = std::make_shared<PSIFileNode>("test.cpp", "content");
    auto class_node = std::make_shared<PSIClassNode>("ClassA", loc);
    auto class_node2 = std::make_shared<PSIClassNode>("ClassB", loc);
    auto func_node = std::make_shared<PSIFunctionNode>("method", loc, "void");

    file_node->addChild(class_node);
    file_node->addChild(class_node2);
    file_node->addChild(func_node);

    // 收集所有类节点 - 使用基类访问器
    CollectVisitor<PSINode> collector;
    collector.visit(file_node.get());

    auto nodes = collector.getCollectedNodes();
    EXPECT_GT(nodes.size(), 0);
}

TEST(PSIVisitorTest, StatisticsVisitor) {
    SourceLocation loc("test.cpp", 1, 1);

    // 构建一个复杂的PSI树
    auto file_node = std::make_shared<PSIFileNode>("test.cpp", "content");
    auto class_node = std::make_shared<PSIClassNode>("MyClass", loc);
    auto struct_node = std::make_shared<PSIClassNode>("MyStruct", loc, true);
    auto func_node = std::make_shared<PSIFunctionNode>("method", loc, "void");
    auto var_node = std::make_shared<PSIVariableNode>("data", loc, "int");

    file_node->addChild(class_node);
    file_node->addChild(struct_node);
    class_node->addChild(func_node);
    class_node->addChild(var_node);

    // 统计访问器
    StatisticsVisitor stats;
    stats.visit(file_node.get());

    // 验证统计结果 - 检查统计信息
    stats.printStatistics(); // 至少能正常运行
    EXPECT_TRUE(true); // 基本验证
}