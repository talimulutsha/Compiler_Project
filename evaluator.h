// evaluator.h
// Module 4: Expression Evaluator
// Job: walk the AST and actually compute the number it represents,
// keeping track of variable values along the way.

#ifndef EVALUATOR_H
#define EVALUATOR_H

#include <map>
#include <string>
#include "ast.h"

class Evaluator {
public:
    // Evaluates a full "assign" statement (e.g. a = 5 + 3 * 2)
    // and stores the result under the variable name.
    double evaluate(const NodePtr& node);

    // Look up a variable's current value (used for printing / testing).
    double getVariable(const std::string& name) const;

private:
    std::map<std::string, double> variables;

    double evalNode(const NodePtr& node);
};

#endif
