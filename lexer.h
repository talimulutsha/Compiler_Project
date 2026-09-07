// lexer.h
// Module 1: Lexical Analyzer
// Job: turn the raw input string into a list of tokens.

#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include "token.h"

class Lexer {
public:
    explicit Lexer(const std::string& sourceText);

    // Scans the whole input and returns all tokens (ends with an END token)
    std::vector<Token> tokenize();

private:
    std::string src;
    size_t pos = 0;

    char peek();
    char advance();
    void skipWhitespace();
    Token makeNumber();
    Token makeIdentifier();
};

#endif
