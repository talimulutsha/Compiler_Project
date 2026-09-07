// evaluator.cpp
// Module 4: Expression Evaluator (implementation)
// Simple recursive tree-walker: give it a node, it gives back a number.

#include "evaluator.h"
#include <stdexcept>

double Evaluator::evalNode(const NodePtr& node) {
    switch (node->type) {
        case NodeType::NUMBER:
            return node->numValue;

        case NodeType::IDENTIFIER:
            return getVariable(node->name);

        case NodeType::BINOP: {
            double l = evalNode(node->left);
            double r = evalNode(node->right);
            switch (node->op) {
                case '+': return l + r;
                case '-': return l - r;
                case '*': return l * r;
                case '/':
                    if (r == 0) throw std::runtime_error("Division by zero");
                    return l / r;
            }
            return 0;
        }

        default:
            return 0;
    }
}

double Evaluator::evaluate(const NodePtr& node) {
    // top-level node should be an ASSIGN node: name = expression
    if (node->type == NodeType::ASSIGN) {
        double result = evalNode(node->right);
        variables[node->name] = result;
        return result;
    }
    return evalNode(node);
}

double Evaluator::getVariable(const std::string& name) const {
    auto it = variables.find(name);
    if (it == variables.end()) {
        // treat unknown variables as 0, keeps things simple for this project
        return 0;
    }
    return it->second;
}
