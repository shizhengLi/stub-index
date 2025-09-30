#pragma once
#include "stub_index.h"
#include <string>
#include <vector>
#include <memory>
#include <regex>

namespace stub_index {

// 解析结果
class ParseResult {
public:
    ParseResult() = default;
    ~ParseResult() = default;

    void addEntry(std::shared_ptr<StubEntry> entry) {
        entries_.push_back(entry);
    }

    const std::vector<std::shared_ptr<StubEntry>>& getEntries() const {
        return entries_;
    }

    size_t size() const { return entries_.size(); }
    bool empty() const { return entries_.empty(); }

    void clear() { entries_.clear(); }

private:
    std::vector<std::shared_ptr<StubEntry>> entries_;
};

// C++代码解析器
class StubParser {
public:
    StubParser() = default;
    ~StubParser() = default;

    // 解析单个文件
    ParseResult parseFile(const std::string& file_path, const std::string& content);

    // 解析代码字符串
    ParseResult parseCode(const std::string& code, const std::string& file_path = "<memory>");

    // 设置解析选项
    void setParseFunctions(bool enable) { parse_functions_ = enable; }
    void setParseVariables(bool enable) { parse_variables_ = enable; }
    void setParseClasses(bool enable) { parse_classes_ = enable; }

private:
    // 解析类定义
    void parseClass(const std::string& code, const std::string& file_path, ParseResult& result);

    // 解析函数定义
    void parseFunction(const std::string& code, const std::string& file_path, ParseResult& result);

    // 解析变量声明
    void parseVariable(const std::string& code, const std::string& file_path, ParseResult& result);

    // 查找模式匹配
    std::vector<std::pair<size_t, size_t>> findPatternMatches(
        const std::string& code, const std::regex& pattern);

    // 获取行号
    int getLineNumber(const std::string& code, size_t pos);

    // 解析选项
    bool parse_functions_ = true;
    bool parse_variables_ = true;
    bool parse_classes_ = true;
};

}