#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace stub_index {

// Stub条目类型
enum class StubType {
    CLASS,
    FUNCTION,
    VARIABLE,
    NAMESPACE,
    ENUM,
    TYPEDEF
};

// 位置信息
struct SourceLocation {
    std::string file_path;
    int line;
    int column;

    SourceLocation(const std::string& path, int l, int c)
        : file_path(path), line(l), column(c) {}
};

// Stub条目基类
class StubEntry {
public:
    StubEntry(StubType type, const std::string& name, const SourceLocation& loc)
        : type_(type), name_(name), location_(loc) {}

    virtual ~StubEntry() = default;

    StubType getType() const { return type_; }
    const std::string& getName() const { return name_; }
    const SourceLocation& getLocation() const { return location_; }

    virtual std::string toString() const = 0;

private:
    StubType type_;
    std::string name_;
    SourceLocation location_;
};

// 类Stub条目
class ClassStub : public StubEntry {
public:
    ClassStub(const std::string& name, const SourceLocation& loc, bool is_struct = false)
        : StubEntry(StubType::CLASS, name, loc), is_struct_(is_struct) {}

    bool isStruct() const { return is_struct_; }

    std::string toString() const override {
        return std::string("Class ") + getName() + " at " + getLocation().file_path + ":" +
               std::to_string(getLocation().line);
    }

private:
    bool is_struct_;
};

// 函数参数信息
struct Parameter {
    std::string type;
    std::string name;

    Parameter(const std::string& t, const std::string& n) : type(t), name(n) {}
};

// 函数Stub条目
class FunctionStub : public StubEntry {
public:
    FunctionStub(const std::string& name, const SourceLocation& loc,
                const std::string& return_type = "void")
        : StubEntry(StubType::FUNCTION, name, loc), return_type_(return_type) {}

    void addParameter(const std::string& type, const std::string& name) {
        parameters_.emplace_back(type, name);
    }

    const std::string& getReturnType() const { return return_type_; }
    const std::vector<Parameter>& getParameters() const { return parameters_; }

    std::string toString() const override {
        std::string result = "Function " + getReturnType() + " " + getName() + "(";
        for (size_t i = 0; i < parameters_.size(); ++i) {
            if (i > 0) result += ", ";
            result += parameters_[i].type + " " + parameters_[i].name;
        }
        result += ") at " + getLocation().file_path + ":" + std::to_string(getLocation().line);
        return result;
    }

private:
    std::string return_type_;
    std::vector<Parameter> parameters_;
};

// 变量Stub条目
class VariableStub : public StubEntry {
public:
    VariableStub(const std::string& name, const SourceLocation& loc,
                 const std::string& var_type, bool is_const = false, bool is_static = false)
        : StubEntry(StubType::VARIABLE, name, loc),
          var_type_(var_type), is_const_(is_const), is_static_(is_static) {}

    const std::string& getVariableType() const { return var_type_; }
    bool isConst() const { return is_const_; }
    bool isStatic() const { return is_static_; }

    std::string toString() const override {
        std::string result = "Variable ";
        if (is_const_) result += "const ";
        if (is_static_) result += "static ";
        result += var_type_ + " " + getName();
        result += " at " + getLocation().file_path + ":" + std::to_string(getLocation().line);
        return result;
    }

private:
    std::string var_type_;
    bool is_const_;
    bool is_static_;
};

}