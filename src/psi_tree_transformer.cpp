#include "psi_tree_operations.h"
#include <algorithm>
#include <stack>
#include <cctype>

namespace stub_index {

// PSITreeTransformer 实现
std::shared_ptr<PSINode> PSITreeTransformer::transformTree(PSINode* root,
                                                          const std::function<std::shared_ptr<PSINode>(PSINode*)>& transformer) {
    if (!root) {
        return nullptr;
    }

    return transformNode(root, transformer);
}

std::shared_ptr<PSINode> PSITreeTransformer::simplifyTree(PSINode* root) {
    // 简化转换器：移除不必要的节点，保留核心结构
    auto simplify_transformer = [](PSINode* node) -> std::shared_ptr<PSINode> {
        // 保留文件、类、函数、变量节点
        switch (node->getType()) {
            case PSINodeType::FILE:
            case PSINodeType::CLASS:
            case PSINodeType::FUNCTION:
            case PSINodeType::VARIABLE:
                return std::make_shared<PSINode>(node->getType(), node->getText(), node->getLocation());
            default:
                return nullptr; // 移除其他类型的节点
        }
    };

    return transformTree(root, simplify_transformer);
}

std::shared_ptr<PSINode> PSITreeTransformer::removeNodesByType(PSINode* root, PSINodeType type_to_remove) {
    auto remove_transformer = [type_to_remove](PSINode* node) -> std::shared_ptr<PSINode> {
        if (node->getType() == type_to_remove) {
            return nullptr; // 移除指定类型的节点
        }

        // 保留其他节点
        auto new_node = std::make_shared<PSINode>(node->getType(), node->getText(), node->getLocation());

        // 复制语义信息
        for (const auto& [key, value] : node->getSemanticInfo()) {
            new_node->setSemanticInfo(key, value);
        }

        return new_node;
    };

    return transformTree(root, remove_transformer);
}

std::shared_ptr<PSINode> PSITreeTransformer::reorganizeByNamespace(PSINode* root) {
    // 简化版本：按名称重新组织节点
    auto namespace_transformer = [](PSINode* node) -> std::shared_ptr<PSINode> {
        auto new_node = std::make_shared<PSINode>(node->getType(), node->getText(), node->getLocation());

        // 复制语义信息
        for (const auto& [key, value] : node->getSemanticInfo()) {
            new_node->setSemanticInfo(key, value);
        }

        return new_node;
    };

    return transformTree(root, namespace_transformer);
}

std::shared_ptr<PSINode> PSITreeTransformer::flattenHierarchy(PSINode* root, int max_depth) {
    auto flatten_transformer = [max_depth](PSINode* node) -> std::shared_ptr<PSINode> {
        // 简化版本：直接复制节点
        auto new_node = std::make_shared<PSINode>(node->getType(), node->getText(), node->getLocation());

        // 复制语义信息
        for (const auto& [key, value] : node->getSemanticInfo()) {
            new_node->setSemanticInfo(key, value);
        }

        return new_node;
    };

    return transformTree(root, flatten_transformer);
}

std::shared_ptr<PSINode> PSITreeTransformer::mergeTrees(const std::vector<PSINode*>& trees) {
    if (trees.empty()) {
        return nullptr;
    }

    // 创建合并后的根节点
    auto merged_root = std::make_shared<PSINode>(PSINodeType::FILE, "merged", SourceLocation("merged", 0, 0));

    // 合并所有树的子节点
    for (auto* tree : trees) {
        if (tree) {
            for (const auto& child : tree->getChildren()) {
                auto cloned_child = cloneAndSimplify(child.get());
                if (cloned_child) {
                    merged_root->addChild(cloned_child);
                }
            }
        }
    }

    return merged_root;
}

std::shared_ptr<PSINode> PSITreeTransformer::overlayTrees(PSINode* base_tree, PSINode* overlay_tree) {
    if (!base_tree) {
        return nullptr;
    }

    if (!overlay_tree) {
        return cloneAndSimplify(base_tree);
    }

    // 简化版本：创建覆盖后的树
    auto overlay_transformer = [](PSINode* node) -> std::shared_ptr<PSINode> {
        auto new_node = std::make_shared<PSINode>(node->getType(), node->getText(), node->getLocation());

        // 复制语义信息
        for (const auto& [key, value] : node->getSemanticInfo()) {
            new_node->setSemanticInfo(key, value);
        }

        return new_node;
    };

    return transformTree(base_tree, overlay_transformer);
}

std::shared_ptr<PSINode> PSITreeTransformer::transformNode(PSINode* node,
                                                           const std::function<std::shared_ptr<PSINode>(PSINode*)>& transformer) {
    if (!node) {
        return nullptr;
    }

    // 应用转换器到当前节点
    auto transformed_node = transformer(node);
    if (!transformed_node) {
        return nullptr; // 转换器决定移除此节点
    }

    // 递归转换子节点
    for (const auto& child : node->getChildren()) {
        auto transformed_child = transformNode(child.get(), transformer);
        if (transformed_child) {
            transformed_node->addChild(transformed_child);
        }
    }

    return transformed_node;
}

bool PSITreeTransformer::shouldKeepNode(PSINode* node, PSINodeType type_to_remove) {
    return node->getType() != type_to_remove;
}

std::shared_ptr<PSINode> PSITreeTransformer::cloneAndSimplify(PSINode* node) {
    if (!node) {
        return nullptr;
    }

    // 创建副本
    auto clone = std::make_shared<PSINode>(node->getType(), node->getText(), node->getLocation());

    // 复制语义信息
    for (const auto& [key, value] : node->getSemanticInfo()) {
        clone->setSemanticInfo(key, value);
    }

    // 递归复制子节点
    for (const auto& child : node->getChildren()) {
        auto cloned_child = cloneAndSimplify(child.get());
        if (cloned_child) {
            clone->addChild(cloned_child);
        }
    }

    return clone;
}

}