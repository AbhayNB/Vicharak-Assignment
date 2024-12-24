#include <iostream>
#include <fstream>
#include <cctype>
#include <cstring>
#include <string>
#include <vector>
using namespace std;
#define MAX_TOKEN_LEN 100

enum class TokenType
{
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
    TOKEN_SEMICOLON,  // ;
    TOKEN_UNKNOWN,
    TOKEN_EOF
};

class Token
{
public:
    TokenType type;
    string text;
    Token(TokenType type = TokenType::TOKEN_UNKNOWN, const string &text = "")
        : type(type), text(text) {}
};

class Lexer
{
private:
    ifstream file;
    char currentChar;
    char getNextChar();
    void ungetChar(char c);

public:
    explicit Lexer(const string &filename);
    ~Lexer();
    Token getNextToken();
};
Lexer::Lexer(const string &filename)
{
    file.open(filename);
    if (!file.is_open())
    {
        throw runtime_error("Failed to open file");
    }
    currentChar = getNextChar();
}
Lexer::~Lexer()
{
    if (file.is_open())
    {
        file.close();
    }
}

char Lexer::getNextChar()
{
    return file.get();
}

void Lexer::ungetChar(char c)
{
    if (file)
        file.unget();
}

Token Lexer::getNextToken()
{
    while (file)
    {
        if (isspace(currentChar))
        {
            currentChar = getNextChar();
            continue;
        }
        if (isalpha(currentChar))
        {
            string text;
            while (isalnum(currentChar))
            {
                text += currentChar;
                currentChar = getNextChar();
            }
            if (text == "int")
                return Token(TokenType::TOKEN_INT, text);
            if (text == "if")
                return Token(TokenType::TOKEN_IF, text);
            return Token(TokenType::TOKEN_IDENTIFIER, text);
        }
        if (isdigit(currentChar))
        {
            string text;
            while (isdigit(currentChar))
            {
                text += currentChar;
                currentChar = getNextChar();
            }
            return Token(TokenType::TOKEN_NUMBER, text);
        }
        switch (currentChar)
        {
        case '=':
            currentChar = getNextChar();
            if (currentChar == '=')
            {
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
        case '{':
            currentChar = getNextChar();
            return Token(TokenType::TOKEN_LBRACE, "{");
        case '}':
            currentChar = getNextChar();
            return Token(TokenType::TOKEN_RBRACE, "}");
        case '(':
            currentChar = getNextChar();
            return Token(TokenType::TOKEN_LBRACE, "(");
        case ')':
            currentChar = getNextChar();
            return Token(TokenType::TOKEN_RBRACE, ")");
        case ';':
            currentChar = getNextChar();
            return Token(TokenType::TOKEN_SEMICOLON, ";");

        default:
            currentChar = getNextChar();
            return Token(TokenType::TOKEN_UNKNOWN, string(1, currentChar));
        }
    }
    return Token(TokenType::TOKEN_EOF, "");
}

// Parser ======================

// AST Nodes--

class ATSNode
{
public:
    virtual ~ATSNode() = default;
};

class BinaryOp : public ATSNode
{
public:
    string op;
    ATSNode *left;
    ATSNode *right;
    BinaryOp(string op, ATSNode *left, ATSNode *right) : op(op), left(left), right(right) {}
};
class Block : public ATSNode {
public:
    vector<ATSNode*> statements;
    Block() {}
    void addStatement(ATSNode* stmt) {
        statements.push_back(stmt);
    }
};

class VarDeclaration : public ATSNode {
public:
    string type;
    string name;
    ATSNode* initializer;
    VarDeclaration(string type, string name, ATSNode* init) 
        : type(type), name(name), initializer(init) {}
};

class Number : public ATSNode
{
public:
    int value;
    Number(int value) : value(value) {}
};
class Identifier : public ATSNode
{
public:
    string name;
    Identifier(string name) : name(name) {}
};

class Assignment : public ATSNode
{
public:
    string identifier;
    ATSNode *exp;
    Assignment(string identifier, ATSNode *exp) : identifier(identifier), exp(exp) {}
};
class If : public ATSNode
{
public:
    ATSNode *cond;
    ATSNode *body;
    If(ATSNode *cond, ATSNode *body) : cond(cond), body(body) {}
};

class Parser
{
private:
    vector<Token> tokens;
    size_t current;

    Token peek()
    {
        return tokens[current];
    }
    Token advance()
    {
        return tokens[current++];
    }
    bool match(TokenType type)
    {
        if (peek().type == type)
        {
            advance();
            return true;
        }
        return false;
    }
    void error(const string &message)
    {
        throw runtime_error("syntax error" + message);
    }

public:
    Parser(const vector<Token>& tokens) : tokens(tokens), current(0) {}

    ATSNode* parseExpression() {
        ATSNode* left = parsePrimary();
        
        while (match(TokenType::TOKEN_PLUS) || 
               match(TokenType::TOKEN_MINUS) || 
               match(TokenType::TOKEN_EQUAL)) {
            string op = tokens[current - 1].text;
            ATSNode* right = parsePrimary();
            left = new BinaryOp(op, left, right);
        }
        return left;
    }
    ATSNode *parsePrimary()
    {
        if (match(TokenType::TOKEN_NUMBER))
        {
            return new Number(stoi(tokens[current - 1].text));
        }
        else if (match(TokenType::TOKEN_IDENTIFIER))
        {
            return new Identifier(tokens[current - 1].text);
        }
        else
        {
            error("expected number or identifier");
        }
        return nullptr;
    }
    ATSNode *parseAssignment()
    {
        if (!match(TokenType::TOKEN_IDENTIFIER))
        {
            error("Expected an identifier");
        }
        string identifier = tokens[current - 1].text;
        if (!match(TokenType::TOKEN_ASSIGN))
        {
            error("Expected '=' after identifier");
        }
        ATSNode *expression = parseExpression();
        return new Assignment(identifier, expression);
    }
    ATSNode* parseStatement() {
        if (match(TokenType::TOKEN_INT)) {
            return parseVarDeclaration();
        }
        if (match(TokenType::TOKEN_IF)) {
            return parseIf();
        }
        if (match(TokenType::TOKEN_IDENTIFIER)) {
            return parseAssignment();
        }
        error("Expected statement");
        return nullptr;
    }
    ATSNode* parseVarDeclaration() {
        if (!match(TokenType::TOKEN_IDENTIFIER)) {
            error("Expected identifier after 'int'");
        }
        string name = tokens[current - 1].text;
        
        if (!match(TokenType::TOKEN_ASSIGN)) {
            error("Expected '=' after identifier");
        }
        
        ATSNode* init = parseExpression();
        
        if (!match(TokenType::TOKEN_SEMICOLON)) {
            error("Expected ';' after declaration");
        }
        
        return new VarDeclaration("int", name, init);
    }
    ATSNode* parseIf() {
        if (!match(TokenType::TOKEN_LPAREN)) {
            error("Expected '(' after if");
        }
        
        ATSNode* condition = parseExpression();
        
        if (!match(TokenType::TOKEN_RPAREN)) {
            error("Expected ')' after condition");
        }
        
        if (!match(TokenType::TOKEN_LBRACE)) {
            error("Expected '{' after if condition");
        }
        
        Block* body = new Block();
        while (!match(TokenType::TOKEN_RBRACE)) {
            body->addStatement(parseStatement());
        }
        
        return new If(condition, body);
    }

    ATSNode *parse()
    {
        if (match(TokenType::TOKEN_IF))
        {
            return parseIf();
        }
        else if (match(TokenType::TOKEN_IDENTIFIER))
        {
            return parseAssignment();
        }
        else
        {
            error("Unexpected token");
        }
        return nullptr;
    }
};

    // int main()
    // {
    //     try
    //     {
    //         Lexer lexer("input.txt");
    //         vector<Token> tokens;
    //         Token token = lexer.getNextToken();
    //         while (token.type != TokenType::TOKEN_EOF)
    //         {
    //             tokens.push_back(token);
    //             token = lexer.getNextToken();
    //         }
    //         Parser parse(tokens);
    //         ATSNode *ast = parse.parse();
    //         cout<<"Successfully genrated ATS"<<'\n';
    //     }
    //     catch (const exception &ex)
    //     {
    //         cerr << "Caught Error: " << ex.what() << '\n';
    //         return 1;
    //     }
    //     return 0;
    // }

    // Main Function
int main() {
    try {
        Lexer lexer("input.txt");
        Token token;

        do {
            token = lexer.getNextToken();
            std::cout << "Token: " << static_cast<int>(token.type) << ", Text: " << token.text << '\n';
        } while (token.type != TokenType::TOKEN_EOF);

    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}