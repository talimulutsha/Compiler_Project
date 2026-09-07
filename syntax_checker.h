// syntax_checker.h
// Module 3: Syntax Checker
// Job: look at the token list and decide if it FOLLOWS THE GRAMMAR RULES
// of our language, without actually calculating anything.
//
// Grammar we're checking:
//   statement  -> IDENTIFIER '=' expr
//   expr       -> term (('+' | '-') term)*
//   term       -> factor (('*' | '/') factor)*
//   factor     -> NUMBER | IDENTIFIER | '(' expr ')'

#ifndef SYNTAX_CHECKER_H
#define SYNTAX_CHECKER_H

#include <vector>
#include <string>
#include "token.h"

class SyntaxChecker {
public:
    explicit SyntaxChecker(const std::vector<Token>& tokenList);

    // Returns true if the tokens form a valid statement.
    // If false, errorMessage() will explain why.
    bool isValid();

    std::string errorMessage() const { return errMsg; }

private:
    const std::vector<Token>& tokens;
    size_t pos = 0;
    std::string errMsg;

    Token current();
    bool match(TokenType type);

    // one function per grammar rule, same shape as the Parser
    bool checkStatement();
    bool checkExpr();
    bool checkTerm();
    bool checkFactor();
};

#endif
