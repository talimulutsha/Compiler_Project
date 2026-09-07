// lexer.cpp
// Module 1: Lexical Analyzer (implementation)
// This just walks through the input character by character and groups
// characters into meaningful "tokens" (numbers, identifiers, symbols).

#include "lexer.h"
#include <cctype>

Lexer::Lexer(const std::string& sourceText) : src(sourceText) {}

char Lexer::peek() {
    if (pos >= src.size()) return '\0';
    return src[pos];
}

char Lexer::advance() {
    return src[pos++];
}

void Lexer::skipWhitespace() {
    while (pos < src.size() && std::isspace((unsigned char)peek())) {
        pos++;
    }
}

// Reads a full number like "123" or "3.14"
Token Lexer::makeNumber() {
    std::string num;
    while (pos < src.size() && (std::isdigit((unsigned char)peek()) || peek() == '.')) {
        num += advance();
    }
    return Token(TokenType::NUMBER, num, std::stod(num));
}

// Reads a full identifier like "a" or "result"
Token Lexer::makeIdentifier() {
    std::string id;
    while (pos < src.size() && std::isalnum((unsigned char)peek())) {
        id += advance();
    }
    return Token(TokenType::IDENTIFIER, id);
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (pos < src.size()) {
        skipWhitespace();
        if (pos >= src.size()) break;

        char c = peek();

        if (std::isdigit((unsigned char)c)) {
            tokens.push_back(makeNumber());
        }
        else if (std::isalpha((unsigned char)c)) {
            tokens.push_back(makeIdentifier());
        }
        else {
            // single-character symbols
            switch (c) {
                case '+': tokens.push_back(Token(TokenType::PLUS, "+")); advance(); break;
                case '-': tokens.push_back(Token(TokenType::MINUS, "-")); advance(); break;
                case '*': tokens.push_back(Token(TokenType::MUL, "*")); advance(); break;
                case '/': tokens.push_back(Token(TokenType::DIV, "/")); advance(); break;
                case '=': tokens.push_back(Token(TokenType::ASSIGN, "=")); advance(); break;
                case '(': tokens.push_back(Token(TokenType::LPAREN, "(")); advance(); break;
                case ')': tokens.push_back(Token(TokenType::RPAREN, ")")); advance(); break;
                default:
                    // unknown character, e.g. '@' or '$'
                    tokens.push_back(Token(TokenType::UNKNOWN, std::string(1, c)));
                    advance();
                    break;
            }
        }
    }

    tokens.push_back(Token(TokenType::END, ""));
    return tokens;
}
