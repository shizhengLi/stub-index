#pragma once
#include "psi_node.h"
#include "psi_visitor.h"
#include <functional>
#include <vector>
#include <memory>
#include <unordered_map>

namespace stub_index {

// PSI树操作器 - 提供高级树操作功能
class PSITreeOperations {
public:
    PSITreeOperations() = default;
    ~PSITreeOperations() = default;

    // 基本查询操作
    std::vector<PSINode*> findAllNodes(PSINode* root, PSINodeType type);
    std::vector<PSINode*> findNodesByCondition(PSINode* root,
                                              const std::function<bool(PSINode*)>& condition);

    // 按名称查找
    std::vector<PSINode*> findNodesByName(PSINode* root, const std::string& name);
    PSINode* findFirstNodeByName(PSINode* root, const std::string& name);

    // 按位置查找
    std::vector<PSINode*> findNodesInFile(PSINode* root, const std::string& file_path);
    std::vector<PSINode*> findNodesInLineRange(PSINode* root, int start_line, int end_line);

    // 层次结构操作
    std::vector<PSINode*> getAllDescendants(PSINode* node);
    std::vector<PSINode*> getAncestors(PSINode* node);
    PSINode* findCommonAncestor(PSINode* node1, PSINode* node2);

    // 路径操作
    std::string getNodePath(PSINode* node);
    PSINode* findNodeByPath(PSINode* root, const std::string& path);

    // 树结构分析
    int getTreeDepth(PSINode* root);
    int getSubtreeSize(PSINode* node);
    std::vector<PSINode*> getLeafNodes(PSINode* root);
    std::vector<PSINode*> getBranchNodes(PSINode* root);

    // 树修改操作
    void removeNode(PSINode* node, bool keep_children = false);
    void moveNode(PSINode* node, PSINode* new_parent);
    void copyNode(PSINode* source, PSINode* target_parent);

    // 树验证
    bool validateTree(PSINode* root);
    std::vector<std::string> getValidationErrors(PSINode* root);

    // 树比较
    double calculateSimilarity(PSINode* tree1, PSINode* tree2);
    std::vector<PSINode*> findDifferences(PSINode* tree1, PSINode* tree2);

    // 性能优化操作
    void optimizeTree(PSINode* root);
    std::shared_ptr<PSINode> cloneSubtree(PSINode* root);

private:
    // 内部辅助方法
    void collectNodesByType(PSINode* node, PSINodeType type, std::vector<PSINode*>& result);
    void collectNodesByCondition(PSINode* node,
                                const std::function<bool(PSINode*)>& condition,
                                std::vector<PSINode*>& result);
    void collectDescendants(PSINode* node, std::vector<PSINode*>& result);
    void buildNodePath(PSINode* node, std::string& path);
    int calculateDepth(PSINode* node);
    int calculateSubtreeSize(PSINode* node);
    void collectLeafNodes(PSINode* node, std::vector<PSINode*>& result);
    void collectBranchNodes(PSINode* node, std::vector<PSINode*>& result);
    bool validateNode(PSINode* node, std::vector<std::string>& errors);
    double calculateNodeSimilarity(PSINode* node1, PSINode* node2);
};

// PSI树查询器 - 提供声明式查询接口
class PSITreeQuery {
public:
    PSITreeQuery(PSINode* root);
    ~PSITreeQuery() = default;

    // 流式查询接口
    PSITreeQuery& ofType(PSINodeType type);
    PSITreeQuery& withName(const std::string& name);
    PSITreeQuery& inFile(const std::string& file_path);
    PSITreeQuery& inLineRange(int start, int end);
    PSITreeQuery& withSemanticInfo(const std::string& key, const std::string& value);
    PSITreeQuery& isLeaf();
    PSITreeQuery& isRoot();

    // 执行查询
    std::vector<PSINode*> execute();
    PSINode* first();
    size_t count();

    // 聚合操作
    std::unordered_map<std::string, size_t> groupByType();
    std::unordered_map<std::string, size_t> groupByName();

private:
    PSINode* root_;
    std::vector<std::function<bool(PSINode*)>> filters_;

    bool matchesFilters(PSINode* node);
};

// PSI树分析器 - 提供代码分析功能
class PSITreeAnalyzer {
public:
    PSITreeAnalyzer() = default;
    ~PSITreeAnalyzer() = default;

    // 代码复杂度分析
    struct ComplexityMetrics {
        int cyclomatic_complexity = 0;
        int depth_of_inheritance = 0;
        int number_of_children = 0;
        int lines_of_code = 0;
        double maintainability_index = 0.0;
    };

    ComplexityMetrics analyzeComplexity(PSINode* root);
    ComplexityMetrics analyzeFunctionComplexity(PSIFunctionNode* func);

    // 依赖关系分析
    struct DependencyInfo {
        std::vector<std::string> includes;
        std::vector<std::string> forward_declarations;
        std::vector<std::string> type_dependencies;
        std::vector<std::string> function_calls;
    };

    DependencyInfo analyzeDependencies(PSINode* root);
    std::unordered_map<std::string, std::vector<std::string>> buildCallGraph(PSINode* root);

    // 代码质量分析
    struct QualityMetrics {
        int total_lines = 0;
        int comment_lines = 0;
        int function_count = 0;
        int class_count = 0;
        double comment_ratio = 0.0;
        double average_function_size = 0.0;
    };

    QualityMetrics analyzeQuality(PSINode* root);

    // 代码度量
    void printMetrics(PSINode* root);
    void exportMetrics(PSINode* root, const std::string& file_path);

private:
    // 内部分析方法
    int calculateCyclomaticComplexity(PSINode* node);
    int calculateInheritanceDepth(PSIClassNode* class_node);
    int countLinesOfCode(PSINode* node);
    double calculateMaintainabilityIndex(const ComplexityMetrics& metrics);
    void extractIncludes(PSINode* node, DependencyInfo& deps);
    void extractFunctionCalls(PSINode* node, DependencyInfo& deps);
    int countCommentLines(const std::string& content);
};

// PSI树转换器 - 提供树结构转换功能
class PSITreeTransformer {
public:
    PSITreeTransformer() = default;
    ~PSITreeTransformer() = default;

    // 树转换操作
    std::shared_ptr<PSINode> transformTree(PSINode* root,
                                          const std::function<std::shared_ptr<PSINode>(PSINode*)>& transformer);

    // 树简化
    std::shared_ptr<PSINode> simplifyTree(PSINode* root);
    std::shared_ptr<PSINode> removeNodesByType(PSINode* root, PSINodeType type_to_remove);

    // 树重构
    std::shared_ptr<PSINode> reorganizeByNamespace(PSINode* root);
    std::shared_ptr<PSINode> flattenHierarchy(PSINode* root, int max_depth = 2);

    // 树合并
    std::shared_ptr<PSINode> mergeTrees(const std::vector<PSINode*>& trees);
    std::shared_ptr<PSINode> overlayTrees(PSINode* base_tree, PSINode* overlay_tree);

private:
    std::shared_ptr<PSINode> transformNode(PSINode* node,
                                           const std::function<std::shared_ptr<PSINode>(PSINode*)>& transformer);
    bool shouldKeepNode(PSINode* node, PSINodeType type_to_remove);
    std::shared_ptr<PSINode> cloneAndSimplify(PSINode* node);
};

}