#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <cctype>

using namespace std;

// Token Structure
struct Token
{
    string value;
    string type;
};

// Reserved Keywords
set<string> keywords = {
    "int", "float", "bool", "char", "double",
    "long", "short", "return", "if", "else",
    "for", "while", "do", "switch", "case",
    "break", "continue", "class", "public",
    "private", "protected", "void"
};

// Identifier Validation
bool isValidIdentifier(const string &id, string &reason)
{
    if (id.empty())
    {
        reason = "Identifier is empty";
        return false;
    }

    if (!(isalpha(id[0]) || id[0] == '_'))
    {
        reason = "First character must be a letter or underscore";
        return false;
    }

    for (char ch : id)
    {
        if (!(isalnum(ch) || ch == '_'))
        {
            reason = "Identifier contains invalid character";
            return false;
        }
    }

    if (keywords.count(id))
    {
        reason = "Identifier is a reserved keyword";
        return false;
    }

    return true;
}

// Number Check
bool isNumber(const string &str)
{
    if (str.empty())
        return false;

    bool decimalFound = false;

    for (char ch : str)
    {
        if (ch == '.')
        {
            if (decimalFound)
                return false;
            decimalFound = true;
        }
        else if (!isdigit(ch))
        {
            return false;
        }
    }

    return true;
}

// Tokenizer (Experiment 1)
bool tokenize(const string &input, vector<Token> &tokens, string &errorMsg)
{
    int i = 0;
    int n = input.length();

    while (i < n)
    {
        char ch = input[i];

        // skip spaces
        if (isspace(ch))
        {
            i++;
            continue;
        }

        // Identifier
        if (isalpha(ch) || ch == '_')
        {
            string word;

            while (i < n &&
                   (isalnum(input[i]) || input[i] == '_'))
            {
                word += input[i];
                i++;
            }

            tokens.push_back({word, "IDENTIFIER"});
        }

        // Number
        else if (isdigit(ch))
        {
            string num;
            bool dotFound = false;

            while (i < n &&
                   (isdigit(input[i]) || input[i] == '.'))
            {
                if (input[i] == '.')
                {
                    if (dotFound)
                        break;

                    dotFound = true;
                }

                num += input[i];
                i++;
            }

            tokens.push_back({num, "NUMBER"});
        }

        // Assignment Operator
        else if (ch == '=')
        {
            tokens.push_back({"=", "ASSIGNMENT"});
            i++;
        }

        // Arithmetic Operators
        else if (ch == '+' || ch == '-' ||
                 ch == '*' || ch == '/')
        {
            string op(1, ch);
            tokens.push_back({op, "ARITHMETIC"});
            i++;
        }

        // Parentheses
        else if (ch == '(' || ch == ')')
        {
            string p(1, ch);
            tokens.push_back({p, "PARENTHESIS"});
            i++;
        }

        // Semicolon
        else if (ch == ';')
        {
            tokens.push_back({";", "SEMICOLON"});
            i++;
        }

        else
        {
            errorMsg = "Lexical Error: Invalid character '";
            errorMsg += ch;
            errorMsg += "'";
            return false;
        }
    }

    return true;
}

// Syntax Analysis
// Grammar:
// id = operand operator operand ;
bool syntaxCheck(const vector<Token> &tokens,
                 string &errorMsg)
{
    if (tokens.size() != 6)
    {
        errorMsg =
            "Syntax Error: Statement must contain exactly 6 tokens";
        return false;
    }

    string reason;

    if (!isValidIdentifier(tokens[0].value, reason))
    {
        errorMsg =
            "Syntax Error: Left side must be a valid identifier";
        return false;
    }

    if (tokens[1].value != "=")
    {
        errorMsg =
            "Syntax Error: Expected '=' after identifier";
        return false;
    }

    if (!(tokens[2].type == "IDENTIFIER" ||
          tokens[2].type == "NUMBER"))
    {
        errorMsg =
            "Syntax Error: Expected first operand";
        return false;
    }

    if (!(tokens[3].value == "+" ||
          tokens[3].value == "-" ||
          tokens[3].value == "*" ||
          tokens[3].value == "/"))
    {
        errorMsg =
            "Syntax Error: Expected arithmetic operator";
        return false;
    }

    if (!(tokens[4].type == "IDENTIFIER" ||
          tokens[4].type == "NUMBER"))
    {
        errorMsg =
            "Syntax Error: Expected second operand";
        return false;
    }

    if (tokens[5].value != ";")
    {
        errorMsg =
            "Syntax Error: Missing semicolon";
        return false;
    }

    return true;
}

// Get Datatype
string getType(const string &operand,
               const map<string, string> &symbolTable,
               string &errorMsg)
{
    if (isNumber(operand))
    {
        if (operand.find('.') != string::npos)
            return "float";

        return "int";
    }

    auto it = symbolTable.find(operand);

    if (it == symbolTable.end())
    {
        errorMsg =
            "Semantic Error: Undeclared identifier '" +
            operand + "'";
        return "";
    }

    return it->second;
}

// Semantic Analysis
bool semanticCheck(const vector<Token> &tokens,
                   const map<string, string> &symbolTable,
                   string &message)
{
    string errorMsg;

    string leftType =
        getType(tokens[0].value, symbolTable, errorMsg);

    if (!errorMsg.empty())
    {
        message = errorMsg;
        return false;
    }

    string op1Type =
        getType(tokens[2].value, symbolTable, errorMsg);

    if (!errorMsg.empty())
    {
        message = errorMsg;
        return false;
    }

    string op2Type =
        getType(tokens[4].value, symbolTable, errorMsg);

    if (!errorMsg.empty())
    {
        message = errorMsg;
        return false;
    }

    if (op1Type == "bool" || op2Type == "bool")
    {
        message =
            "Semantic Error: bool cannot be used in arithmetic operations";
        return false;
    }

    string exprType;

    if (op1Type == "float" || op2Type == "float")
        exprType = "float";
    else
        exprType = "int";

    if (leftType == "int" &&
        exprType == "float")
    {
        message =
            "Semantic Error: Cannot assign float result to int variable";
        return false;
    }

    message =
        "Semantic Analysis Successful. Expression type = " +
        exprType;

    return true;
}

// Main Program
int main()
{
    map<string, string> symbolTable;

    // Sample Symbol Table
    symbolTable["count"] = "int";
    symbolTable["price"] = "float";
    symbolTable["total"] = "float";
    symbolTable["active"] = "bool";

    string statement;

    cout << "Compiler Front-End Checker\n";
    cout << "----------------------------------\n";

    cout << "\nSymbol Table:\n";
    for (auto &entry : symbolTable)
    {
        cout << entry.first
             << " : "
             << entry.second
             << endl;
    }

    cout << "\nEnter Statement:\n";
    getline(cin, statement);

    vector<Token> tokens;
    string errorMsg;

    // Phase 1: Lexical Analysis
    if (!tokenize(statement, tokens, errorMsg))
    {
        cout << "\n[LEXICAL ANALYSIS FAILED]\n";
        cout << errorMsg << endl;
        return 0;
    }

    cout << "\nTokens:\n";
    for (auto &token : tokens)
    {
        cout << token.value
             << " -> "
             << token.type
             << endl;
    }

    // Phase 2: Identifier Validation
    for (auto &token : tokens)
    {
        if (token.type == "IDENTIFIER")
        {
            string reason;

            if (!isValidIdentifier(token.value,
                                   reason))
            {
                cout << "\n[IDENTIFIER CHECK FAILED]\n";
                cout << token.value
                     << " : "
                     << reason
                     << endl;

                return 0;
            }
        }
    }

    // Phase 3: Syntax Analysis
    if (!syntaxCheck(tokens, errorMsg))
    {
        cout << "\n[SYNTAX ANALYSIS FAILED]\n";
        cout << errorMsg << endl;
        return 0;
    }

    // Phase 4: Semantic Analysis
    string semanticResult;

    if (!semanticCheck(tokens,
                       symbolTable,
                       semanticResult))
    {
        cout << "\n[SEMANTIC ANALYSIS FAILED]\n";
        cout << semanticResult << endl;
        return 0;
    }

    cout << "\n[ACCEPTED]\n";
    cout << semanticResult << endl;

    return 0;
}
