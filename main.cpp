// main.cpp
// Project 2: Simple Calculator Compiler
// Ties together the 4 modules in order:
//   1. Lexical Analyzer  -> tokens
//   2. Syntax Checker    -> is it grammatically valid?
//   3. Parser            -> build the AST
//   4. Expression Evaluator -> compute the result

#include <iostream>
#include <string>
#include "lexer.h"
#include "syntax_checker.h"
#include "parser.h"
#include "evaluator.h"

void runLine(const std::string& line, Evaluator& evaluator) {
    if (line.empty()) return;

    std::cout << "Input:\n" << line << "\n\n";

    // Step 1: Lexical Analysis
    Lexer lexer(line);
    std::vector<Token> tokens = lexer.tokenize();

    // Step 2: Syntax Checking
    SyntaxChecker checker(tokens);
    if (!checker.isValid()) {
        std::cout << "Invalid Expression\n";
        std::cout << "Reason: " << checker.errorMessage() << "\n";
        std::cout << "----------------------------------------\n";
        return;
    }
    std::cout << "Valid Expression\n\n";

    // Step 3: Parsing (build the AST)
    Parser parser(tokens);
    NodePtr tree = parser.parseStatement();
    std::cout << "Parse Successful\n\n";

    // Step 4: Evaluation
    try {
        double result = evaluator.evaluate(tree);
        // print as an integer when it's a whole number, e.g. "11" not "11.0"
        if (result == (long long)result) {
            std::cout << "Result = " << (long long)result << "\n";
        } else {
            std::cout << "Result = " << result << "\n";
        }
    } catch (const std::exception& e) {
        std::cout << "Evaluation Error: " << e.what() << "\n";
    }

    std::cout << "----------------------------------------\n";
}

int main() {
    Evaluator evaluator; // shared across lines so variables carry over

    std::cout << "=== Simple Calculator Compiler ===\n";
    std::cout << "Type an expression like: a = 5 + 3 * 2\n";
    std::cout << "Type 'exit' to quit.\n\n";

    std::string line;
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;
        if (line == "exit") break;
        runLine(line, evaluator);
    }

    return 0;
}
