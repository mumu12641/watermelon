#ifndef TOKEN_HPP
#define TOKEN_HPP

#include <string>
#include <variant>
#include "../utils/error.hpp"

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
    SEMICOLON,
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
    // int                                                         line;
    // int                                                         column;
    // std::string                                                 filename;
    Location location;

    Token(TokenType type, int line, int column, const std::string& filename = "")
        : type(type)
        , value(std::monostate{})
        , location(line, column, filename)
    {
    }

    Token(TokenType type, int value, int line, int column, const std::string& filename = "")
        : type(type)
        , value(value)
        , location(line, column, filename)
    {
    }

    Token(TokenType type, float value, int line, int column, const std::string& filename = "")
        : type(type)
        , value(value)
        , location(line, column, filename)
    {
    }

    Token(TokenType type, bool value, int line, int column, const std::string& filename = "")
        : type(type)
        , value(value)
        , location(line, column, filename)
    {
    }

    Token(TokenType type, std::string value, int line, int column, const std::string& filename = "")
        : type(type)
        , value(std::move(value))
        , location(line, column, filename)
    {
    }

    std::string toString() const;
};

#endif   // TOKEN_HPP