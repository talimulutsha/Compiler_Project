// ast.h
// The tree structure the Parser builds and the Evaluator walks.
// Kept super simple: every node is either a number, a variable,
// an assignment, or a binary operation (left OP right).

#ifndef AST_H
#define AST_H

#include <string>
#include <memory>

enum class NodeType { NUMBER, IDENTIFIER, BINOP, ASSIGN };

struct Node {
    NodeType type;

    double numValue = 0;        // used when type == NUMBER
    std::string name;           // used when type == IDENTIFIER / ASSIGN (var name)
    char op = 0;                // used when type == BINOP ('+','-','*','/')

    std::shared_ptr<Node> left;
    std::shared_ptr<Node> right; // for ASSIGN, "right" holds the expression

    Node(NodeType t) : type(t) {}
};

using NodePtr = std::shared_ptr<Node>;

#endif
