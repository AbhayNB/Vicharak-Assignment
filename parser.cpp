#include <iostream>
#include <fstream>
#include <cctype>
#include <cstring>
#include <string>
#include <vector>
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
    TOKEN_LPAREN,    // (
    TOKEN_RPAREN,    // )
    TOKEN_LBRACE,    // {
    TOKEN_RBRACE,    // }
    TOKEN_NOT_EQUAL, // !=
    TOKEN_SEMICOLON, // ;
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
    ifstream file;
    char currentChar;
    
    char getNextChar() {
        return file.get();
    }
    
    void ungetChar(char c) {
        if (file) {
            file.unget();
        }
    }

public:
    explicit Lexer(const string& filename) {
        file.open(filename);
        if (!file.is_open()) {
            throw runtime_error("Failed to open file: " + filename);
        }
        currentChar = getNextChar();
    }
    
    ~Lexer() {
        if (file.is_open()) {
            file.close();
        }
    }

    Token getNextToken() {
        while (file) {
            // Skip whitespace
            if (isspace(currentChar)) {
                currentChar = getNextChar();
                continue;
            }

            // Identify keywords and identifiers
            if (isalpha(currentChar)) {
                string text;
                while (isalnum(currentChar)) {
                    text += currentChar;
                    currentChar = getNextChar();
                }
                
                if (text == "int") return Token(TokenType::TOKEN_INT, text);
                if (text == "if") return Token(TokenType::TOKEN_IF, text);
                return Token(TokenType::TOKEN_IDENTIFIER, text);
            }

            // Handle numbers
            if (isdigit(currentChar)) {
                string text;
                while (isdigit(currentChar)) {
                    text += currentChar;
                    currentChar = getNextChar();
                }
                return Token(TokenType::TOKEN_NUMBER, text);
            }

            // Handle operators and special characters
            switch (currentChar) {
                case '=':
                    currentChar = getNextChar();
                    if (currentChar == '=') {
                        currentChar = getNextChar();
                        return Token(TokenType::TOKEN_EQUAL, "==");
                    }
                    return Token(TokenType::TOKEN_ASSIGN, "=");
                
                case '+':
                    currentChar = getNextChar();
                    return Token(TokenType::TOKEN_PLUS, "+");
                
                case '-':
                    currentChar = getNextChar();
                    return Token(TokenType::TOKEN_MINUS, "-");
                
                case '!':
                    currentChar = getNextChar();
                    if (currentChar == '=') {
                        currentChar = getNextChar();
                        return Token(TokenType::TOKEN_NOT_EQUAL, "!=");
                    }
                    return Token(TokenType::TOKEN_UNKNOWN, "!");
                
                case '{':
                    currentChar = getNextChar();
                    return Token(TokenType::TOKEN_LBRACE, "{");
                
                case '}':
                    currentChar = getNextChar();
                    return Token(TokenType::TOKEN_RBRACE, "}");
                
                case '(':
                    currentChar = getNextChar();
                    return Token(TokenType::TOKEN_LPAREN, "(");
                
                case ')':
                    currentChar = getNextChar();
                    return Token(TokenType::TOKEN_RPAREN, ")");
                
                case ';':
                    currentChar = getNextChar();
                    return Token(TokenType::TOKEN_SEMICOLON, ";");
                
                default:
                    char invalidChar = currentChar;
                    currentChar = getNextChar();
                    return Token(TokenType::TOKEN_UNKNOWN, string(1, invalidChar));
            }
        }
        return Token(TokenType::TOKEN_EOF, "");
    }
};

// AST Node Classes
class ASTNode {
public:
    virtual ~ASTNode() = default;
};

class Expression : public ASTNode {
public:
    virtual ~Expression() = default;
};

class Statement : public ASTNode {
public:
    virtual ~Statement() = default;
};

class BinaryOp : public Expression {
public:
    string op;
    Expression* left;
    Expression* right;
    
    BinaryOp(string op, Expression* left, Expression* right)
        : op(op), left(left), right(right) {}
        
    ~BinaryOp() {
        delete left;
        delete right;
    }
};

class Number : public Expression {
public:
    int value;
    explicit Number(int value) : value(value) {}
};

class Identifier : public Expression {
public:
    string name;
    explicit Identifier(string name) : name(name) {}
};

class Block : public Statement {
public:
    vector<Statement*> statements;
    
    ~Block() {
        for (auto stmt : statements) {
            delete stmt;
        }
    }
    
    void addStatement(Statement* stmt) {
        statements.push_back(stmt);
    }
};

class VarDeclaration : public Statement {
public:
    string type;
    string name;
    Expression* initializer;
    
    VarDeclaration(string type, string name, Expression* init)
        : type(type), name(name), initializer(init) {}
        
    ~VarDeclaration() {
        delete initializer;
    }
};

class Assignment : public Statement {
public:
    string identifier;
    Expression* exp;
    
    Assignment(string identifier, Expression* exp)
        : identifier(identifier), exp(exp) {}
        
    ~Assignment() {
        delete exp;
    }
};

class If : public Statement {
public:
    Expression* condition;
    Statement* thenBranch;
    Statement* elseBranch;
    
    If(Expression* condition, Statement* thenBranch, Statement* elseBranch = nullptr)
        : condition(condition), thenBranch(thenBranch), elseBranch(elseBranch) {}
        
    ~If() {
        delete condition;
        delete thenBranch;
        delete elseBranch;
    }
};

// Parser Class
class Parser {
private:
    vector<Token> tokens;
    size_t current;

    Token peek() {
        if (current >= tokens.size()) {
            return Token(TokenType::TOKEN_EOF, "");
        }
        return tokens[current];
    }

    Token advance() {
        if (current >= tokens.size()) {
            return Token(TokenType::TOKEN_EOF, "");
        }
        return tokens[current++];
    }

    bool match(TokenType type) {
        if (peek().type == type) {
            advance();
            return true;
        }
        return false;
    }

    void error(const string& message) {
        throw runtime_error("Parse error: " + message);
    }

public:
    explicit Parser(const vector<Token>& tokens) : tokens(tokens), current(0) {}

    Block* parseProgram() {
        Block* program = new Block();
        while (peek().type != TokenType::TOKEN_EOF) {
            try {
                program->addStatement(parseStatement());
            } catch (const runtime_error& e) {
                delete program;
                throw;
            }
        }
        return program;
    }

    Statement* parseStatement() {
        if (match(TokenType::TOKEN_INT)) {
            return parseVarDeclaration();
        }
        if (match(TokenType::TOKEN_IF)) {
            return parseIf();
        }
        if (peek().type == TokenType::TOKEN_IDENTIFIER) {
            return parseAssignmentStatement();
        }
        error("Expected statement");
        return nullptr;
    }

    Statement* parseVarDeclaration() {
        if (!match(TokenType::TOKEN_IDENTIFIER)) {
            error("Expected identifier after 'int'");
        }
        string name = tokens[current - 1].text;
        
        Expression* init = nullptr;
        if (match(TokenType::TOKEN_ASSIGN)) {
            init = parseExpression();
        }
        
        if (!match(TokenType::TOKEN_SEMICOLON)) {
            delete init;
            error("Expected ';' after declaration");
        }
        
        return new VarDeclaration("int", name, init);
    }

    Statement* parseAssignmentStatement() {
        Statement* assignment = parseAssignment();
        if (!match(TokenType::TOKEN_SEMICOLON)) {
            delete assignment;
            error("Expected ';' after assignment");
        }
        return assignment;
    }

    Statement* parseAssignment() {
        if (!match(TokenType::TOKEN_IDENTIFIER)) {
            error("Expected identifier");
        }
        string identifier = tokens[current - 1].text;
        
        if (!match(TokenType::TOKEN_ASSIGN)) {
            error("Expected '=' after identifier");
        }
        
        Expression* expression = parseExpression();
        return new Assignment(identifier, expression);
    }

    Expression* parseExpression() {
        return parseEquality();
    }

    Expression* parseEquality() {
        Expression* left = parseAdditive();
        
        while (peek().type == TokenType::TOKEN_EQUAL || 
               peek().type == TokenType::TOKEN_NOT_EQUAL) {
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
                error("Expected ')'");
            }
            return expr;
        }
        
        error("Expected expression");
        return nullptr;
    }

    Statement* parseIf() {
        if (!match(TokenType::TOKEN_LPAREN)) {
            error("Expected '(' after 'if'");
        }
        
        Expression* condition = parseExpression();
        
        if (!match(TokenType::TOKEN_RPAREN)) {
            delete condition;
            error("Expected ')'");
        }
        
        if (!match(TokenType::TOKEN_LBRACE)) {
            delete condition;
            error("Expected '{'");
        }
        
        Block* body = new Block();
        while (!match(TokenType::TOKEN_RBRACE) && 
               peek().type != TokenType::TOKEN_EOF) {
            try {
                body->addStatement(parseStatement());
            } catch (const runtime_error& e) {
                delete condition;
                delete body;
                throw;
            }
        }
        
        return new If(condition, body);
    }
};

// Main function
int main() {
    try {
        // Create lexer and collect tokens
        Lexer lexer("input.txt");
        vector<Token> tokens;
        Token token;
        
        // Tokenize the input
        do {
            token = lexer.getNextToken();
            tokens.push_back(token);
            cout << "Token: " << static_cast<int>(token.type) 
                 << ", Text: " << token.text << '\n';
        } while (token.type != TokenType::TOKEN_EOF);
        
        // Parse the tokens
        Parser parser(tokens);
        ASTNode* ast = parser.parseProgram();
        cout << "Successfully generated AST\n";
        
        // Clean up
        delete ast;
        
    } catch (const exception& ex) {
        cerr << "Error: " << ex.what() << '\n';
        return 1;
    }
    
    return 0;
}