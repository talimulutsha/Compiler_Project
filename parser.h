// parser.h
// Module 2: Parser
// Job: take the tokens (already confirmed valid by the Syntax Checker)
// and build an Abstract Syntax Tree (AST) that respects operator precedence.

#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include "token.h"
#include "ast.h"

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokenList);

    // Builds and returns the root of the AST for the whole statement.
    NodePtr parseStatement();

private:
    const std::vector<Token>& tokens;
    size_t pos = 0;

    Token current();
    Token advance();

    NodePtr parseExpr();    // handles + and -
    NodePtr parseTerm();    // handles * and /
    NodePtr parseFactor();  // numbers, identifiers, ( expr )
};

#endif
