#include <iostream>
#include <chrono>
#include <string>
#include <vector>
#include "stub_parser.h"
#include "stub_index.h"

using namespace stub_index;

// 演示代码样本
const std::string sample_code = R"(
#include <iostream>
#include <string>
#include <vector>

// 数据库连接类
class DatabaseConnection {
public:
    static const int MAX_CONNECTIONS = 10;
    static const std::string DEFAULT_HOST;

    DatabaseConnection(const std::string& host, int port);
    virtual ~DatabaseConnection();

    bool connect();
    void disconnect();
    bool isConnected() const;

    virtual void executeQuery(const std::string& query) = 0;

protected:
    std::string host_;
    int port_;
    bool connected_;
};

const std::string DatabaseConnection::DEFAULT_HOST = "localhost";

DatabaseConnection::DatabaseConnection(const std::string& host, int port)
    : host_(host), port_(port), connected_(false) {
}

DatabaseConnection::~DatabaseConnection() {
    if (connected_) {
        disconnect();
    }
}

bool DatabaseConnection::connect() {
    std::cout << "Connecting to " << host_ << ":" << port_ << std::endl;
    connected_ = true;
    return true;
}

void DatabaseConnection::disconnect() {
    std::cout << "Disconnecting from " << host_ << std::endl;
    connected_ = false;
}

bool DatabaseConnection::isConnected() const {
    return connected_;
}

// MySQL连接实现
class MySQLConnection : public DatabaseConnection {
public:
    MySQLConnection(const std::string& host, int port);
    ~MySQLConnection() override;

    void executeQuery(const std::string& query) override;

private:
    void* mysql_handle_;
};

MySQLConnection::MySQLConnection(const std::string& host, int port)
    : DatabaseConnection(host, port), mysql_handle_(nullptr) {
}

MySQLConnection::~MySQLConnection() {
}

void MySQLConnection::executeQuery(const std::string& query) {
    std::cout << "MySQL executing: " << query << std::endl;
}

// 工具函数
std::string buildConnectionString(const std::string& host, int port, const std::string& database) {
    return host + ":" + std::to_string(port) + "/" + database;
}

// 全局变量
int global_connection_count = 0;
const std::string GLOBAL_CONFIG_PATH = "/etc/database/config.ini";
)";

void printQueryResults(const QueryResult& result, const std::string& title) {
    std::cout << "\n=== " << title << " ===" << std::endl;
    std::cout << "Found " << result.size() << " entries:" << std::endl;

    for (size_t i = 0; i < result.entries.size(); ++i) {
        const auto& entry = result.entries[i];
        std::cout << i + 1 << ". " << entry->toString() << std::endl;
    }
}

int main() {
    std::cout << "=== Stub索引系统演示 ===" << std::endl;

    // 1. 解析代码
    std::cout << "\n1. 解析代码..." << std::endl;
    auto start_time = std::chrono::high_resolution_clock::now();

    StubParser parser;
    auto parse_result = parser.parseCode(sample_code, "database.cpp");

    auto end_time = std::chrono::high_resolution_clock::now();
    auto parse_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

    std::cout << "解析完成，耗时: " << parse_duration.count() << " 微秒" << std::endl;
    std::cout << "解析出 " << parse_result.size() << " 个代码元素" << std::endl;

    // 2. 构建索引
    std::cout << "\n2. 构建索引..." << std::endl;
    start_time = std::chrono::high_resolution_clock::now();

    StubIndex index;
    for (const auto& entry : parse_result.getEntries()) {
        index.addEntry(entry);
    }

    end_time = std::chrono::high_resolution_clock::now();
    auto index_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

    std::cout << "索引构建完成，耗时: " << index_duration.count() << " 微秒" << std::endl;
    std::cout << "索引包含 " << index.size() << " 个条目" << std::endl;

    // 3. 查询演示
    std::cout << "\n3. 查询演示..." << std::endl;

    // 3.1 查询所有类
    auto classes = index.queryByType(StubType::CLASS);
    printQueryResults(classes, "所有类");

    // 3.2 查询所有函数
    auto functions = index.queryByType(StubType::FUNCTION);
    printQueryResults(functions, "所有函数");

    // 3.3 查询所有变量
    auto variables = index.queryByType(StubType::VARIABLE);
    printQueryResults(variables, "所有变量");

    // 3.4 按名称查询
    auto connection_queries = index.queryByName("Connection");
    printQueryResults(connection_queries, "名称包含'Connection'的元素");

    // 3.5 复合查询
    QueryFilter filter(StubType::FUNCTION, "execute");
    auto execute_functions = index.query(filter);
    printQueryResults(execute_functions, "名为'execute'的函数");

    // 3.6 查询静态常量
    QueryFilter const_filter(StubType::VARIABLE, "", "");
    auto const_vars = index.query(const_filter);
    std::cout << "\n=== 静态常量 ===" << std::endl;
    for (const auto& entry : const_vars.entries) {
        if (auto var_entry = std::dynamic_pointer_cast<VariableStub>(entry)) {
            if (var_entry->isConst() && var_entry->isStatic()) {
                std::cout << "- " << var_entry->toString() << std::endl;
            }
        }
    }

    // 4. 性能测试
    std::cout << "\n4. 性能测试..." << std::endl;

    // 4.1 查询性能测试
    const int test_count = 1000;
    start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < test_count; ++i) {
        index.queryByName("Database");
        index.queryByType(StubType::CLASS);
        index.queryByFile("database.cpp");
    }

    end_time = std::chrono::high_resolution_clock::now();
    auto query_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

    std::cout << "执行 " << test_count << " 次查询，总耗时: " << query_duration.count() << " 微秒" << std::endl;
    std::cout << "平均查询时间: " << query_duration.count() / test_count << " 微秒" << std::endl;

    // 5. 统计信息
    std::cout << "\n5. 统计信息..." << std::endl;
    std::cout << "总条目数: " << index.size() << std::endl;
    std::cout << "类数量: " << index.queryByType(StubType::CLASS).size() << std::endl;
    std::cout << "函数数量: " << index.queryByType(StubType::FUNCTION).size() << std::endl;
    std::cout << "变量数量: " << index.queryByType(StubType::VARIABLE).size() << std::endl;

    std::cout << "\n=== 演示完成 ===" << std::endl;
    return 0;
}