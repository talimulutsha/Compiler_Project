"""
Simple Calculator Compiler (Backend & HTTP Server)
--------------------------------------------------
A full 4-stage compiler front-end and evaluator for arithmetic expressions & variable assignments:
    e.g. a = 5 + 3 * 2
         b = (a + 2) * 3

Compiler Pipeline:
    Source Code -> [1. Lexical Analyzer]   -> Tokens
                -> [2. Parser]             -> Abstract Syntax Tree (AST)
                -> [3. Syntax Checker]     -> Valid / Invalid Grammar
                -> [4. Evaluator]          -> Step Trace, Symbol Table & Result

Includes a built-in zero-dependency HTTP server to power frontend.html.
"""

import sys
import os
import re
import json
import webbrowser
from http.server import HTTPServer, BaseHTTPRequestHandler
from urllib.parse import urlparse

# Global Symbol Table to persist variable states
symbol_table = {}


# ============================================================
# MODULE 1: LEXICAL ANALYZER
# ============================================================
TOKEN_SPEC = [
    ("NUMBER",   r"\d+(\.\d+)?"),    # integers and floats: 42, 3.1415
    ("ID",       r"[A-Za-z_]\w*"),   # identifiers / variables: a, total, x1
    ("ASSIGN",   r"="),
    ("PLUS",     r"\+"),
    ("MINUS",    r"-"),
    ("MUL",      r"\*"),
    ("DIV",      r"/"),
    ("LPAREN",   r"\("),
    ("RPAREN",   r"\)"),
    ("SKIP",     r"[ \t\r]+"),       # whitespace ignored
    ("NEWLINE",  r"\n"),             # newlines for multi-statement scripts
    ("MISMATCH", r"."),              # illegal character
]

MASTER_PATTERN = "|".join(f"(?P<{name}>{pattern})" for name, pattern in TOKEN_SPEC)


class Token:
    """Represents a single lexical token with position information."""
    def __init__(self, type_, value, line=1, col=1, start=0, end=0):
        self.type = type_
        self.value = value
        self.line = line
        self.col = col
        self.start = start
        self.end = end

    def to_dict(self):
        return {
            "type": self.type,
            "value": str(self.value) if self.value is not None else "EOF",
            "line": self.line,
            "col": self.col,
            "start": self.start,
            "end": self.end,
        }

    def __repr__(self):
        return f"Token({self.type}, {self.value}, line={self.line}, col={self.col})"


class LexicalError(Exception):
    def __init__(self, message, line=1, col=1):
        super().__init__(f"Line {line}, Col {col}: {message}")
        self.line = line
        self.col = col
        self.raw_message = message


def tokenize(source_code):
    """
    Scans source_code and returns a list of Token objects ending in EOF.
    Raises LexicalError on unknown characters.
    """
    tokens = []
    line_num = 1
    line_start = 0

    for match in re.finditer(MASTER_PATTERN, source_code):
        kind = match.lastgroup
        text = match.group()
        col = match.start() - line_start + 1

        if kind == "SKIP":
            continue

        if kind == "NEWLINE":
            line_num += 1
            line_start = match.end()
            tokens.append(Token("NEWLINE", "\n", line_num, col, match.start(), match.end()))
            continue

        if kind == "MISMATCH":
            raise LexicalError(f"Illegal character '{text}'", line_num, col)

        if kind == "NUMBER":
            val = float(text) if "." in text else int(text)
            tokens.append(Token("NUMBER", val, line_num, col, match.start(), match.end()))
        else:
            tokens.append(Token(kind, text, line_num, col, match.start(), match.end()))

    tokens.append(Token("EOF", None, line_num, len(source_code) - line_start + 1, len(source_code), len(source_code)))
    return tokens


# ============================================================
# AST NODE DEFINITIONS
# ============================================================
class ASTNode:
    _next_id = 1

    def __init__(self):
        self.node_id = ASTNode._next_id
        ASTNode._next_id += 1

    def to_dict(self):
        raise NotImplementedError


class Num(ASTNode):
    def __init__(self, value):
        super().__init__()
        self.value = value

    def to_dict(self):
        return {
            "id": self.node_id,
            "type": "Num",
            "label": f"{self.value}",
            "value": self.value,
            "children": []
        }


class Var(ASTNode):
    def __init__(self, name):
        super().__init__()
        self.name = name

    def to_dict(self):
        return {
            "id": self.node_id,
            "type": "Var",
            "label": f"Var({self.name})",
            "name": self.name,
            "children": []
        }


class BinOp(ASTNode):
    def __init__(self, left, op, right):
        super().__init__()
        self.left = left
        self.op = op  # 'PLUS', 'MINUS', 'MUL', 'DIV'
        self.right = right

    def to_dict(self):
        op_symbol = {"PLUS": "+", "MINUS": "-", "MUL": "*", "DIV": "/"}.get(self.op, self.op)
        return {
            "id": self.node_id,
            "type": "BinOp",
            "label": f"BinOp ({op_symbol})",
            "op": self.op,
            "op_symbol": op_symbol,
            "children": [self.left.to_dict(), self.right.to_dict()]
        }


class UnaryOp(ASTNode):
    def __init__(self, op, operand):
        super().__init__()
        self.op = op  # 'PLUS', 'MINUS'
        self.operand = operand

    def to_dict(self):
        op_symbol = "+" if self.op == "PLUS" else "-"
        return {
            "id": self.node_id,
            "type": "UnaryOp",
            "label": f"Unary ({op_symbol})",
            "op": self.op,
            "op_symbol": op_symbol,
            "children": [self.operand.to_dict()]
        }


class Assign(ASTNode):
    def __init__(self, name, expr):
        super().__init__()
        self.name = name
        self.expr = expr

    def to_dict(self):
        return {
            "id": self.node_id,
            "type": "Assign",
            "label": f"Assign (=)",
            "name": self.name,
            "children": [
                {"id": self.node_id * 100 + 1, "type": "Var", "label": f"ID({self.name})", "name": self.name, "children": []},
                self.expr.to_dict()
            ]
        }


class Program(ASTNode):
    def __init__(self, statements):
        super().__init__()
        self.statements = statements

    def to_dict(self):
        return {
            "id": self.node_id,
            "type": "Program",
            "label": "Program",
            "children": [stmt.to_dict() for stmt in self.statements]
        }


# ============================================================
# MODULE 2: PARSER & MODULE 3: SYNTAX CHECKER
# ============================================================
class SyntaxCheckError(Exception):
    def __init__(self, message, token=None):
        line = token.line if token else 1
        col = token.col if token else 1
        tok_desc = f"'{token.value}' ({token.type})" if token and token.value is not None else (token.type if token else "unknown")
        super().__init__(f"Line {line}, Col {col}: {message} (near {tok_desc})")
        self.line = line
        self.col = col
        self.token = token
        self.raw_message = message


class Parser:
    def __init__(self, tokens):
        self.tokens = tokens
        self.pos = 0
        self.current = self.tokens[self.pos]

    def error(self, message):
        raise SyntaxCheckError(message, self.current)

    def eat(self, token_type):
        if self.current.type == token_type:
            self.pos += 1
            if self.pos < len(self.tokens):
                self.current = self.tokens[self.pos]
        else:
            self.error(f"Expected token type '{token_type}'")

    def factor(self):
        tok = self.current

        if tok.type == "NUMBER":
            self.eat("NUMBER")
            return Num(tok.value)

        if tok.type == "ID":
            self.eat("ID")
            return Var(tok.value)

        if tok.type in ("PLUS", "MINUS"):
            op = tok.type
            self.eat(op)
            return UnaryOp(op, self.factor())

        if tok.type == "LPAREN":
            self.eat("LPAREN")
            node = self.expr()
            self.eat("RPAREN")
            return node

        self.error("Expected number, variable identifier, or '('")

    def term(self):
        node = self.factor()
        while self.current.type in ("MUL", "DIV"):
            op = self.current.type
            self.eat(op)
            node = BinOp(node, op, self.factor())
        return node

    def expr(self):
        node = self.term()
        while self.current.type in ("PLUS", "MINUS"):
            op = self.current.type
            self.eat(op)
            node = BinOp(node, op, self.term())
        return node

    def statement(self):
        # Look ahead: "ID =" means assignment
        if self.current.type == "ID" and self.pos + 1 < len(self.tokens) and self.tokens[self.pos + 1].type == "ASSIGN":
            name = self.current.value
            self.eat("ID")
            self.eat("ASSIGN")
            return Assign(name, self.expr())
        return self.expr()

    def parse_program(self):
        statements = []
        # Skip leading newlines
        while self.current.type == "NEWLINE":
            self.eat("NEWLINE")

        if self.current.type == "EOF":
            self.error("Empty expression or statement")

        while self.current.type != "EOF":
            stmt = self.statement()
            statements.append(stmt)

            if self.current.type == "NEWLINE":
                while self.current.type == "NEWLINE":
                    self.eat("NEWLINE")
            elif self.current.type != "EOF":
                self.error("Unexpected token after statement. Expected newline or end of input")

        if len(statements) == 1:
            return statements[0]
        return Program(statements)


def check_syntax(tokens):
    """Entry point for syntax checking and AST generation."""
    try:
        parser = Parser(tokens)
        ast = parser.parse_program()
        return True, ast, None
    except SyntaxCheckError as e:
        return False, None, e


# ============================================================
# MODULE 4: EXPRESSION EVALUATOR WITH STEP-BY-STEP TRACE
# ============================================================
class EvalError(Exception):
    pass


class Evaluator:
    def __init__(self, env=None):
        self.env = env if env is not None else symbol_table
        self.steps = []

    def format_num(self, val):
        if isinstance(val, float) and val.is_integer():
            return str(int(val))
        if isinstance(val, float):
            return f"{round(val, 6)}"
        return str(val)

    def evaluate(self, node):
        if isinstance(node, Program):
            res = None
            for idx, stmt in enumerate(node.statements, 1):
                self.steps.append(f"--- Executing Statement {idx} ---")
                res = self.evaluate(stmt)
            return res

        if isinstance(node, Num):
            return node.value

        if isinstance(node, Var):
            if node.name not in self.env:
                raise EvalError(f"Undefined variable '{node.name}'. Assign a value to it first.")
            val = self.env[node.name]
            self.steps.append(f"Looked up variable '{node.name}' → {self.format_num(val)}")
            return val

        if isinstance(node, UnaryOp):
            val = self.evaluate(node.operand)
            res = val if node.op == "PLUS" else -val
            op_sym = "+" if node.op == "PLUS" else "-"
            self.steps.append(f"Unary {op_sym}({self.format_num(val)}) → {self.format_num(res)}")
            return res

        if isinstance(node, BinOp):
            left_val = self.evaluate(node.left)
            right_val = self.evaluate(node.right)
            op_map = {"PLUS": "+", "MINUS": "-", "MUL": "*", "DIV": "/"}
            op_sym = op_map.get(node.op, node.op)

            if node.op == "PLUS":
                res = left_val + right_val
            elif node.op == "MINUS":
                res = left_val - right_val
            elif node.op == "MUL":
                res = left_val * right_val
            elif node.op == "DIV":
                if right_val == 0:
                    raise EvalError("Division by zero error")
                res = left_val / right_val
            else:
                raise EvalError(f"Unsupported operator {node.op}")

            self.steps.append(
                f"Evaluated {self.format_num(left_val)} {op_sym} {self.format_num(right_val)} → {self.format_num(res)}"
            )
            return res

        if isinstance(node, Assign):
            val = self.evaluate(node.expr)
            self.env[node.name] = val
            self.steps.append(f"Assigned variable '{node.name}' = {self.format_num(val)}")
            return val

        raise EvalError(f"Unknown AST node: {type(node).__name__}")


def evaluate(node):
    """Simple evaluator helper."""
    evaluator = Evaluator(symbol_table)
    return evaluator.evaluate(node)


# ============================================================
# COMPILER PIPELINE RUNNER & JSON API GENERATOR
# ============================================================
def compile_and_run(source_code, env=None):
    """
    Executes the entire 4-stage compiler pipeline and returns
    a comprehensive dictionary with token details, AST tree,
    syntax validity, evaluation steps, final result, and symbol table.
    """
    if env is None:
        env = symbol_table

    source_code = source_code.strip()
    if not source_code:
        return {
            "success": False,
            "error_stage": "input",
            "error_message": "Expression cannot be empty.",
            "tokens": [],
            "ast": None,
            "is_valid_syntax": False,
            "steps": [],
            "result": None,
            "symbols": {k: float(v) if isinstance(v, float) else int(v) for k, v in env.items()}
        }

    # Reset AST node ID counter for clean visualizations
    ASTNode._next_id = 1

    # Stage 1: Lexical Analysis
    try:
        tokens = tokenize(source_code)
        token_dicts = [t.to_dict() for t in tokens]
    except LexicalError as e:
        return {
            "success": False,
            "error_stage": "lexical",
            "error_message": str(e),
            "line": e.line,
            "col": e.col,
            "tokens": [],
            "ast": None,
            "is_valid_syntax": False,
            "steps": [],
            "result": None,
            "symbols": {k: float(v) if isinstance(v, float) else int(v) for k, v in env.items()}
        }

    # Stage 2 & 3: Parser & Syntax Checker
    is_valid, ast, syntax_err = check_syntax(tokens)
    if not is_valid:
        return {
            "success": False,
            "error_stage": "syntax",
            "error_message": str(syntax_err),
            "line": syntax_err.line,
            "col": syntax_err.col,
            "tokens": token_dicts,
            "ast": None,
            "is_valid_syntax": False,
            "steps": [],
            "result": None,
            "symbols": {k: float(v) if isinstance(v, float) else int(v) for k, v in env.items()}
        }

    ast_dict = ast.to_dict()

    # Stage 4: Expression Evaluation
    evaluator = Evaluator(env)
    try:
        val = evaluator.evaluate(ast)
        if isinstance(val, float) and val.is_integer():
            formatted_val = int(val)
        elif isinstance(val, float):
            formatted_val = round(val, 6)
        else:
            formatted_val = val

        return {
            "success": True,
            "error_stage": None,
            "error_message": None,
            "tokens": token_dicts,
            "ast": ast_dict,
            "is_valid_syntax": True,
            "steps": evaluator.steps,
            "result": formatted_val,
            "symbols": {k: float(v) if isinstance(v, float) else int(v) for k, v in env.items()}
        }
    except EvalError as e:
        return {
            "success": False,
            "error_stage": "evaluation",
            "error_message": str(e),
            "tokens": token_dicts,
            "ast": ast_dict,
            "is_valid_syntax": True,
            "steps": evaluator.steps,
            "result": None,
            "symbols": {k: float(v) if isinstance(v, float) else int(v) for k, v in env.items()}
        }


# ============================================================
# HTTP SERVER FOR FRONTEND INTEGRATION
# ============================================================
class CompilerHTTPHandler(BaseHTTPRequestHandler):
    def _send_json(self, data, status=200):
        body = json.dumps(data).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type")
        self.end_headers()
        self.wfile.write(body)

    def do_OPTIONS(self):
        self.send_response(200)
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type")
        self.end_headers()

    def do_GET(self):
        parsed = urlparse(self.path)
        path = parsed.path

        if path in ("/", "/index.html", "/frontend.html"):
            html_path = os.path.join(os.path.dirname(__file__), "frontend.html")
            if os.path.exists(html_path):
                with open(html_path, "rb") as f:
                    content = f.read()
                self.send_response(200)
                self.send_header("Content-Type", "text/html; charset=utf-8")
                self.send_header("Content-Length", str(len(content)))
                self.end_headers()
                self.wfile.write(content)
            else:
                self.send_error(404, "frontend.html not found.")
            return

        if path == "/api/symbols":
            self._send_json({"symbols": symbol_table})
            return

        if path == "/api/ping":
            self._send_json({"status": "ok", "engine": "Python Compiler Backend"})
            return

        self.send_error(404, "Endpoint not found")

    def do_POST(self):
        parsed = urlparse(self.path)
        path = parsed.path
        length = int(self.headers.get("Content-Length", 0))
        raw_data = self.rfile.read(length)

        try:
            body = json.loads(raw_data.decode("utf-8")) if raw_data else {}
        except Exception:
            body = {}

        if path == "/api/compile":
            code = body.get("code", "")
            response = compile_and_run(code)
            self._send_json(response)
            return

        if path == "/api/reset":
            symbol_table.clear()
            self._send_json({"success": True, "message": "Symbol table cleared", "symbols": {}})
            return

        if path == "/api/set_symbol":
            name = body.get("name")
            val = body.get("value")
            if name:
                try:
                    num_val = float(val) if "." in str(val) else int(val)
                    symbol_table[name] = num_val
                    self._send_json({"success": True, "symbols": symbol_table})
                    return
                except ValueError:
                    pass
            self._send_json({"success": False, "error": "Invalid variable name or value"}, status=400)
            return

        self.send_error(404, "Endpoint not found")

    def log_message(self, format, *args):
        # Keep console output neat
        sys.stderr.write(f"[Server] {format % args}\n")


def start_server(port=5000, open_browser=False):
    server_address = ("", port)
    httpd = HTTPServer(server_address, CompilerHTTPHandler)
    url = f"http://localhost:{port}"
    print(f"=" * 60)
    print(f"🚀 Calculator Compiler Server running at {url}")
    print(f"   Open {url} in your browser to access the Web UI")
    print(f"=" * 60)

    if open_browser:
        webbrowser.open(url)

    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\nStopping compiler server...")
        httpd.server_close()


# ============================================================
# CLI DRIVER
# ============================================================
def run(source_code):
    """Run function as per assignment specifications."""
    print(f"Input: {source_code}")
    res = compile_and_run(source_code)
    if res["success"]:
        print("Valid Expression")
        print("Parse Successful")
        print(f"Result = {res['result']}")
    else:
        stage = res["error_stage"]
        if stage == "lexical":
            print(f"Invalid Expression\nLexical Error: {res['error_message']}")
        elif stage == "syntax":
            print(f"Invalid Expression\nSyntax Error: {res['error_message']}")
        elif stage == "evaluation":
            print(f"Valid Expression\nParse Successful\nEvaluation Error: {res['error_message']}")
        else:
            print(f"Error: {res['error_message']}")


def cli_main():
    if len(sys.argv) > 1:
        arg = sys.argv[1]
        if arg in ("--serve", "-s", "serve"):
            port = int(sys.argv[2]) if len(sys.argv) > 2 else 5000
            start_server(port, open_browser=True)
            return

        # Execute expression passed as argument
        expr = " ".join(sys.argv[1:])
        run(expr)
        return

    # Default test cases
    examples = [
        "a = 5 + 3 * 2",
        "b = (a + 2) * 3",
        "c = 10 / 0",
        "d = 5 + * 2"
    ]
    for ex in examples:
        run(ex)
        print()


if __name__ == "__main__":
    cli_main()