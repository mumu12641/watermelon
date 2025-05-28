#include "../../include/lexer/lexer.hpp"

#include <cctype>
#include <iostream>
#include <optional>
#include <sstream>

const std::map<std::string, TokenType> Lexer::keywords = {{"enum", TokenType::ENUM},
                                                          {"class", TokenType::CLASS},
                                                          {"data", TokenType::DATA},
                                                          {"base", TokenType::BASE},
                                                          {"inherits", TokenType::INHERITS},
                                                          {"init", TokenType::INIT},
                                                          {"fn", TokenType::FN},
                                                          {"fun", TokenType::FUN},
                                                          {"var", TokenType::VAR},
                                                          {"val", TokenType::VAL},
                                                          {"return", TokenType::RETURN},
                                                          {"if", TokenType::IF},
                                                          {"else", TokenType::ELSE},
                                                          {"when", TokenType::WHEN},
                                                          {"for", TokenType::FOR},
                                                          {"in", TokenType::IN},
                                                          {"is", TokenType::IS},
                                                          {"self", TokenType::SELF},
                                                          {"operator", TokenType::OPERATOR},
                                                          //   {"print", TokenType::PRINT},
                                                          //   {"println", TokenType::PRINTLN},
                                                          {"void", TokenType::VOID},
                                                          {"int", TokenType::INT_TYPE},
                                                          {"float", TokenType::FLOAT_TYPE},
                                                          {"bool", TokenType::BOOL_TYPE},
                                                          {"string", TokenType::STRING_TYPE},
                                                          {"String", TokenType::STRING_TYPE},
                                                          {"Array", TokenType::ARRAY_TYPE},
                                                          {"true", TokenType::BOOL_LITERAL},
                                                          {"false", TokenType::BOOL_LITERAL}};

Lexer::Lexer(std::ifstream& file, const std::string& filename)
    : filename(filename)
{
    std::stringstream buffer;
    buffer << file.rdbuf();
    source = buffer.str();
}

char Lexer::peek() const
{
    if (position >= source.length()) {
        return '\0';
    }
    return source[position];
}

char Lexer::advance()
{
    char current = peek();
    position++;

    if (current == '\n') {
        line++;
        column = 1;
    }
    else {
        column++;
    }

    return current;
}

bool Lexer::match(char expected)
{
    if (peek() != expected) {
        return false;
    }

    advance();
    return true;
}

void Lexer::skipWhitespace()
{
    while (isspace(peek())) {
        advance();
    }
}

void Lexer::skipComment()
{
    if (peek() == '/' && position + 1 < source.length() && source[position + 1] == '/') {
        advance();
        advance();

        while (peek() != '\n' && peek() != '\0') {
            advance();
        }
    }
    else if (peek() == '/' && position + 1 < source.length() && source[position + 1] == '*') {
        advance();
        advance();

        while (!(peek() == '*' && position + 1 < source.length() && source[position + 1] == '/')) {
            if (peek() == '\0') {
                return;
            }
            advance();
        }
        advance();
        advance();
    }
}

Token Lexer::scanIdentifier()
{
    int         startLine   = line;
    int         startColumn = column;
    std::string identifier;

    while (isalnum(peek()) || peek() == '_') {
        identifier += advance();
    }

    // 检查是否是关键字
    auto it = keywords.find(identifier);
    if (it != keywords.end()) {
        if (it->second == TokenType::BOOL_LITERAL) {
            return Token(it->second, identifier == "true", startLine, startColumn, filename);
        }
        return Token(it->second, startLine, startColumn, filename);
    }

    return Token(TokenType::IDENTIFIER, identifier, startLine, startColumn, filename);
}

Token Lexer::scanNumber()
{
    int         startLine   = line;
    int         startColumn = column;
    std::string number;
    bool        isFloat = false;

    while (isdigit(peek())) {
        number += advance();
    }

    if (peek() == '.' && isdigit(source[position + 1])) {
        isFloat = true;
        number += advance();

        while (isdigit(peek())) {
            number += advance();
        }
    }

    if (isFloat) {
        return Token(TokenType::FLOAT_LITERAL, std::stof(number), startLine, startColumn, filename);
    }
    else {
        return Token(TokenType::INT_LITERAL, std::stoi(number), startLine, startColumn, filename);
    }
}

Token Lexer::scanString()
{
    int         startLine   = line;
    int         startColumn = column;
    std::string str;

    // 跳过开始的引号
    advance();

    while (peek() != '"' && peek() != '\0') {
        if (peek() == '\\' && position + 1 < source.length()) {
            advance();   // 跳过反斜杠

            // 处理转义字符
            switch (peek()) {
                case 'n': str += '\n'; break;
                case 't': str += '\t'; break;
                case 'r': str += '\r'; break;
                case '\\': str += '\\'; break;
                case '"': str += '"'; break;
                default: str += peek(); break;
            }

            advance();
        }
        else {
            str += advance();
        }
    }

    if (peek() == '\0') {
        return Token(TokenType::ERROR, "Unterminated string", startLine, startColumn, filename);
    }

    advance();

    return Token(TokenType::STRING_LITERAL, str, startLine, startColumn, filename);
}

Token Lexer::nextToken()
{
    while (true) {
        skipWhitespace();

        if ((peek() == '/' && position + 1 < source.length() &&
             (source[position + 1] == '/' || source[position + 1] == '*'))) {
            skipComment();
        }
        else {
            break;
        }
    }

    int currentLine   = line;
    int currentColumn = column;

    if (position >= source.length()) {
        return Token(TokenType::END_OF_FILE, currentLine, currentColumn, filename);
    }

    char c = peek();

    if (isalpha(c) || c == '_') {
        return scanIdentifier();
    }

    if (isdigit(c)) {
        return scanNumber();
    }

    if (c == '"') {
        return scanString();
    }

    switch (c) {
        case '=':
            advance();
            if (match('=')) {
                return Token(TokenType::EQ, currentLine, currentColumn, filename);
            }
            return Token(TokenType::ASSIGN, currentLine, currentColumn, filename);

        case '!':
            advance();
            if (match('=')) {
                return Token(TokenType::NEQ, currentLine, currentColumn, filename);
            }
            return Token(TokenType::NOT, currentLine, currentColumn, filename);

        case '<':
            advance();
            if (match('=')) {
                return Token(TokenType::LE, currentLine, currentColumn, filename);
            }
            return Token(TokenType::LT, currentLine, currentColumn, filename);

        case '>':
            advance();
            if (match('=')) {
                return Token(TokenType::GE, currentLine, currentColumn, filename);
            }
            return Token(TokenType::GT, currentLine, currentColumn, filename);

        case '+': advance(); return Token(TokenType::PLUS, currentLine, currentColumn, filename);
        case '-':
            advance();
            if (match('>')) {
                return Token(TokenType::ARROW, currentLine, currentColumn, filename);
            }
            return Token(TokenType::MINUS, currentLine, currentColumn, filename);

        case '*': advance(); return Token(TokenType::MULT, currentLine, currentColumn, filename);
        case '/': advance(); return Token(TokenType::DIV, currentLine, currentColumn, filename);
        case '%': advance(); return Token(TokenType::MOD, currentLine, currentColumn, filename);

        case '&':
            advance();
            if (match('&')) {
                return Token(TokenType::AND, currentLine, currentColumn, filename);
            }
            return Token(
                TokenType::ERROR, "Unexpected character '&'", currentLine, currentColumn, filename);

        case '|':
            advance();
            if (match('|')) {
                return Token(TokenType::OR, currentLine, currentColumn, filename);
            }
            return Token(
                TokenType::ERROR, "Unexpected character '|'", currentLine, currentColumn, filename);

        case ':': advance(); return Token(TokenType::COLON, currentLine, currentColumn, filename);
        case ',': advance(); return Token(TokenType::COMMA, currentLine, currentColumn, filename);
        case '.': advance(); return Token(TokenType::DOT, currentLine, currentColumn, filename);

        case '(': advance(); return Token(TokenType::LPAREN, currentLine, currentColumn, filename);
        case ')': advance(); return Token(TokenType::RPAREN, currentLine, currentColumn, filename);
        case '{': advance(); return Token(TokenType::LBRACE, currentLine, currentColumn, filename);
        case '}': advance(); return Token(TokenType::RBRACE, currentLine, currentColumn, filename);
        case '[':
            advance();
            return Token(TokenType::LBRACKET, currentLine, currentColumn, filename);
        case ']':
            advance();
            return Token(TokenType::RBRACKET, currentLine, currentColumn, filename);

        default:
            advance();
            return Token(TokenType::ERROR,
                         std::string("Unexpected character '") + c + "'",
                         currentLine,
                         currentColumn,
                         filename);
    }
}

std::pair<std::vector<Token>, std::optional<LexerError>> Lexer::tokenize()
{
    std::vector<Token> tokens;

    while (true) {
        Token token = nextToken();
        tokens.push_back(token);

        if (token.type == TokenType::END_OF_FILE) {
            break;
        }
        if (token.type == TokenType::ERROR) {
            LexerError error(
                std::get<std::string>(token.value), token.line, token.column, token.filename);
            return {tokens, error};
        }
    }

    return {tokens, std::nullopt};
}