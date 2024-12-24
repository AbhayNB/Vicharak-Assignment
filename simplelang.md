# SimpleLang & it's Complier

### Define SimpleLang Syntax and Semantics

#### **Language Constructs and Syntax**

1. **Variable Declaration**
   - **Syntax**: `int varName;`
   - **Explanation**: Declares a variable `varName` of type integer. The variable is initialized to 0 by default.
   - **Example**:
     ```simplelang
     int a;
     int b;
     ```

2. **Assignment**
   - **Syntax**: `varName = expression;`
   - **Explanation**: Assigns the result of the evaluated `expression` to the variable `varName`.
   - **Example**:
     ```simplelang
     a = 10;
     b = a + 5;
     ```

3. **Arithmetic Operations**
   - **Supported Operators**: `+`, `-`
   - **Syntax**: `varName = operand1 operator operand2;`
   - **Explanation**: Performs the arithmetic operation (`+` or `-`) on `operand1` and `operand2`, then assigns the result to `varName`.
   - **Example**:
     ```simplelang
     c = a + b;
     d = c - 2;
     ```

4. **Conditionals**
   - **Syntax**: `if (condition) { statements; }`
   - **Explanation**: Executes the block of `statements` if the `condition` evaluates to true.
   - **Supported Conditions**: `==`, `!=`, `<`, `>`, `<=`, `>=`
   - **Example**:
     ```simplelang
     if (a == b) {
         a = a + 1;
     }
     ```

---

#### **Semantics**

1. **Variable Declaration**:
   - **Behavior**: Allocates memory for the variable and initializes it to 0.
   - **Example Execution**:
     ```simplelang
     int x;
     ```
     - Memory: `x = 0`

2. **Assignment**:
   - **Behavior**: Evaluates the expression on the right-hand side and stores the result in the variable on the left-hand side.
   - **Example Execution**:
     ```simplelang
     x = 10;
     ```
     - Memory: `x = 10`

3. **Arithmetic Operations**:
   - **Behavior**: Performs the specified operation on the operands and assigns the result to the variable.
   - **Example Execution**:
     ```simplelang
     a = 5;
     b = 3;
     c = a + b;
     ```
     - Memory: `a = 5, b = 3, c = 8`

4. **Conditionals**:
   - **Behavior**: Evaluates the condition. If true, executes the statements inside the block.
   - **Example Execution**:
     ```simplelang
     if (a == b) {
         a = a + 1;
     }
     ```
     - Memory (if `a = 5` and `b = 5`): `a = 6`
     - Memory (if `a = 5` and `b = 3`): `a = 5`

---

#### **Mapping to Assembly Instructions**

1. **Variable Declaration**:
   - SimpleLang: `int x;`
   - Assembly:
     ```assembly
     MEM x  ; Allocate memory for x
     LOAD 0 ; Initialize x to 0
     STORE x
     ```

2. **Assignment**:
   - SimpleLang: `x = 10;`
   - Assembly:
     ```assembly
     LOAD 10
     STORE x
     ```

3. **Arithmetic Operations**:
   - SimpleLang: `c = a + b;`
   - Assembly:
     ```assembly
     LOAD a
     ADD b
     STORE c
     ```

4. **Conditionals**:
   - SimpleLang: `if (a == b) { c = c + 1; }`
   - Assembly:
     ```assembly
     LOAD a
     CMP b
     JNZ end_if
     LOAD c
     ADD 1
     STORE c
     end_if:
     ```

---

#### **Reference Guide**

**Variable Declaration**:
- Syntax: `int varName;`
- Example: `int a;`
- Assembly Mapping:
  ```assembly
  MEM varName
  LOAD 0
  STORE varName
  ```

**Assignment**:
- Syntax: `varName = expression;`
- Example: `a = 5;`
- Assembly Mapping:
  ```assembly
  LOAD value_or_expression
  STORE varName
  ```

**Arithmetic Operations**:
- Syntax: `varName = operand1 + operand2;`
- Example: `c = a - b;`
- Assembly Mapping:
  ```assembly
  LOAD operand1
  SUB operand2  ; Use ADD for addition
  STORE varName
  ```

**Conditionals**:
- Syntax: `if (condition) { statements; }`
- Example: `if (x < y) { z = z + 1; }`
- Assembly Mapping:
  ```assembly
  LOAD x
  CMP y
  JGE skip_block
  LOAD z
  ADD 1
  STORE z
  skip_block:
  ```

---
# Compiler Design and Implementation Documentation

## Overview

This document provides a detailed explanation of the design and implementation of a basic compiler written in C++. The compiler includes components such as a lexer, parser, abstract syntax tree (AST) nodes, and a code generator. It processes a simple programming language with features like variable declarations, arithmetic expressions, conditional statements, and assembly code generation.

---

## 1. Lexer

The lexer is responsible for breaking the input source code into meaningful tokens. Each token represents a fundamental element of the programming language, such as keywords, identifiers, operators, or punctuation.

### Token Types

The token types are defined as an enumeration `TokenType`. Examples include:

- `TOKEN_INT` - for the `int` keyword.
- `TOKEN_IDENTIFIER` - for variable names.
- `TOKEN_NUMBER` - for numeric literals.
- `TOKEN_PLUS`, `TOKEN_MINUS` - for arithmetic operators.
- `TOKEN_IF` - for conditional statements.
- `TOKEN_EQUAL`, `TOKEN_ASSIGN` - for comparison and assignment operators.

### Token Class

The `Token` class encapsulates a token’s type and its associated text.

```cpp
class Token {
public:
    TokenType type;
    string text;
    Token(TokenType type = TokenType::TOKEN_UNKNOWN, const string& text = "")
        : type(type), text(text) {}
};
```

### Lexer Implementation

The `Lexer` class processes the input string, identifies tokens, and skips whitespace. It supports:

- Identifiers and keywords.
- Numeric literals.
- Single-character and multi-character operators.

#### Key Methods:

- `currentChar`: Retrieves the current character.
- `advance`: Moves to the next character.
- `skipWhitespace`: Skips over spaces, tabs, and newlines.
- `getNextToken`: Identifies and returns the next token.

---

## 2. Parser

The parser transforms the stream of tokens into a structured representation of the program using an Abstract Syntax Tree (AST).

### Parser Components

#### Expressions:

- **Equality Expression:** Handles equality checks (e.g., `==`).
- **Additive Expression:** Manages addition and subtraction (e.g., `+`, `-`).
- **Primary Expression:** Processes numbers, identifiers, and parenthesized expressions.

#### Statements:

- **Variable Declarations:** e.g., `int x = 5;`
- **Assignments:** e.g., `x = 10;`
- **Conditional Statements:** e.g., `if (x == 5) { ... }`

#### Key Methods:

- `parseExpression`: Parses expressions recursively.
- `parseStatement`: Determines the type of statement to parse.
- `match`: Checks if the current token matches the expected type.
- `advance`: Consumes the current token and moves to the next.

---

## 3. Abstract Syntax Tree (AST)

The AST represents the hierarchical structure of the source code.

### Node Types

- **Expression:** Base class for expressions.
- **Statement:** Base class for statements.

#### Implementations:

1. **Number**: Represents numeric literals.

   ```cpp
   class Number : public Expression {
   public:
       int value;
       explicit Number(int value) : value(value) {}
   };
   ```

2. **Identifier**: Represents variable names.

3. **BinaryOp**: Represents binary operations (e.g., addition).

4. **Assignment**: Represents assignment statements.

5. **VarDeclaration**: Represents variable declarations.

6. **Block**: Represents a sequence of statements.

7. **If**: Represents conditional statements.

---

## 4. Code Generator

The `CodeGenerator` class converts the AST into assembly code for a hypothetical machine.

### Responsibilities:

1. **Register and Label Management:**

   - `getNewRegister`: Allocates new registers.
   - `getNewLabel`: Generates unique labels.

2. **Emit Assembly Code:**

   - `emit`: Outputs assembly instructions.

3. **Variable Management:**

   - `declareVariable`: Declares variables in the data section.
   - `getVariableLocation`: Retrieves memory locations of variables.

### Code Generation Phases:

1. **Prelude:** Sets up the data section.
2. **Postlude:** Sets up the text section and entry point.
3. **Epilogue:** Emits exit instructions.

#### Example:

For an arithmetic operation `x + 5`:

```assembly
MOV R1, x
MOV R2, #5
ADD R3, R1, R2
```

---

## 5. Example Program

Source Code:

```c
int x = 5;
x = x + 10;
if (x == 15) {
    int y = x - 5;
}
```

Generated Assembly:

```assembly
.section .data
x: .word 0
y: .word 0

.section .text
.global _start
_start:
MOV R1, #5
STR R1, [x]
LDR R2, [x]
MOV R3, #10
ADD R4, R2, R3
STR R4, [x]
LDR R5, [x]
CMP R5, #15
BNE L0
MOV R6, #5
SUB R7, R5, R6
STR R7, [y]
L0:
MOV R7, #1
MOV R0, #0
SWI 0
```

---

## 6. Error Handling

The compiler includes error handling to detect and report syntax and semantic errors:

- **Lexer:** Unrecognized characters result in `TOKEN_UNKNOWN`.
- **Parser:** Throws exceptions for unexpected tokens.
- **Code Generator:** Ensures variables are declared before use.

---

## 7. Extensibility

The compiler is modular, making it easy to add:

- New token types.
- Additional language constructs (e.g., loops, functions).
- Advanced optimizations in code generation.

---

## Conclusion

This compiler demonstrates the essential components of a basic compilation process. Its modular design and clear separation of concerns make it a solid foundation for learning compiler construction or building more sophisticated language tools.


