#include "psi_tree_builder.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <iostream>

namespace stub_index {

std::shared_ptr<PSIFileNode> PSITreeBuilder::buildTreeFromFile(const std::string& file_path) {
    // 读取文件内容
    std::ifstream file(file_path);
    if (!file.is_open()) {
        return nullptr;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();

    return buildTreeFromContent(file_path, content);
}

std::shared_ptr<PSIFileNode> PSITreeBuilder::buildTreeFromContent(const std::string& file_path,
                                                                  const std::string& content) {
    // 创建文件节点作为根节点
    auto file_node = std::make_shared<PSIFileNode>(file_path, content);

    // 使用Stub解析器解析代码
    StubParser parser;
    auto parse_result = parser.parseCode(content, file_path);
    auto stubs = parse_result.getEntries();

    if (stubs.empty()) {
        return file_node;
    }

    // 第一阶段：构建类和结构体结构
    buildClassStructure(file_node.get(), stubs);

    // 第二阶段：构建函数和变量结构
    buildFunctionStructure(file_node.get(), stubs);
    buildVariableStructure(file_node.get(), stubs);

    return file_node;
}

void PSITreeBuilder::buildClassStructure(PSINode* parent,
                                        const std::vector<std::shared_ptr<StubEntry>>& stubs) {
    for (const auto& stub : stubs) {
        if (stub->getType() == StubType::CLASS) {
            auto class_node = createClassNode(stub);
            parent->addChild(class_node);
        }
    }
}

void PSITreeBuilder::buildFunctionStructure(PSINode* parent,
                                          const std::vector<std::shared_ptr<StubEntry>>& stubs) {
    for (const auto& stub : stubs) {
        if (stub->getType() == StubType::FUNCTION) {
            auto func_node = createFunctionNode(stub);
            parent->addChild(func_node);
        }
    }
}

void PSITreeBuilder::buildVariableStructure(PSINode* parent,
                                          const std::vector<std::shared_ptr<StubEntry>>& stubs) {
    for (const auto& stub : stubs) {
        if (stub->getType() == StubType::VARIABLE) {
            auto var_node = createVariableNode(stub);
            parent->addChild(var_node);
        }
    }
}

std::shared_ptr<PSIClassNode> PSITreeBuilder::createClassNode(const std::shared_ptr<StubEntry>& entry) {
    auto* class_stub = static_cast<ClassStub*>(entry.get());
    auto node = std::make_shared<PSIClassNode>(
        entry->getName(),
        entry->getLocation(),
        class_stub->isStruct()
    );

    // 添加语义信息
    node->setSemanticInfo("stub_id", entry->getName());

    return node;
}

std::shared_ptr<PSIFunctionNode> PSITreeBuilder::createFunctionNode(const std::shared_ptr<StubEntry>& entry) {
    auto* func_stub = static_cast<FunctionStub*>(entry.get());
    auto node = std::make_shared<PSIFunctionNode>(
        entry->getName(),
        entry->getLocation(),
        func_stub->getReturnType()
    );

    // 添加参数
    const auto& params = func_stub->getParameters();
    for (const auto& param : params) {
        node->addParameter(param.type, param.name);
    }

    // 添加语义信息
    node->setSemanticInfo("stub_id", entry->getName());

    return node;
}

std::shared_ptr<PSIVariableNode> PSITreeBuilder::createVariableNode(const std::shared_ptr<StubEntry>& entry) {
    auto* var_stub = static_cast<VariableStub*>(entry.get());
    auto node = std::make_shared<PSIVariableNode>(
        entry->getName(),
        entry->getLocation(),
        var_stub->getVariableType()
    );

    // 设置变量修饰符
    node->setConst(var_stub->isConst());
    node->setStatic(var_stub->isStatic());

    // 添加语义信息
    node->setSemanticInfo("stub_id", entry->getName());

    return node;
}

// PSITreeFactory 实现
std::shared_ptr<PSIFileNode> PSITreeFactory::createFromFile(const std::string& file_path) {
    PSITreeBuilder builder;
    return builder.buildTreeFromFile(file_path);
}

std::shared_ptr<PSIFileNode> PSITreeFactory::createFromContent(const std::string& file_path,
                                                               const std::string& content) {
    PSITreeBuilder builder;
    return builder.buildTreeFromContent(file_path, content);
}

std::shared_ptr<PSIFileNode> PSITreeFactory::createWithDetailedAnalysis(const std::string& file_path) {
    PSITreeBuilder builder;
    builder.setIncludeComments(true);
    builder.setIncludePreprocessor(true);
    builder.setDetailedExpressions(true);
    return builder.buildTreeFromFile(file_path);
}

}