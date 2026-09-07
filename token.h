// token.h
// Just holds the different kinds of tokens our lexer can spit out.
// Every other file includes this so everyone agrees on what a "token" is.

#ifndef TOKEN_H
#define TOKEN_H

#include <string>

enum class TokenType {
    NUMBER,      // 5, 3, 2, 10 ...
    IDENTIFIER,  // a, b, result ...
    PLUS,        // +
    MINUS,       // -
    MUL,         // *
    DIV,         // /
    ASSIGN,      // =
    LPAREN,      // (
    RPAREN,      // )
    END,         // end of input
    UNKNOWN      // anything we don't recognize (bad character)
};

struct Token {
    TokenType type;
    std::string text;   // the raw text, e.g. "5" or "a"
    double value = 0;    // only used when type == NUMBER

    Token(TokenType t, std::string txt, double v = 0)
        : type(t), text(std::move(txt)), value(v) {}
};

#endif
