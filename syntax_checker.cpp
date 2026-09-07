// syntax_checker.cpp
// Module 3: Syntax Checker (implementation)
// This walks the tokens the same way a parser would, but it only cares
// about "does this match the grammar" — it doesn't build a tree.

#include "syntax_checker.h"

SyntaxChecker::SyntaxChecker(const std::vector<Token>& tokenList) : tokens(tokenList) {}

Token SyntaxChecker::current() {
    return tokens[pos];
}

// If the current token matches what we expect, consume it and return true.
bool SyntaxChecker::match(TokenType type) {
    if (current().type == type) {
        pos++;
        return true;
    }
    return false;
}

bool SyntaxChecker::isValid() {
    pos = 0;
    if (!checkStatement()) return false;

    // after a full statement we should be at END, nothing extra hanging around
    if (current().type != TokenType::END) {
        errMsg = "Unexpected extra input after expression: '" + current().text + "'";
        return false;
    }
    return true;
}

// statement -> IDENTIFIER '=' expr
bool SyntaxChecker::checkStatement() {
    if (current().type != TokenType::IDENTIFIER) {
        errMsg = "Expected a variable name at the start (e.g. 'a = ...')";
        return false;
    }
    match(TokenType::IDENTIFIER);

    if (!match(TokenType::ASSIGN)) {
        errMsg = "Expected '=' after variable name";
        return false;
    }

    return checkExpr();
}

// expr -> term (('+' | '-') term)*
bool SyntaxChecker::checkExpr() {
    if (!checkTerm()) return false;

    while (current().type == TokenType::PLUS || current().type == TokenType::MINUS) {
        pos++; // consume the operator
        if (!checkTerm()) return false;
    }
    return true;
}

// term -> factor (('*' | '/') factor)*
bool SyntaxChecker::checkTerm() {
    if (!checkFactor()) return false;

    while (current().type == TokenType::MUL || current().type == TokenType::DIV) {
        pos++; // consume the operator
        if (!checkFactor()) return false;
    }
    return true;
}

// factor -> NUMBER | IDENTIFIER | '(' expr ')'
bool SyntaxChecker::checkFactor() {
    if (current().type == TokenType::NUMBER) {
        pos++;
        return true;
    }
    if (current().type == TokenType::IDENTIFIER) {
        pos++;
        return true;
    }
    if (current().type == TokenType::LPAREN) {
        pos++;
        if (!checkExpr()) return false;
        if (!match(TokenType::RPAREN)) {
            errMsg = "Missing closing ')'";
            return false;
        }
        return true;
    }
    if (current().type == TokenType::UNKNOWN) {
        errMsg = "Unrecognized character: '" + current().text + "'";
        return false;
    }

    errMsg = "Expected a number, variable, or '(' but got '" + current().text + "'";
    return false;
}
