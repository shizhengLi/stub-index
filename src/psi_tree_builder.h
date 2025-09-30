#pragma once
#include "psi_node.h"
#include "stub_parser.h"
#include "stub_entry.h"
#include <memory>
#include <fstream>

namespace stub_index {

// PSI树构建器
class PSITreeBuilder {
public:
    PSITreeBuilder() = default;
    ~PSITreeBuilder() = default;

    // 构建PSI树的入口方法
    std::shared_ptr<PSIFileNode> buildTreeFromFile(const std::string& file_path);
    std::shared_ptr<PSIFileNode> buildTreeFromContent(const std::string& file_path, const std::string& content);

    // 设置构建选项
    void setIncludeComments(bool include) { include_comments_ = include; }
    void setIncludePreprocessor(bool include) { include_preprocessor_ = include; }
    void setDetailedExpressions(bool detailed) { detailed_expressions_ = detailed; }

private:
    // 构建选项
    bool include_comments_ = false;
    bool include_preprocessor_ = false;
    bool detailed_expressions_ = false;

    // 内部构建方法
    void buildClassStructure(PSINode* parent, const std::vector<std::shared_ptr<StubEntry>>& stubs);
    void buildFunctionStructure(PSINode* parent, const std::vector<std::shared_ptr<StubEntry>>& stubs);
    void buildVariableStructure(PSINode* parent, const std::vector<std::shared_ptr<StubEntry>>& stubs);

    // 辅助方法
    std::shared_ptr<PSIClassNode> createClassNode(const std::shared_ptr<StubEntry>& entry);
    std::shared_ptr<PSIFunctionNode> createFunctionNode(const std::shared_ptr<StubEntry>& entry);
    std::shared_ptr<PSIVariableNode> createVariableNode(const std::shared_ptr<StubEntry>& entry);
};

// PSI树工厂类
class PSITreeFactory {
public:
    static std::shared_ptr<PSIFileNode> createFromFile(const std::string& file_path);
    static std::shared_ptr<PSIFileNode> createFromContent(const std::string& file_path,
                                                         const std::string& content);
    static std::shared_ptr<PSIFileNode> createWithDetailedAnalysis(const std::string& file_path);
};

}