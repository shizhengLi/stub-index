#include "psi_tree_operations.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <stack>

namespace stub_index {

// PSITreeOperations 实现
std::vector<PSINode*> PSITreeOperations::findAllNodes(PSINode* root, PSINodeType type) {
    std::vector<PSINode*> result;
    collectNodesByType(root, type, result);
    return result;
}

std::vector<PSINode*> PSITreeOperations::findNodesByCondition(
    PSINode* root,
    const std::function<bool(PSINode*)>& condition) {
    std::vector<PSINode*> result;
    collectNodesByCondition(root, condition, result);
    return result;
}

std::vector<PSINode*> PSITreeOperations::findNodesByName(PSINode* root, const std::string& name) {
    return findNodesByCondition(root, [name](PSINode* node) {
        return node->getText() == name;
    });
}

PSINode* PSITreeOperations::findFirstNodeByName(PSINode* root, const std::string& name) {
    auto nodes = findNodesByName(root, name);
    return nodes.empty() ? nullptr : nodes[0];
}

std::vector<PSINode*> PSITreeOperations::findNodesInFile(PSINode* root, const std::string& file_path) {
    return findNodesByCondition(root, [file_path](PSINode* node) {
        return node->getLocation().file_path == file_path;
    });
}

std::vector<PSINode*> PSITreeOperations::findNodesInLineRange(PSINode* root, int start_line, int end_line) {
    return findNodesByCondition(root, [start_line, end_line](PSINode* node) {
        int line = node->getLocation().line;
        return line >= start_line && line <= end_line;
    });
}

std::vector<PSINode*> PSITreeOperations::getAllDescendants(PSINode* node) {
    std::vector<PSINode*> result;
    collectDescendants(node, result);
    return result;
}

std::vector<PSINode*> PSITreeOperations::getAncestors(PSINode* node) {
    std::vector<PSINode*> ancestors;
    PSINode* current = node->getParent();
    while (current) {
        ancestors.push_back(current);
        current = current->getParent();
    }
    std::reverse(ancestors.begin(), ancestors.end());
    return ancestors;
}

PSINode* PSITreeOperations::findCommonAncestor(PSINode* node1, PSINode* node2) {
    auto ancestors1 = getAncestors(node1);
    auto ancestors2 = getAncestors(node2);

    PSINode* common = nullptr;
    size_t min_size = std::min(ancestors1.size(), ancestors2.size());

    for (size_t i = 0; i < min_size; ++i) {
        if (ancestors1[i] == ancestors2[i]) {
            common = ancestors1[i];
        } else {
            break;
        }
    }

    return common;
}

std::string PSITreeOperations::getNodePath(PSINode* node) {
    std::string path;
    buildNodePath(node, path);
    return path;
}

PSINode* PSITreeOperations::findNodeByPath(PSINode* root, const std::string& path) {
    std::vector<std::string> parts;
    std::stringstream ss(path);
    std::string part;

    while (std::getline(ss, part, '/')) {
        if (!part.empty()) {
            parts.push_back(part);
        }
    }

    PSINode* current = root;
    for (const auto& part : parts) {
        bool found = false;
        for (const auto& child : current->getChildren()) {
            if (child->getText() == part) {
                current = child.get();
                found = true;
                break;
            }
        }
        if (!found) {
            return nullptr;
        }
    }

    return current;
}

int PSITreeOperations::getTreeDepth(PSINode* root) {
    return calculateDepth(root);
}

int PSITreeOperations::getSubtreeSize(PSINode* node) {
    return calculateSubtreeSize(node);
}

std::vector<PSINode*> PSITreeOperations::getLeafNodes(PSINode* root) {
    std::vector<PSINode*> leaves;
    collectLeafNodes(root, leaves);
    return leaves;
}

std::vector<PSINode*> PSITreeOperations::getBranchNodes(PSINode* root) {
    std::vector<PSINode*> branches;
    collectBranchNodes(root, branches);
    return branches;
}

void PSITreeOperations::removeNode(PSINode* node, bool keep_children) {
    if (!node || !node->getParent()) {
        return;
    }

    auto parent = node->getParent();
    auto& children = const_cast<std::vector<std::shared_ptr<PSINode>>&>(parent->getChildren());

    if (keep_children) {
        // 将子节点移动到父节点
        for (auto& child : node->getChildren()) {
            child->setParent(parent);
            children.push_back(child);
        }
    }

    // 查找并移除节点
    auto it = std::find_if(children.begin(), children.end(),
        [node](const std::shared_ptr<PSINode>& child) {
            return child.get() == node;
        });

    if (it != children.end()) {
        children.erase(it);
    }
}

void PSITreeOperations::moveNode(PSINode* node, PSINode* new_parent) {
    if (!node || !new_parent) {
        return;
    }

    // 先从原父节点移除
    removeNode(node, false);

    // 添加到新父节点
    auto shared_node = std::shared_ptr<PSINode>(node, [](PSINode*){}); // 非拥有式shared_ptr
    const_cast<PSINode*>(new_parent)->addChild(shared_node);
}

void PSITreeOperations::copyNode(PSINode* source, PSINode* target_parent) {
    if (!source || !target_parent) {
        return;
    }

    // 创建副本
    auto clone = cloneSubtree(source);
    clone->setParent(target_parent);
    const_cast<PSINode*>(target_parent)->addChild(clone);
}

bool PSITreeOperations::validateTree(PSINode* root) {
    std::vector<std::string> errors = getValidationErrors(root);
    return errors.empty();
}

std::vector<std::string> PSITreeOperations::getValidationErrors(PSINode* root) {
    std::vector<std::string> errors;
    validateNode(root, errors);
    return errors;
}

double PSITreeOperations::calculateSimilarity(PSINode* tree1, PSINode* tree2) {
    if (!tree1 || !tree2) {
        return 0.0;
    }

    auto size1 = getSubtreeSize(tree1);
    auto size2 = getSubtreeSize(tree2);

    if (size1 == 0 || size2 == 0) {
        return 0.0;
    }

    // 简单的相似度计算：基于结构相似性
    double node_similarity = calculateNodeSimilarity(tree1, tree2);
    double size_similarity = 1.0 - std::abs(static_cast<double>(size1 - size2)) / std::max(size1, size2);

    return (node_similarity + size_similarity) / 2.0;
}

std::vector<PSINode*> PSITreeOperations::findDifferences(PSINode* tree1, PSINode* tree2) {
    std::vector<PSINode*> differences;

    if (!tree1 || !tree2) {
        return differences;
    }

    // 简单的差异查找：比较节点类型和名称
    auto nodes1 = getAllDescendants(tree1);
    auto nodes2 = getAllDescendants(tree2);

    for (auto* node1 : nodes1) {
        bool found = false;
        for (auto* node2 : nodes2) {
            if (node1->getType() == node2->getType() &&
                node1->getText() == node2->getText()) {
                found = true;
                break;
            }
        }
        if (!found) {
            differences.push_back(node1);
        }
    }

    return differences;
}

void PSITreeOperations::optimizeTree(PSINode* root) {
    // 简单的树优化：移除空节点
    auto empty_nodes = findNodesByCondition(root, [](PSINode* node) {
        return node->getChildren().empty() &&
               node->getType() != PSINodeType::VARIABLE &&
               node->getType() != PSINodeType::FUNCTION;
    });

    for (auto* node : empty_nodes) {
        if (node->getParent()) { // 不删除根节点
            removeNode(node, false);
        }
    }
}

std::shared_ptr<PSINode> PSITreeOperations::cloneSubtree(PSINode* root) {
    if (!root) {
        return nullptr;
    }

    // 创建新节点（简化版本，实际中需要根据具体类型创建）
    auto clone = std::make_shared<PSINode>(root->getType(), root->getText(), root->getLocation());

    // 复制语义信息
    // （在实际实现中需要更复杂的语义信息复制逻辑）

    // 递归复制子节点
    for (const auto& child : root->getChildren()) {
        auto child_clone = cloneSubtree(child.get());
        if (child_clone) {
            clone->addChild(child_clone);
        }
    }

    return clone;
}

// 私有辅助方法实现
void PSITreeOperations::collectNodesByType(PSINode* node, PSINodeType type, std::vector<PSINode*>& result) {
    if (!node) {
        return;
    }

    if (node->getType() == type) {
        result.push_back(node);
    }

    for (const auto& child : node->getChildren()) {
        collectNodesByType(child.get(), type, result);
    }
}

void PSITreeOperations::collectNodesByCondition(
    PSINode* node,
    const std::function<bool(PSINode*)>& condition,
    std::vector<PSINode*>& result) {
    if (!node) {
        return;
    }

    if (condition(node)) {
        result.push_back(node);
    }

    for (const auto& child : node->getChildren()) {
        collectNodesByCondition(child.get(), condition, result);
    }
}

void PSITreeOperations::collectDescendants(PSINode* node, std::vector<PSINode*>& result) {
    if (!node) {
        return;
    }

    for (const auto& child : node->getChildren()) {
        result.push_back(child.get());
        collectDescendants(child.get(), result);
    }
}

void PSITreeOperations::buildNodePath(PSINode* node, std::string& path) {
    if (!node) {
        return;
    }

    if (node->getParent()) {
        buildNodePath(node->getParent(), path);
        path += "/";
    }

    path += node->getText();
}

int PSITreeOperations::calculateDepth(PSINode* node) {
    if (!node || node->getChildren().empty()) {
        return 1;
    }

    int max_depth = 0;
    for (const auto& child : node->getChildren()) {
        max_depth = std::max(max_depth, calculateDepth(child.get()));
    }

    return max_depth + 1;
}

int PSITreeOperations::calculateSubtreeSize(PSINode* node) {
    if (!node) {
        return 0;
    }

    int size = 1; // 当前节点
    for (const auto& child : node->getChildren()) {
        size += calculateSubtreeSize(child.get());
    }

    return size;
}

void PSITreeOperations::collectLeafNodes(PSINode* node, std::vector<PSINode*>& result) {
    if (!node) {
        return;
    }

    if (node->getChildren().empty()) {
        result.push_back(node);
    } else {
        for (const auto& child : node->getChildren()) {
            collectLeafNodes(child.get(), result);
        }
    }
}

void PSITreeOperations::collectBranchNodes(PSINode* node, std::vector<PSINode*>& result) {
    if (!node) {
        return;
    }

    if (!node->getChildren().empty()) {
        result.push_back(node);
        for (const auto& child : node->getChildren()) {
            collectBranchNodes(child.get(), result);
        }
    }
}

bool PSITreeOperations::validateNode(PSINode* node, std::vector<std::string>& errors) {
    if (!node) {
        errors.push_back("Null node found");
        return false;
    }

    // 检查父节点一致性
    for (const auto& child : node->getChildren()) {
        if (child->getParent() != node) {
            errors.push_back("Parent-child inconsistency at node: " + node->getText());
        }
        validateNode(child.get(), errors);
    }

    return errors.empty();
}

double PSITreeOperations::calculateNodeSimilarity(PSINode* node1, PSINode* node2) {
    if (!node1 || !node2) {
        return 0.0;
    }

    if (node1->getType() != node2->getType()) {
        return 0.0;
    }

    double similarity = 0.0;

    // 基于名称的相似度
    if (node1->getText() == node2->getText()) {
        similarity += 0.5;
    }

    // 基于子节点数量的相似度
    auto size1 = node1->getChildren().size();
    auto size2 = node2->getChildren().size();
    if (size1 == size2) {
        similarity += 0.3;
    }

    // 基于子节点结构的相似度
    double child_similarity = 0.0;
    size_t min_children = std::min(size1, size2);
    for (size_t i = 0; i < min_children; ++i) {
        child_similarity += calculateNodeSimilarity(
            node1->getChildren()[i].get(),
            node2->getChildren()[i].get()
        );
    }

    if (min_children > 0) {
        child_similarity /= min_children;
        similarity += child_similarity * 0.2;
    }

    return similarity;
}

// PSITreeQuery 实现
PSITreeQuery::PSITreeQuery(PSINode* root) : root_(root) {}

PSITreeQuery& PSITreeQuery::ofType(PSINodeType type) {
    filters_.push_back([type](PSINode* node) {
        return node->getType() == type;
    });
    return *this;
}

PSITreeQuery& PSITreeQuery::withName(const std::string& name) {
    filters_.push_back([name](PSINode* node) {
        return node->getText() == name;
    });
    return *this;
}

PSITreeQuery& PSITreeQuery::inFile(const std::string& file_path) {
    filters_.push_back([file_path](PSINode* node) {
        return node->getLocation().file_path == file_path;
    });
    return *this;
}

PSITreeQuery& PSITreeQuery::inLineRange(int start, int end) {
    filters_.push_back([start, end](PSINode* node) {
        int line = node->getLocation().line;
        return line >= start && line <= end;
    });
    return *this;
}

PSITreeQuery& PSITreeQuery::withSemanticInfo(const std::string& key, const std::string& value) {
    filters_.push_back([key, value](PSINode* node) {
        return node->getSemanticInfo(key) == value;
    });
    return *this;
}

PSITreeQuery& PSITreeQuery::isLeaf() {
    filters_.push_back([](PSINode* node) {
        return node->getChildren().empty();
    });
    return *this;
}

PSITreeQuery& PSITreeQuery::isRoot() {
    filters_.push_back([](PSINode* node) {
        return node->getParent() == nullptr;
    });
    return *this;
}

std::vector<PSINode*> PSITreeQuery::execute() {
    std::vector<PSINode*> result;

    if (!root_) {
        return result;
    }

    PSITreeOperations ops;
    return ops.findNodesByCondition(root_, [this](PSINode* node) {
        return matchesFilters(node);
    });
}

PSINode* PSITreeQuery::first() {
    auto results = execute();
    return results.empty() ? nullptr : results[0];
}

size_t PSITreeQuery::count() {
    return execute().size();
}

std::unordered_map<std::string, size_t> PSITreeQuery::groupByType() {
    auto results = execute();
    std::unordered_map<std::string, size_t> groups;

    for (auto* node : results) {
        std::string type_name = "Unknown";
        switch (node->getType()) {
            case PSINodeType::CLASS: type_name = "Class"; break;
            case PSINodeType::FUNCTION: type_name = "Function"; break;
            case PSINodeType::VARIABLE: type_name = "Variable"; break;
            case PSINodeType::NAMESPACE: type_name = "Namespace"; break;
            default: break;
        }
        groups[type_name]++;
    }

    return groups;
}

std::unordered_map<std::string, size_t> PSITreeQuery::groupByName() {
    auto results = execute();
    std::unordered_map<std::string, size_t> groups;

    for (auto* node : results) {
        groups[node->getText()]++;
    }

    return groups;
}

bool PSITreeQuery::matchesFilters(PSINode* node) {
    for (const auto& filter : filters_) {
        if (!filter(node)) {
            return false;
        }
    }
    return true;
}

// PSITreeAnalyzer 实现（简化版本）
PSITreeAnalyzer::ComplexityMetrics PSITreeAnalyzer::analyzeComplexity(PSINode* root) {
    ComplexityMetrics metrics;

    if (!root) {
        return metrics;
    }

    // 简化的复杂度计算
    PSITreeOperations ops;
    auto functions = ops.findAllNodes(root, PSINodeType::FUNCTION);
    auto classes = ops.findAllNodes(root, PSINodeType::CLASS);

    metrics.cyclomatic_complexity = functions.size(); // 简化计算
    metrics.number_of_children = root->getChildren().size();
    metrics.lines_of_code = ops.getSubtreeSize(root) * 5; // 估算

    return metrics;
}

void PSITreeAnalyzer::printMetrics(PSINode* root) {
    auto metrics = analyzeComplexity(root);

    std::cout << "=== PSI Tree Analysis Metrics ===" << std::endl;
    std::cout << "Cyclomatic Complexity: " << metrics.cyclomatic_complexity << std::endl;
    std::cout << "Number of Children: " << metrics.number_of_children << std::endl;
    std::cout << "Lines of Code: " << metrics.lines_of_code << std::endl;
    std::cout << "Maintainability Index: " << metrics.maintainability_index << std::endl;
    std::cout << "=================================" << std::endl;
}

}