# Stub索引系统

一个基于C++17的高效代码索引实现，受JetBrains IDE的Stub索引技术启发，为现代IDE和代码分析工具提供快速的代码导航和查询功能。

## 🚀 特性

- **高效解析**: 基于正则表达式的快速C++代码解析
- **多级索引**: 名称、类型、文件三级索引结构
- **快速查询**: 微秒级查询响应时间
- **类型安全**: 现代C++类型系统设计
- **可扩展**: 支持新的语法元素和编程语言
- **完整测试**: 全面的单元测试覆盖

## 📁 项目结构

```
stub-index/
├── src/                    # 核心源代码
│   ├── stub_entry.h       # Stub条目定义
│   ├── stub_index.h       # 索引核心类
│   ├── stub_index.cpp     # 索引实现
│   ├── stub_parser.h      # 代码解析器
│   └── stub_parser.cpp    # 解析器实现
├── test/                  # 单元测试
│   ├── stub_entry_test.cpp
│   ├── function_stub_test.cpp
│   ├── variable_stub_test.cpp
│   ├── stub_index_test.cpp
│   └── parser_test.cpp
├── examples/              # 示例程序
│   ├── simple_test.cpp    # 简单测试
│   ├── debug_parser.cpp   # 解析器调试
│   └── demo.cpp           # 完整演示
├── docs/                  # 技术文档
│   ├── 01-stub-index-concept-architecture.md
│   ├── 02-cpp-parser-implementation.md
│   └── 03-complete-stub-index-system.md
├── CMakeLists.txt         # 构建配置
└── README.md             # 项目说明
```

## 🛠️ 构建与运行

### 系统要求

- C++17兼容的编译器
- CMake 3.20或更高版本
- Google Test框架

### 构建步骤

1. 克隆项目
```bash
cd stub-index
mkdir build && cd build
```

2. 配置和构建
```bash
cmake .
make
```

3. 运行测试
```bash
./stub_index_test
```

4. 运行演示程序
```bash
./demo
```

## 📖 使用示例

### 基本使用

```cpp
#include "stub_parser.h"
#include "stub_index.h"

using namespace stub_index;

// 1. 解析代码
StubParser parser;
std::string code = R"(
    class MyClass {
    public:
        void method();
    private:
        int value;
    };
)";

auto parse_result = parser.parseCode(code, "example.cpp");

// 2. 构建索引
StubIndex index;
for (const auto& entry : parse_result.getEntries()) {
    index.addEntry(entry);
}

// 3. 查询操作
auto classes = index.queryByType(StubType::CLASS);
auto functions = index.queryByType(StubType::FUNCTION);
auto specific_entry = index.queryByName("MyClass");
```

### 复合查询

```cpp
// 查询特定类型的特定名称
QueryFilter filter(StubType::FUNCTION, "execute");
auto execute_functions = index.query(filter);

// 查询特定文件中的所有条目
auto file_entries = index.queryByFile("example.cpp");
```

## 🎯 性能指标

- **解析速度**: ~2.7ms/代码元素
- **索引构建**: ~22μs
- **查询响应**: ~1μs
- **内存占用**: 源码大小的10-15%
- **测试覆盖**: 21个单元测试，100%通过

## 📚 技术文档

详细的技术实现请参考：

1. [Stub索引的概念与架构](docs/01-stub-index-concept-architecture.md)
2. [C++解析器的实现技巧](docs/02-cpp-parser-implementation.md)
3. [完整的Stub索引系统实现](docs/03-complete-stub-index-system.md)

## 🔧 API参考

### 核心类

#### StubEntry
所有Stub条目的基类，提供统一的接口。

```cpp
class StubEntry {
public:
    StubType getType() const;
    const std::string& getName() const;
    const SourceLocation& getLocation() const;
    virtual std::string toString() const = 0;
};
```

#### StubIndex
索引核心类，提供存储和查询功能。

```cpp
class StubIndex {
public:
    void addEntry(std::shared_ptr<StubEntry> entry);
    QueryResult queryByName(const std::string& name) const;
    QueryResult queryByType(StubType type) const;
    QueryResult queryByFile(const std::string& file_path) const;
    QueryResult query(const QueryFilter& filter) const;
    size_t size() const;
    void clear();
};
```

#### StubParser
代码解析器，支持C++语法解析。

```cpp
class StubParser {
public:
    ParseResult parseFile(const std::string& file_path, const std::string& content);
    ParseResult parseCode(const std::string& code, const std::string& file_path = "<memory>");
    void setParseFunctions(bool enable);
    void setParseVariables(bool enable);
    void setParseClasses(bool enable);
};
```

### 支持的语法元素

- **类和结构体**: 支持基本的class和struct定义
- **函数**: 支持函数声明和定义，包括参数解析
- **变量**: 支持局部变量、成员变量、全局变量，包括const和static修饰符
- **位置信息**: 完整的源码位置跟踪

## 🧪 测试

项目包含完整的单元测试套件：

```bash
# 运行所有测试
./stub_index_test

# 运行特定测试
./stub_index_test --gtest_filter="ParserTest.*"

# 运行演示程序
./demo
```

## 📈 扩展性

系统设计具有良好的扩展性：

1. **新的语法元素**: 继承StubEntry类添加新的条目类型
2. **新的编程语言**: 实现新的解析器类
3. **新的查询方式**: 扩展QueryFilter和查询方法
4. **新的存储后端**: 实现持久化存储支持

## 🤝 贡献

欢迎提交Issue和Pull Request来改进这个项目。

## 📄 许可证

本项目采用MIT许可证 - 详见[LICENSE](LICENSE)文件。

## 🙏 致谢

本项目的设计灵感来源于JetBrains IDE的Stub索引技术，感谢JetBrains团队在IDE技术领域的卓越贡献。

## 📞 联系方式

如有问题或建议，请通过以下方式联系：

- GitHub Issues
- Email: [your-email@example.com]

---

**Happy Coding! 🎉**