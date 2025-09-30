#pragma once
#include "psi_node.h"
#include <iostream>
#include <functional>

namespace stub_index {

// PSI访问器接口（访问者模式）
class PSIVisitor {
public:
    virtual ~PSIVisitor() = default;

    // 节点访问方法
    virtual void visit(PSINode* node) {
        if (node) {
            node->accept(this);
        }
    }

    // 具体节点类型访问方法
    virtual void visitFile(PSIFileNode* node) { visitNode(node); }
    virtual void visitNamespace(PSINamespaceNode* node) { visitNode(node); }
    virtual void visitClass(PSIClassNode* node) { visitNode(node); }
    virtual void visitFunction(PSIFunctionNode* node) { visitNode(node); }
    virtual void visitVariable(PSIVariableNode* node) { visitNode(node); }

    // 通用节点访问
    virtual void visitNode(PSINode* node) {
        // 默认情况下，遍历子节点
        if (node) {
            for (const auto& child : node->getChildren()) {
                visit(child.get());
            }
        }
    }
};

// 打印访问器（用于调试）
class PrintVisitor : public PSIVisitor {
public:
    void visitFile(PSIFileNode* node) override {
        std::cout << std::string(indent_, ' ') << "📁 File: " << node->getFilePath() << std::endl;
        indent_ += 2;
        PSIVisitor::visitFile(node);
        indent_ -= 2;
    }

    void visitNamespace(PSINamespaceNode* node) override {
        std::cout << std::string(indent_, ' ') << "📦 Namespace: " << node->getName() << std::endl;
        indent_ += 2;
        PSIVisitor::visitNamespace(node);
        indent_ -= 2;
    }

    void visitClass(PSIClassNode* node) override {
        std::string prefix = node->isStruct() ? "🏗️  Struct" : "🏛️  Class";
        if (node->isAbstract()) prefix = "🎯 " + prefix;
        std::cout << std::string(indent_, ' ') << prefix << ": " << node->getName() << std::endl;
        indent_ += 2;
        PSIVisitor::visitClass(node);
        indent_ -= 2;
    }

    void visitFunction(PSIFunctionNode* node) override {
        std::cout << std::string(indent_, ' ') << "⚙️  Function: " << node->toString() << std::endl;
        indent_ += 2;
        PSIVisitor::visitFunction(node);
        indent_ -= 2;
    }

    void visitVariable(PSIVariableNode* node) override {
        std::cout << std::string(indent_, ' ') << "🔷 Variable: " << node->toString() << std::endl;
        PSIVisitor::visitVariable(node);
    }

    void visitNode(PSINode* node) override {
        // 对于其他类型的节点，使用简化的显示
        std::string indent_str(indent_, ' ');
        std::cout << indent_str << "🔹 " << node->toString() << std::endl;
        indent_ += 2;
        PSIVisitor::visitNode(node);
        indent_ -= 2;
    }

private:
    int indent_ = 0;
};

// 收集访问器（用于收集特定类型的节点）
template<typename T>
class CollectVisitor : public PSIVisitor {
public:
    void visit(T* node) {
        collected_nodes_.push_back(node);
        PSIVisitor::visit(node);
    }

    const std::vector<T*>& getCollectedNodes() const {
        return collected_nodes_;
    }

    void clear() {
        collected_nodes_.clear();
    }

private:
    std::vector<T*> collected_nodes_;
};

// 查找访问器（用于查找第一个匹配的节点）
template<typename T>
class FindVisitor : public PSIVisitor {
public:
    FindVisitor(const std::function<bool(T*)>& predicate)
        : predicate_(predicate) {}

    void visit(T* node) {
        if (predicate_(node)) {
            found_node_ = node;
            return; // 找到后停止遍历
        }
        PSIVisitor::visit(node);
    }

    T* getFoundNode() const {
        return found_node_;
    }

private:
    std::function<bool(T*)> predicate_;
    T* found_node_ = nullptr;
};

// 统计访问器（用于统计节点信息）
class StatisticsVisitor : public PSIVisitor {
public:
    void visitFile(PSIFileNode* node) override {
        file_count_++;
        PSIVisitor::visitFile(node);
    }

    void visitNamespace(PSINamespaceNode* node) override {
        namespace_count_++;
        PSIVisitor::visitNamespace(node);
    }

    void visitClass(PSIClassNode* node) override {
        class_count_++;
        if (node->isStruct()) struct_count_++;
        if (node->isAbstract()) abstract_class_count_++;
        PSIVisitor::visitClass(node);
    }

    void visitFunction(PSIFunctionNode* node) override {
        function_count_++;
        if (node->isVirtual()) virtual_function_count_++;
        if (node->isStatic()) static_function_count_++;
        if (node->isConst()) const_function_count_++;
        PSIVisitor::visitFunction(node);
    }

    void visitVariable(PSIVariableNode* node) override {
        variable_count_++;
        if (node->isConst()) const_variable_count_++;
        if (node->isStatic()) static_variable_count_++;
        if (node->isMember()) member_variable_count_++;
        if (node->isParameter()) parameter_count_++;
        PSIVisitor::visitVariable(node);
    }

    void visitNode(PSINode* node) override {
        total_node_count_++;
        PSIVisitor::visitNode(node);
    }

    void printStatistics() const {
        std::cout << "=== PSI Tree Statistics ===" << std::endl;
        std::cout << "Total nodes: " << total_node_count_ << std::endl;
        std::cout << "Files: " << file_count_ << std::endl;
        std::cout << "Namespaces: " << namespace_count_ << std::endl;
        std::cout << "Classes: " << class_count_ << " (Structs: " << struct_count_
                  << ", Abstract: " << abstract_class_count_ << ")" << std::endl;
        std::cout << "Functions: " << function_count_ << " (Virtual: " << virtual_function_count_
                  << ", Static: " << static_function_count_ << ", Const: " << const_function_count_ << ")" << std::endl;
        std::cout << "Variables: " << variable_count_ << " (Const: " << const_variable_count_
                  << ", Static: " << static_variable_count_ << ", Member: " << member_variable_count_
                  << ", Parameter: " << parameter_count_ << ")" << std::endl;
    }

    void reset() {
        total_node_count_ = 0;
        file_count_ = 0;
        namespace_count_ = 0;
        class_count_ = 0;
        struct_count_ = 0;
        abstract_class_count_ = 0;
        function_count_ = 0;
        virtual_function_count_ = 0;
        static_function_count_ = 0;
        const_function_count_ = 0;
        variable_count_ = 0;
        const_variable_count_ = 0;
        static_variable_count_ = 0;
        member_variable_count_ = 0;
        parameter_count_ = 0;
    }

    // 用于测试的访问方法
    const std::vector<PSINode*>& getCollectedNodes() const {
        // 返回一个空向量，因为StatisticsVisitor主要是统计，不收集节点
        static std::vector<PSINode*> empty;
        return empty;
    }

private:
    int total_node_count_ = 0;
    int file_count_ = 0;
    int namespace_count_ = 0;
    int class_count_ = 0;
    int struct_count_ = 0;
    int abstract_class_count_ = 0;
    int function_count_ = 0;
    int virtual_function_count_ = 0;
    int static_function_count_ = 0;
    int const_function_count_ = 0;
    int variable_count_ = 0;
    int const_variable_count_ = 0;
    int static_variable_count_ = 0;
    int member_variable_count_ = 0;
    int parameter_count_ = 0;
};

}