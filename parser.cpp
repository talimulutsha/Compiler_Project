// parser.cpp
// Module 2: Parser (implementation)
// Classic recursive-descent parser. Same grammar as the syntax checker,
// except this time we actually build tree nodes as we go.
//
// Precedence is handled just by the ORDER of the functions:
// parseExpr calls parseTerm calls parseFactor, so * and / naturally
// bind tighter than + and -.

#include "parser.h"

Parser::Parser(const std::vector<Token>& tokenList) : tokens(tokenList) {}

Token Parser::current() {
    return tokens[pos];
}

Token Parser::advance() {
    return tokens[pos++];
}

// statement -> IDENTIFIER '=' expr
NodePtr Parser::parseStatement() {
    Token idToken = advance();          // the variable name, e.g. "a"
    advance();                          // the '=' sign, we already know it's there

    NodePtr exprNode = parseExpr();

    auto assignNode = std::make_shared<Node>(NodeType::ASSIGN);
    assignNode->name = idToken.text;
    assignNode->right = exprNode;
    return assignNode;
}

// expr -> term (('+' | '-') term)*
NodePtr Parser::parseExpr() {
    NodePtr left = parseTerm();

    while (current().type == TokenType::PLUS || current().type == TokenType::MINUS) {
        char op = advance().text[0];
        NodePtr right = parseTerm();

        auto node = std::make_shared<Node>(NodeType::BINOP);
        node->op = op;
        node->left = left;
        node->right = right;
        left = node;
    }
    return left;
}

// term -> factor (('*' | '/') factor)*
NodePtr Parser::parseTerm() {
    NodePtr left = parseFactor();

    while (current().type == TokenType::MUL || current().type == TokenType::DIV) {
        char op = advance().text[0];
        NodePtr right = parseFactor();

        auto node = std::make_shared<Node>(NodeType::BINOP);
        node->op = op;
        node->left = left;
        node->right = right;
        left = node;
    }
    return left;
}

// factor -> NUMBER | IDENTIFIER | '(' expr ')'
NodePtr Parser::parseFactor() {
    Token tok = current();

    if (tok.type == TokenType::NUMBER) {
        advance();
        auto node = std::make_shared<Node>(NodeType::NUMBER);
        node->numValue = tok.value;
        return node;
    }

    if (tok.type == TokenType::IDENTIFIER) {
        advance();
        auto node = std::make_shared<Node>(NodeType::IDENTIFIER);
        node->name = tok.text;
        return node;
    }

    if (tok.type == TokenType::LPAREN) {
        advance();               // consume '('
        NodePtr inner = parseExpr();
        advance();               // consume ')'
        return inner;
    }

    // We shouldn't really get here if SyntaxChecker already approved the input.
    return std::make_shared<Node>(NodeType::NUMBER);
}
