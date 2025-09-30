#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "stub_entry.h"

namespace stub_index {

// PSI节点类型枚举
enum class PSINodeType {
    // 文件结构
    FILE,
    NAMESPACE,
    CLASS,
    STRUCT,
    FUNCTION,
    VARIABLE,
    ENUM,
    TYPEDEF,

    // 语句
    COMPOUND_STATEMENT,
    IF_STATEMENT,
    FOR_STATEMENT,
    WHILE_STATEMENT,
    RETURN_STATEMENT,
    EXPRESSION_STATEMENT,
    DECLARATION_STATEMENT,

    // 表达式
    BINARY_EXPRESSION,
    UNARY_EXPRESSION,
    CALL_EXPRESSION,
    MEMBER_EXPRESSION,
    LITERAL_EXPRESSION,
    IDENTIFIER_EXPRESSION,

    // 类型
    BUILTIN_TYPE,
    QUALIFIED_TYPE,
    POINTER_TYPE,
    REFERENCE_TYPE,
    ARRAY_TYPE,

    // 其他
    COMMENT,
    PREPROCESSOR_DIRECTIVE,
    UNKNOWN
};

// PSI节点基类
class PSINode {
public:
    PSINode(PSINodeType type, const std::string& text, const SourceLocation& location);
    virtual ~PSINode() = default;

    // 基本属性访问
    PSINodeType getType() const { return type_; }
    const std::string& getText() const { return text_; }
    const SourceLocation& getLocation() const { return location_; }

    // 树结构操作
    PSINode* getParent() const { return parent_; }
    const std::vector<std::shared_ptr<PSINode>>& getChildren() const { return children_; }

    void setParent(PSINode* parent) { parent_ = parent; }
    void addChild(std::shared_ptr<PSINode> child);
    void removeChild(size_t index);
    void clearChildren();

    // 树遍历
    PSINode* getFirstChild() const;
    PSINode* getLastChild() const;
    PSINode* getNextSibling() const;
    PSINode* getPrevSibling() const;

    // 查找操作
    std::vector<PSINode*> findChildren(PSINodeType type) const;
    PSINode* findFirstChild(PSINodeType type) const;
    PSINode* findLastChild(PSINodeType type) const;

    // 文本范围
    struct TextRange {
        size_t start_offset;
        size_t end_offset;

        TextRange(size_t start, size_t end) : start_offset(start), end_offset(end) {}
        size_t getLength() const { return end_offset - start_offset; }
        bool contains(size_t offset) const { return offset >= start_offset && offset <= end_offset; }
    };

    const TextRange& getTextRange() const { return text_range_; }
    void setTextRange(const TextRange& range) { text_range_ = range; }

    // 语义信息
    void setSemanticInfo(const std::string& key, const std::string& value);
    std::string getSemanticInfo(const std::string& key) const;
    bool hasSemanticInfo(const std::string& key) const;
    const std::unordered_map<std::string, std::string>& getSemanticInfo() const;

    // 虚拟接口
    virtual std::string toString() const;
    virtual bool isValid() const { return true; }
    virtual void accept(class PSIVisitor* visitor);

protected:
    PSINodeType type_;
    std::string text_;
    SourceLocation location_;
    TextRange text_range_;

    PSINode* parent_;
    std::vector<std::shared_ptr<PSINode>> children_;

    // 语义信息存储
    std::unordered_map<std::string, std::string> semantic_info_;
};

// 文件节点（根节点）
class PSIFileNode : public PSINode {
public:
    PSIFileNode(const std::string& file_path, const std::string& content);

    const std::string& getFilePath() const { return file_path_; }
    const std::string& getContent() const { return content_; }

    std::string toString() const override;
    void accept(PSIVisitor* visitor) override;

private:
    std::string file_path_;
    std::string content_;
};

// 命名空间节点
class PSINamespaceNode : public PSINode {
public:
    PSINamespaceNode(const std::string& name, const SourceLocation& location);

    const std::string& getName() const { return name_; }

    std::string toString() const override;
    void accept(PSIVisitor* visitor) override;

private:
    std::string name_;
};

// 类节点
class PSIClassNode : public PSINode {
public:
    PSIClassNode(const std::string& name, const SourceLocation& location, bool is_struct = false);

    const std::string& getName() const { return name_; }
    bool isStruct() const { return is_struct_; }
    bool isAbstract() const { return is_abstract_; }
    void setAbstract(bool abstract) { is_abstract_ = abstract; }

    std::string toString() const override;
    void accept(PSIVisitor* visitor) override;

private:
    std::string name_;
    bool is_struct_;
    bool is_abstract_;
};

// 函数节点
class PSIFunctionNode : public PSINode {
public:
    struct Parameter {
        std::string type;
        std::string name;
        std::string default_value;

        Parameter(const std::string& t, const std::string& n, const std::string& dv = "")
            : type(t), name(n), default_value(dv) {}
    };

    PSIFunctionNode(const std::string& name, const SourceLocation& location, const std::string& return_type = "void");

    const std::string& getName() const { return name_; }
    const std::string& getReturnType() const { return return_type_; }
    const std::vector<Parameter>& getParameters() const { return parameters_; }
    bool isVirtual() const { return is_virtual_; }
    bool isStatic() const { return is_static_; }
    bool isConst() const { return is_const_; }
    bool isOverride() const { return is_override_; }

    void addParameter(const std::string& type, const std::string& name, const std::string& default_value = "");
    void setVirtual(bool v) { is_virtual_ = v; }
    void setStatic(bool s) { is_static_ = s; }
    void setConst(bool c) { is_const_ = c; }
    void setOverride(bool o) { is_override_ = o; }

    std::string toString() const override;
    void accept(PSIVisitor* visitor) override;

private:
    std::string name_;
    std::string return_type_;
    std::vector<Parameter> parameters_;
    bool is_virtual_;
    bool is_static_;
    bool is_const_;
    bool is_override_;
};

// 变量节点
class PSIVariableNode : public PSINode {
public:
    PSIVariableNode(const std::string& name, const SourceLocation& location, const std::string& var_type);

    const std::string& getName() const { return name_; }
    const std::string& getVariableType() const { return var_type_; }
    bool isConst() const { return is_const_; }
    bool isStatic() const { return is_static_; }
    bool isMember() const { return is_member_; }
    bool isParameter() const { return is_parameter_; }

    void setConst(bool c) { is_const_ = c; }
    void setStatic(bool s) { is_static_ = s; }
    void setMember(bool m) { is_member_ = m; }
    void setParameter(bool p) { is_parameter_ = p; }

    std::string toString() const override;
    void accept(PSIVisitor* visitor) override;

private:
    std::string name_;
    std::string var_type_;
    bool is_const_;
    bool is_static_;
    bool is_member_;
    bool is_parameter_;
};

}