#include "psi_node.h"
#include "psi_visitor.h"
#include <iostream>
#include <algorithm>

namespace stub_index {

// PSINode基类实现
PSINode::PSINode(PSINodeType type, const std::string& text, const SourceLocation& location)
    : type_(type), text_(text), location_(location), parent_(nullptr), text_range_(0, 0) {
}

void PSINode::addChild(std::shared_ptr<PSINode> child) {
    if (child) {
        child->setParent(this);
        children_.push_back(child);
    }
}

void PSINode::removeChild(size_t index) {
    if (index < children_.size()) {
        children_[index]->setParent(nullptr);
        children_.erase(children_.begin() + index);
    }
}

void PSINode::clearChildren() {
    for (auto& child : children_) {
        child->setParent(nullptr);
    }
    children_.clear();
}

PSINode* PSINode::getFirstChild() const {
    return children_.empty() ? nullptr : children_[0].get();
}

PSINode* PSINode::getLastChild() const {
    return children_.empty() ? nullptr : children_.back().get();
}

PSINode* PSINode::getNextSibling() const {
    if (!parent_) return nullptr;

    const auto& siblings = parent_->getChildren();
    auto it = std::find_if(siblings.begin(), siblings.end(),
        [this](const std::shared_ptr<PSINode>& node) { return node.get() == this; });

    if (it != siblings.end() && ++it != siblings.end()) {
        return it->get();
    }
    return nullptr;
}

PSINode* PSINode::getPrevSibling() const {
    if (!parent_) return nullptr;

    const auto& siblings = parent_->getChildren();
    auto it = std::find_if(siblings.begin(), siblings.end(),
        [this](const std::shared_ptr<PSINode>& node) { return node.get() == this; });

    if (it != siblings.end() && it != siblings.begin()) {
        return (--it)->get();
    }
    return nullptr;
}

std::vector<PSINode*> PSINode::findChildren(PSINodeType type) const {
    std::vector<PSINode*> result;
    for (const auto& child : children_) {
        if (child->getType() == type) {
            result.push_back(child.get());
        }
    }
    return result;
}

PSINode* PSINode::findFirstChild(PSINodeType type) const {
    for (const auto& child : children_) {
        if (child->getType() == type) {
            return child.get();
        }
    }
    return nullptr;
}

PSINode* PSINode::findLastChild(PSINodeType type) const {
    for (auto it = children_.rbegin(); it != children_.rend(); ++it) {
        if ((*it)->getType() == type) {
            return it->get();
        }
    }
    return nullptr;
}

void PSINode::setSemanticInfo(const std::string& key, const std::string& value) {
    semantic_info_[key] = value;
}

std::string PSINode::getSemanticInfo(const std::string& key) const {
    auto it = semantic_info_.find(key);
    return it != semantic_info_.end() ? it->second : "";
}

bool PSINode::hasSemanticInfo(const std::string& key) const {
    return semantic_info_.find(key) != semantic_info_.end();
}

const std::unordered_map<std::string, std::string>& PSINode::getSemanticInfo() const {
    return semantic_info_;
}

std::string PSINode::toString() const {
    std::string type_name = "Unknown";
    switch (type_) {
        case PSINodeType::FILE: type_name = "File"; break;
        case PSINodeType::NAMESPACE: type_name = "Namespace"; break;
        case PSINodeType::CLASS: type_name = "Class"; break;
        case PSINodeType::STRUCT: type_name = "Struct"; break;
        case PSINodeType::FUNCTION: type_name = "Function"; break;
        case PSINodeType::VARIABLE: type_name = "Variable"; break;
        case PSINodeType::ENUM: type_name = "Enum"; break;
        case PSINodeType::TYPEDEF: type_name = "Typedef"; break;
        case PSINodeType::COMPOUND_STATEMENT: type_name = "CompoundStatement"; break;
        case PSINodeType::IF_STATEMENT: type_name = "IfStatement"; break;
        case PSINodeType::FOR_STATEMENT: type_name = "ForStatement"; break;
        case PSINodeType::WHILE_STATEMENT: type_name = "WhileStatement"; break;
        case PSINodeType::RETURN_STATEMENT: type_name = "ReturnStatement"; break;
        case PSINodeType::EXPRESSION_STATEMENT: type_name = "ExpressionStatement"; break;
        case PSINodeType::DECLARATION_STATEMENT: type_name = "DeclarationStatement"; break;
        case PSINodeType::BINARY_EXPRESSION: type_name = "BinaryExpression"; break;
        case PSINodeType::UNARY_EXPRESSION: type_name = "UnaryExpression"; break;
        case PSINodeType::CALL_EXPRESSION: type_name = "CallExpression"; break;
        case PSINodeType::MEMBER_EXPRESSION: type_name = "MemberExpression"; break;
        case PSINodeType::LITERAL_EXPRESSION: type_name = "LiteralExpression"; break;
        case PSINodeType::IDENTIFIER_EXPRESSION: type_name = "IdentifierExpression"; break;
        case PSINodeType::BUILTIN_TYPE: type_name = "BuiltinType"; break;
        case PSINodeType::QUALIFIED_TYPE: type_name = "QualifiedType"; break;
        case PSINodeType::POINTER_TYPE: type_name = "PointerType"; break;
        case PSINodeType::REFERENCE_TYPE: type_name = "ReferenceType"; break;
        case PSINodeType::ARRAY_TYPE: type_name = "ArrayType"; break;
        case PSINodeType::COMMENT: type_name = "Comment"; break;
        case PSINodeType::PREPROCESSOR_DIRECTIVE: type_name = "PreprocessorDirective"; break;
        default: break;
    }
    return type_name + ": " + text_;
}

void PSINode::accept(PSIVisitor* visitor) {
    // 默认实现，子类可以重写
}

// PSIFileNode实现
PSIFileNode::PSIFileNode(const std::string& file_path, const std::string& content)
    : PSINode(PSINodeType::FILE, file_path, SourceLocation(file_path, 1, 1)),
      file_path_(file_path), content_(content) {
    setTextRange(TextRange(0, content.length()));
}

std::string PSIFileNode::toString() const {
    return "File: " + file_path_ + " (" + std::to_string(children_.size()) + " children)";
}

void PSIFileNode::accept(PSIVisitor* visitor) {
    if (visitor) {
        visitor->visitFile(this);
    }
}

// PSINamespaceNode实现
PSINamespaceNode::PSINamespaceNode(const std::string& name, const SourceLocation& location)
    : PSINode(PSINodeType::NAMESPACE, name, location), name_(name) {
}

std::string PSINamespaceNode::toString() const {
    return "Namespace: " + name_ + " (" + std::to_string(children_.size()) + " children)";
}

void PSINamespaceNode::accept(PSIVisitor* visitor) {
    if (visitor) {
        visitor->visitNamespace(this);
    }
}

// PSIClassNode实现
PSIClassNode::PSIClassNode(const std::string& name, const SourceLocation& location, bool is_struct)
    : PSINode(is_struct ? PSINodeType::STRUCT : PSINodeType::CLASS, name, location),
      name_(name), is_struct_(is_struct), is_abstract_(false) {
}

std::string PSIClassNode::toString() const {
    std::string prefix = is_struct_ ? "Struct" : "Class";
    if (is_abstract_) prefix = "Abstract " + prefix;
    return prefix + ": " + name_ + " (" + std::to_string(children_.size()) + " children)";
}

void PSIClassNode::accept(PSIVisitor* visitor) {
    if (visitor) {
        visitor->visitClass(this);
    }
}

// PSIFunctionNode实现
PSIFunctionNode::PSIFunctionNode(const std::string& name, const SourceLocation& location, const std::string& return_type)
    : PSINode(PSINodeType::FUNCTION, name, location),
      name_(name), return_type_(return_type),
      is_virtual_(false), is_static_(false), is_const_(false), is_override_(false) {
}

void PSIFunctionNode::addParameter(const std::string& type, const std::string& name, const std::string& default_value) {
    parameters_.emplace_back(type, name, default_value);
}

std::string PSIFunctionNode::toString() const {
    std::string result = return_type_ + " " + name_ + "(";
    for (size_t i = 0; i < parameters_.size(); ++i) {
        if (i > 0) result += ", ";
        result += parameters_[i].type + " " + parameters_[i].name;
        if (!parameters_[i].default_value.empty()) {
            result += " = " + parameters_[i].default_value;
        }
    }
    result += ")";

    if (is_const_) result += " const";
    if (is_virtual_) result = "virtual " + result;
    if (is_static_) result = "static " + result;
    if (is_override_) result += " override";

    return result;
}

void PSIFunctionNode::accept(PSIVisitor* visitor) {
    if (visitor) {
        visitor->visitFunction(this);
    }
}

// PSIVariableNode实现
PSIVariableNode::PSIVariableNode(const std::string& name, const SourceLocation& location, const std::string& var_type)
    : PSINode(PSINodeType::VARIABLE, name, location),
      name_(name), var_type_(var_type),
      is_const_(false), is_static_(false), is_member_(false), is_parameter_(false) {
}

std::string PSIVariableNode::toString() const {
    std::string result;
    if (is_const_) result += "const ";
    if (is_static_) result += "static ";
    result += var_type_ + " " + name_;
    return result;
}

void PSIVariableNode::accept(PSIVisitor* visitor) {
    if (visitor) {
        visitor->visitVariable(this);
    }
}

}