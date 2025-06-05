#include "../../include/lexer/token.hpp"

#include <map>
#include <sstream>

std::string Token::toString() const
{
    static const std::map<TokenType, std::string> tokenNames = {
        {TokenType::ENUM, "ENUM"},
        {TokenType::CLASS, "CLASS"},
        {TokenType::DATA, "DATA"},
        {TokenType::BASE, "BASE"},
        {TokenType::INHERITS, "INHERITS"},
        {TokenType::INIT, "INIT"},
        {TokenType::FN, "FN"},
        {TokenType::FUN, "FUN"},
        {TokenType::VAR, "VAR"},
        {TokenType::VAL, "VAL"},
        {TokenType::LET, "LET"},
        {TokenType::RETURN, "RETURN"},
        {TokenType::IF, "IF"},
        {TokenType::ELSE, "ELSE"},
        {TokenType::WHEN, "WHEN"},
        {TokenType::FOR, "FOR"},
        {TokenType::IN, "IN"},
        {TokenType::IS, "IS"},
        {TokenType::SELF, "SELF"},
        {TokenType::OPERATOR, "OPERATOR"},
        // {TokenType::PRINT, "PRINT"},
        // {TokenType::PRINTLN, "PRINTLN"},
        {TokenType::VOID, "VOID"},
        {TokenType::INT_TYPE, "INT_TYPE"},
        {TokenType::FLOAT_TYPE, "FLOAT_TYPE"},
        {TokenType::BOOL_TYPE, "BOOL_TYPE"},
        {TokenType::STRING_TYPE, "STRING_TYPE"},
        {TokenType::ARRAY_TYPE, "ARRAY_TYPE"},
        {TokenType::IDENTIFIER, "IDENTIFIER"},
        {TokenType::INT_LITERAL, "INT_LITERAL"},
        {TokenType::FLOAT_LITERAL, "FLOAT_LITERAL"},
        {TokenType::BOOL_LITERAL, "BOOL_LITERAL"},
        {TokenType::STRING_LITERAL, "STRING_LITERAL"},
        {TokenType::ASSIGN, "ASSIGN"},
        {TokenType::EQ, "EQ"},
        {TokenType::NEQ, "NEQ"},
        {TokenType::LT, "LT"},
        {TokenType::LE, "LE"},
        {TokenType::GT, "GT"},
        {TokenType::GE, "GE"},
        {TokenType::PLUS, "PLUS"},
        {TokenType::MINUS, "MINUS"},
        {TokenType::MULT, "MULT"},
        {TokenType::DIV, "DIV"},
        {TokenType::MOD, "MOD"},
        {TokenType::AND, "AND"},
        {TokenType::OR, "OR"},
        {TokenType::NOT, "NOT"},
        {TokenType::ARROW, "ARROW"},
        {TokenType::COLON, "COLON"},
        // {TokenType::SEMICOLON, "SEMICOLON"},
        {TokenType::COMMA, "COMMA"},
        {TokenType::DOT, "DOT"},
        {TokenType::LPAREN, "LPAREN"},
        {TokenType::RPAREN, "RPAREN"},
        {TokenType::LBRACE, "LBRACE"},
        {TokenType::RBRACE, "RBRACE"},
        {TokenType::LBRACKET, "LBRACKET"},
        {TokenType::RBRACKET, "RBRACKET"},
        {TokenType::LANGLE, "LANGLE"},
        {TokenType::RANGLE, "RANGLE"},
        {TokenType::END_OF_FILE, "END_OF_FILE"},
        {TokenType::ERROR, "ERROR"}};

    std::stringstream ss;
    if (!location.filename.empty()) {
        ss << location.filename << ":";
    }
    ss << location.line << ":" << location.column << " " << tokenNames.at(type);


    if (type == TokenType::IDENTIFIER || type == TokenType::STRING_LITERAL) {
        ss << " '" << std::get<std::string>(value) << "'";
    }
    else if (type == TokenType::INT_LITERAL) {
        ss << " " << std::get<int>(value);
    }
    else if (type == TokenType::FLOAT_LITERAL) {
        ss << " " << std::get<float>(value);
    }
    else if (type == TokenType::BOOL_LITERAL) {
        ss << " " << (std::get<bool>(value) ? "true" : "false");
    }

    return ss.str();
}