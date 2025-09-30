#pragma once
#include "stub_entry.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace stub_index {

// 索引查询结果
struct QueryResult {
    std::vector<std::shared_ptr<StubEntry>> entries;

    size_t size() const { return entries.size(); }
    bool empty() const { return entries.empty(); }

    void addEntry(std::shared_ptr<StubEntry> entry) {
        entries.push_back(entry);
    }
};

// 查询过滤器
struct QueryFilter {
    StubType type_filter;
    std::string name_pattern;
    std::string file_pattern;

    QueryFilter(StubType type = StubType::CLASS,
               const std::string& name = "",
               const std::string& file = "")
        : type_filter(type), name_pattern(name), file_pattern(file) {}
};

// Stub索引核心类
class StubIndex {
public:
    StubIndex() = default;
    ~StubIndex() = default;

    // 添加Stub条目
    void addEntry(std::shared_ptr<StubEntry> entry);

    // 根据名称查询
    QueryResult queryByName(const std::string& name) const;

    // 根据类型查询
    QueryResult queryByType(StubType type) const;

    // 根据文件路径查询
    QueryResult queryByFile(const std::string& file_path) const;

    // 复合查询
    QueryResult query(const QueryFilter& filter) const;

    // 获取所有条目
    QueryResult getAllEntries() const;

    // 清空索引
    void clear();

    // 获取索引大小
    size_t size() const;
    bool empty() const;

private:
    // 主要索引：名称 -> 条目列表
    std::unordered_map<std::string, std::vector<std::shared_ptr<StubEntry>>> name_index_;

    // 类型索引：类型 -> 条目列表
    std::unordered_map<StubType, std::vector<std::shared_ptr<StubEntry>>> type_index_;

    // 文件索引：文件路径 -> 条目列表
    std::unordered_map<std::string, std::vector<std::shared_ptr<StubEntry>>> file_index_;

    // 所有条目
    std::vector<std::shared_ptr<StubEntry>> all_entries_;
};

}