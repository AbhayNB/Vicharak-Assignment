#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <cctype>
#include <string>
#include <stdexcept>

using namespace std;

// Token Types
enum class TokenType {
    TOKEN_INT,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_ASSIGN,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_IF,
    TOKEN_EQUAL,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_NOT_EQUAL,
    TOKEN_SEMICOLON,
    TOKEN_UNKNOWN,
    TOKEN_EOF
};

// Token Class
class Token {
public:
    TokenType type;
    string text;
    Token(TokenType type = TokenType::TOKEN_UNKNOWN, const string& text = "")
        : type(type), text(text) {}
};

// Lexer Class
class Lexer {
private:
    string input;
    size_t position;
    
    char currentChar() const {
        if (position >= input.length()) return '\0';
        return input[position];
    }
    
    void advance() {
        position++;
    }
    
    void skipWhitespace() {
        while (isspace(currentChar())) {
            advance();
        }
    }

public:
    explicit Lexer(const string& input) : input(input), position(0) {}

    Token getNextToken() {
        skipWhitespace();
        
        if (position >= input.length()) {
            return Token(TokenType::TOKEN_EOF, "");
        }

        if (isalpha(currentChar())) {
            string text;
            while (isalnum(currentChar())) {
                text += currentChar();
                advance();
            }
            
            if (text == "int") return Token(TokenType::TOKEN_INT, text);
            if (text == "if") return Token(TokenType::TOKEN_IF, text);
            return Token(TokenType::TOKEN_IDENTIFIER, text);
        }

        if (isdigit(currentChar())) {
            string number;
            while (isdigit(currentChar())) {
                number += currentChar();
                advance();
            }
            return Token(TokenType::TOKEN_NUMBER, number);
        }

        char c = currentChar();
        advance();
        
        switch (c) {
            case '=':
                if (currentChar() == '=') {
                    advance();
                    return Token(TokenType::TOKEN_EQUAL, "==");
                }
                return Token(TokenType::TOKEN_ASSIGN, "=");
            case '+': return Token(TokenType::TOKEN_PLUS, "+");
            case '-': return Token(TokenType::TOKEN_MINUS, "-");
            case '(': return Token(TokenType::TOKEN_LPAREN, "(");
            case ')': return Token(TokenType::TOKEN_RPAREN, ")");
            case '{': return Token(TokenType::TOKEN_LBRACE, "{");
            case '}': return Token(TokenType::TOKEN_RBRACE, "}");
            case ';': return Token(TokenType::TOKEN_SEMICOLON, ";");
        }

        return Token(TokenType::TOKEN_UNKNOWN, string(1, c));
    }
};

// AST Classes
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual string generateAssembly(class CodeGenerator& generator) = 0;
};

class Expression : public ASTNode {
public:
    virtual ~Expression() = default;
};

class Statement : public ASTNode {
public:
    virtual ~Statement() = default;
};

// CodeGenerator Class
class CodeGenerator {
private:
    int registerCount = 0;
    int labelCount = 0;
    stringstream assemblyCode;
    map<string, string> variables;

public:
    string getNewRegister() {
        return "R" + to_string(registerCount++);
    }

    string getNewLabel() {
        return "L" + to_string(labelCount++);
    }

    void emit(const string& code) {
        assemblyCode << code << "\n";
    }

    void declareVariable(const string& name) {
        if (variables.find(name) == variables.end()) {
            variables[name] = name;
            emit(name + ": .word 0");
        }
    }

    string getVariableLocation(const string& name) {
        if (variables.find(name) == variables.end()) {
            declareVariable(name);
        }
        return variables[name];
    }

    void generatePrelude() {
        emit(".section .data");
    }

    void generatePostlude() {
        emit(".section .text");
        emit(".global _start");
        emit("_start:");
    }

    void generateEpilogue() {
        emit("MOV R7, #1");  // Exit syscall
        emit("MOV R0, #0");  // Return 0
        emit("SWI 0");       // Software interrupt
    }

    string getCurrentCode() const {
        return assemblyCode.str();
    }
};

// AST Node implementations
class Number : public Expression {
public:
    int value;
    explicit Number(int value) : value(value) {}
    
    string generateAssembly(CodeGenerator& generator) override {
        string reg = generator.getNewRegister();
        generator.emit("MOV " + reg + ", #" + to_string(value));
        return reg;
    }
};

class Identifier : public Expression {
public:
    string name;
    explicit Identifier(string name) : name(name) {}
    
    string generateAssembly(CodeGenerator& generator) override {
        string reg = generator.getNewRegister();
        string location = generator.getVariableLocation(name);
        generator.emit("LDR " + reg + ", [" + location + "]");
        return reg;
    }
};

class BinaryOp : public Expression {
public:
    string op;
    Expression* left;
    Expression* right;
    
    BinaryOp(string op, Expression* left, Expression* right)
        : op(op), left(left), right(right) {}
    
    string generateAssembly(CodeGenerator& generator) override {
        string leftReg = left->generateAssembly(generator);
        string rightReg = right->generateAssembly(generator);
        string resultReg = generator.getNewRegister();
        
        if (op == "+") {
            generator.emit("ADD " + resultReg + ", " + leftReg + ", " + rightReg);
        } else if (op == "-") {
            generator.emit("SUB " + resultReg + ", " + leftReg + ", " + rightReg);
        } else if (op == "==") {
            generator.emit("CMP " + leftReg + ", " + rightReg);
            generator.emit("MOV " + resultReg + ", #0");
            generator.emit("MOVEQ " + resultReg + ", #1");
        }
        return resultReg;
    }
};

class Assignment : public Statement {
public:
    string identifier;
    Expression* exp;
    
    Assignment(string identifier, Expression* exp)
        : identifier(identifier), exp(exp) {}
    
    string generateAssembly(CodeGenerator& generator) override {
        string valueReg = exp->generateAssembly(generator);
        string location = generator.getVariableLocation(identifier);
        generator.emit("STR " + valueReg + ", [" + location + "]");
        return "";
    }
};

class VarDeclaration : public Statement {
public:
    string type;
    string name;
    Expression* initializer;
    
    VarDeclaration(string type, string name, Expression* init)
        : type(type), name(name), initializer(init) {}
    
    string generateAssembly(CodeGenerator& generator) override {
        generator.declareVariable(name);
        if (initializer) {
            string valueReg = initializer->generateAssembly(generator);
            string location = generator.getVariableLocation(name);
            generator.emit("STR " + valueReg + ", [" + location + "]");
        }
        return "";
    }
};

class Block : public Statement {
public:
    vector<Statement*> statements;
    
    void addStatement(Statement* stmt) {
        statements.push_back(stmt);
    }
    
    string generateAssembly(CodeGenerator& generator) override {
        for (auto stmt : statements) {
            stmt->generateAssembly(generator);
        }
        return "";
    }
};

class If : public Statement {
public:
    Expression* condition;
    Statement* thenBranch;
    
    If(Expression* condition, Statement* thenBranch)
        : condition(condition), thenBranch(thenBranch) {}
    
    string generateAssembly(CodeGenerator& generator) override {
        string condReg = condition->generateAssembly(generator);
        string endLabel = generator.getNewLabel();
        
        generator.emit("CMP " + condReg + ", #1");
        generator.emit("BNE " + endLabel);
        
        thenBranch->generateAssembly(generator);
        
        generator.emit(endLabel + ":");
        return "";
    }
};

// Parser Class
class Parser {
private:
    vector<Token> tokens;
    size_t current = 0;

    Token peek() {
        if (current >= tokens.size()) return Token(TokenType::TOKEN_EOF, "");
        return tokens[current];
    }

    Token advance() {
        if (current >= tokens.size()) return Token(TokenType::TOKEN_EOF, "");
        return tokens[current++];
    }

    bool match(TokenType type) {
        if (peek().type == type) {
            advance();
            return true;
        }
        return false;
    }

    Expression* parseExpression() {
        return parseEquality();
    }

    Expression* parseEquality() {
        Expression* left = parseAdditive();
        
        while (peek().type == TokenType::TOKEN_EQUAL) {
            string op = advance().text;
            Expression* right = parseAdditive();
            left = new BinaryOp(op, left, right);
        }
        
        return left;
    }

    Expression* parseAdditive() {
        Expression* left = parsePrimary();
        
        while (peek().type == TokenType::TOKEN_PLUS || 
               peek().type == TokenType::TOKEN_MINUS) {
            string op = advance().text;
            Expression* right = parsePrimary();
            left = new BinaryOp(op, left, right);
        }
        
        return left;
    }

    Expression* parsePrimary() {
        if (match(TokenType::TOKEN_NUMBER)) {
            return new Number(stoi(tokens[current - 1].text));
        }
        
        if (match(TokenType::TOKEN_IDENTIFIER)) {
            return new Identifier(tokens[current - 1].text);
        }
        
        if (match(TokenType::TOKEN_LPAREN)) {
            Expression* expr = parseExpression();
            if (!match(TokenType::TOKEN_RPAREN)) {
                delete expr;
                throw runtime_error("Expected ')'");
            }
            return expr;
        }
        
        throw runtime_error("Expected expression");
    }

    Statement* parseStatement() {
        if (match(TokenType::TOKEN_INT)) {
            return parseVarDeclaration();
        }
        if (match(TokenType::TOKEN_IF)) {
            return parseIf();
        }
        if (peek().type == TokenType::TOKEN_IDENTIFIER) {
            return parseAssignment();
        }
        throw runtime_error("Expected statement");
    }

    Statement* parseVarDeclaration() {
        if (!match(TokenType::TOKEN_IDENTIFIER)) {
            throw runtime_error("Expected identifier after 'int'");
        }
        string name = tokens[current - 1].text;
        
        Expression* init = nullptr;
        if (match(TokenType::TOKEN_ASSIGN)) {
            init = parseExpression();
        }
        
        if (!match(TokenType::TOKEN_SEMICOLON)) {
            delete init;
            throw runtime_error("Expected ';'");
        }
        
        return new VarDeclaration("int", name, init);
    }

    Statement* parseAssignment() {
        string name = advance().text;
        
        if (!match(TokenType::TOKEN_ASSIGN)) {
            throw runtime_error("Expected '='");
        }
        
        Expression* value = parseExpression();
        
        if (!match(TokenType::TOKEN_SEMICOLON)) {
            delete value;
            throw runtime_error("Expected ';'");
        }
        
        return new Assignment(name, value);
    }

    Statement* parseIf() {
        if (!match(TokenType::TOKEN_LPAREN)) {
            throw runtime_error("Expected '('");
        }
        
        Expression* condition = parseExpression();
        
        if (!match(TokenType::TOKEN_RPAREN)) {
            delete condition;
            throw runtime_error("Expected ')'");
        }
        
        if (!match(TokenType::TOKEN_LBRACE)) {
            delete condition;
            throw runtime_error("Expected '{'");
        }
        
        Block* body = new Block();
        while (!match(TokenType::TOKEN_RBRACE)) {
            body->addStatement(parseStatement());
        }
        
        return new If(condition, body);
    }

public:
    Block* parse(const string& input) {
        // Tokenize input
        Lexer lexer(input);
        Token token;
        do {
            token = lexer.getNextToken();
            tokens.push_back(token);
        } while (token.type != TokenType::TOKEN_EOF);
        
        // Parse tokens
        Block* program = new Block();
        while (peek().type != TokenType::TOKEN_EOF) {
            program->addStatement(parseStatement());
        }
        return program;
    }
};

int main() {
    try {
        // Read input file
        ifstream input("input.txt");
        if (!input) {
            throw runtime_error("Cannot open input.txt");
        }
        string content((istreambuf_iterator<char>(input)),
                      istreambuf_iterator<char>());
        input.close();

        // Parse the input
        Parser parser;
        Block* ast = parser.parse(content);

        // Generate assembly
        CodeGenerator generator;
        
        // Generate code sections
        generator.generatePrelude();
        generator.generatePostlude();
        
        // Generate code from AST
        ast->generateAssembly(generator);
        
        // Generate program exit
        generator.generateEpilogue();

        // Save the generated assembly to output.s
        ofstream output("output.s");
        if (!output) {
            throw runtime_error("Cannot create output.s");
        }
        output << generator.getCurrentCode();
        output.close();

        cout << "Assembly code has been generated and saved to output.s" << endl;

        // Clean up
        delete ast;

    } catch (const exception& ex) {
        cerr << "Error: " << ex.what() << '\n';
        return 1;
    }
    
    return 0;
}