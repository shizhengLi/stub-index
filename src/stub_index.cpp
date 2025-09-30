#include "stub_index.h"

namespace stub_index {

void StubIndex::addEntry(std::shared_ptr<StubEntry> entry) {
    if (!entry) return;

    // 添加到总列表
    all_entries_.push_back(entry);

    // 添加到名称索引
    name_index_[entry->getName()].push_back(entry);

    // 添加到类型索引
    type_index_[entry->getType()].push_back(entry);

    // 添加到文件索引
    file_index_[entry->getLocation().file_path].push_back(entry);
}

QueryResult StubIndex::queryByName(const std::string& name) const {
    QueryResult result;
    auto it = name_index_.find(name);
    if (it != name_index_.end()) {
        result.entries = it->second;
    }
    return result;
}

QueryResult StubIndex::queryByType(StubType type) const {
    QueryResult result;
    auto it = type_index_.find(type);
    if (it != type_index_.end()) {
        result.entries = it->second;
    }
    return result;
}

QueryResult StubIndex::queryByFile(const std::string& file_path) const {
    QueryResult result;
    auto it = file_index_.find(file_path);
    if (it != file_index_.end()) {
        result.entries = it->second;
    }
    return result;
}

QueryResult StubIndex::query(const QueryFilter& filter) const {
    QueryResult result;

    // 根据名称模式过滤
    if (!filter.name_pattern.empty()) {
        auto name_results = queryByName(filter.name_pattern);
        for (const auto& entry : name_results.entries) {
            // 检查类型匹配
            if (filter.type_filter != StubType::TYPEDEF && entry->getType() != filter.type_filter) {
                continue;
            }
            // 检查文件模式匹配
            if (!filter.file_pattern.empty() &&
                entry->getLocation().file_path.find(filter.file_pattern) == std::string::npos) {
                continue;
            }
            result.addEntry(entry);
        }
    } else if (filter.type_filter != StubType::CLASS) {
        // 仅按类型过滤
        auto type_results = queryByType(filter.type_filter);
        for (const auto& entry : type_results.entries) {
            if (!filter.file_pattern.empty() &&
                entry->getLocation().file_path.find(filter.file_pattern) == std::string::npos) {
                continue;
            }
            result.addEntry(entry);
        }
    } else if (!filter.file_pattern.empty()) {
        // 仅按文件过滤
        auto file_results = queryByFile(filter.file_pattern);
        for (const auto& entry : file_results.entries) {
            result.addEntry(entry);
        }
    } else {
        // 返回所有条目
        result.entries = all_entries_;
    }

    return result;
}

QueryResult StubIndex::getAllEntries() const {
    QueryResult result;
    result.entries = all_entries_;
    return result;
}

void StubIndex::clear() {
    name_index_.clear();
    type_index_.clear();
    file_index_.clear();
    all_entries_.clear();
}

size_t StubIndex::size() const {
    return all_entries_.size();
}

bool StubIndex::empty() const {
    return all_entries_.empty();
}

}