#include "psi_tree_builder.h"
#include "psi_tree_operations.h"
#include "psi_visitor.h"
#include <iostream>

using namespace stub_index;

int main() {
    std::string content = R"(
class Calculator {
public:
    int add(int a, int b);
    int multiply(int x, int y);
};

class Helper {
public:
    void help();
    void assist();
};

int global_var = 42;
)";

    PSITreeBuilder builder;
    auto tree = builder.buildTreeFromContent("test.cpp", content);

    // Print all nodes
    PrintVisitor visitor;
    tree->accept(&visitor);

    // Count nodes by type
    PSITreeOperations ops;
    auto classes = ops.findAllNodes(tree.get(), PSINodeType::CLASS);
    auto functions = ops.findAllNodes(tree.get(), PSINodeType::FUNCTION);
    auto variables = ops.findAllNodes(tree.get(), PSINodeType::VARIABLE);

    std::cout << "Found " << classes.size() << " classes" << std::endl;
    std::cout << "Found " << functions.size() << " functions" << std::endl;
    std::cout << "Found " << variables.size() << " variables" << std::endl;

    for (auto* node : classes) {
        std::cout << "Class: " << node->getText() << std::endl;
    }

    for (auto* node : functions) {
        std::cout << "Function: " << node->getText() << std::endl;
    }

    return 0;
}
