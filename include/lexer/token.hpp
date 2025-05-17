#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>
#include <variant>

enum class TokenType
{
    // 关键字
    ENUM,
    CLASS,
    DATA,
    BASE,
    INHERITS,
    INIT,
    FN,
    FUN,
    VAR,
    VAL,
    LET,
    RETURN,
    IF,
    ELSE,
    WHEN,
    FOR,
    IN,
    IS,
    SELF,
    OPERATOR,
    // PRINT,
    // PRINTLN,
    VOID,
    INT_TYPE,
    FLOAT_TYPE,
    BOOL_TYPE,
    STRING_TYPE,
    ARRAY_TYPE,

    // 标识符和字面量
    IDENTIFIER,
    INT_LITERAL,
    FLOAT_LITERAL,
    BOOL_LITERAL,
    STRING_LITERAL,

    // 运算符和标点符号
    ASSIGN,
    EQ,
    NEQ,
    LT,
    LE,
    GT,
    GE,
    PLUS,
    MINUS,
    MULT,
    DIV,
    MOD,
    AND,
    OR,
    NOT,
    ARROW,
    COLON,
    // SEMICOLON,
    COMMA,
    DOT,
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    LBRACKET,
    RBRACKET,
    LANGLE,
    RANGLE,

    // 特殊标记
    END_OF_FILE,
    ERROR
};

struct Token
{
    TokenType                                                   type;
    std::variant<std::monostate, int, float, bool, std::string> value;
    int                                                         line;
    int                                                         column;

    Token(TokenType type, int line, int column)
        : type(type)
        , value(std::monostate{})
        , line(line)
        , column(column)
    {}

    Token(TokenType type, int value, int line, int column)
        : type(type)
        , value(value)
        , line(line)
        , column(column)
    {}

    Token(TokenType type, float value, int line, int column)
        : type(type)
        , value(value)
        , line(line)
        , column(column)
    {}

    Token(TokenType type, bool value, int line, int column)
        : type(type)
        , value(value)
        , line(line)
        , column(column)
    {}

    Token(TokenType type, std::string value, int line, int column)
        : type(type)
        , value(std::move(value))
        , line(line)
        , column(column)
    {}

    std::string toString() const;
};

#endif   // TOKEN_HPP