# ⚡ Simple Calculator Compiler

A complete, 4-stage arithmetic compiler front-end and expression evaluator with an interactive, modern web UI.

---

## 🎯 Compiler Architecture & Pipeline

```
Source Code (e.g. a = 5 + 3 * 2)
     │
     ▼
[1. Lexical Analyzer] ──► Tokens (NUMBER, ID, ASSIGN, PLUS, MUL, ...)
     │
     ▼
[2. Parser & 3. Syntax Checker] ──► Abstract Syntax Tree (AST) & Grammar Verification
     │
     ▼
[4. Expression Evaluator] ──► Step Trace, Live Symbol Table & Final Result
```

### Supported Grammar
- **Statements**: Variable assignments (`ID = expr`) or bare arithmetic expressions (`expr`)
- **Operators**: Addition (`+`), Subtraction (`-`), Multiplication (`*`), Division (`/`) with standard precedence (`*`, `/` higher than `+`, `-`)
- **Unary Signs**: `+`, `-` (e.g., `-5`, `-(a + 2)`)
- **Parentheses**: Arbitrary nesting `( ... )`
- **Identifiers / Variables**: Stored in a persistent symbol table

---

## 🚀 How to Run

### Method 1: Web Studio (Recommended)
Run the following command in terminal:
```bash
py main.py
```
This automatically starts the local backend server at `http://localhost:5000` and opens the interactive dashboard in your browser.

Or run directly via calculator:
```bash
py calculator.py --serve
```

### Method 2: Standalone Browser Mode (Zero Setup)
Simply open `frontend.html` in any modern web browser. It contains an integrated client-side compiler engine with identical grammar and parsing logic.

### Method 3: Command Line (CLI)
Evaluate expressions directly from the terminal:
```bash
py calculator.py "a = 5 + 3 * 2"
```

---

## 📁 Project Structure

- **[calculator.py](file:///c:/Users/talim/Documents/Compiler/calculator.py)**: The 4-stage compiler backend (Lexer, Parser, Syntax Checker, Evaluator) and HTTP REST server.
- **[frontend.html](file:///c:/Users/talim/Documents/Compiler/frontend.html)**: Interactive visual dashboard with AST tree graph, token stream chips, reduction trace, and symbol table.
- **[main.py](file:///c:/Users/talim/Documents/Compiler/main.py)**: Entry runner that boots the server and browser.
