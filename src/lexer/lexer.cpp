#include "../../include/lexer/lexer.hpp"

#include <cctype>
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
                                                          {"print", TokenType::PRINT},
                                                          {"println", TokenType::PRINTLN},
                                                          {"void", TokenType::VOID},
                                                          {"int", TokenType::INT_TYPE},
                                                          {"float", TokenType::FLOAT_TYPE},
                                                          {"bool", TokenType::BOOL_TYPE},
                                                          {"string", TokenType::STRING_TYPE},
                                                          {"String", TokenType::STRING_TYPE},
                                                          {"Array", TokenType::ARRAY_TYPE},
                                                          {"true", TokenType::BOOL_LITERAL},
                                                          {"false", TokenType::BOOL_LITERAL}};

Lexer::Lexer(std::ifstream& file)
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
    // 跳过单行注释
    if (peek() == '/' && position + 1 < source.length() && source[position + 1] == '/') {
        while (peek() != '\n' && peek() != '\0') {
            advance();
        }
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
            return Token(it->second, identifier == "true", startLine, startColumn);
        }
        return Token(it->second, startLine, startColumn);
    }

    return Token(TokenType::IDENTIFIER, identifier, startLine, startColumn);
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

    // 检查是否是浮点数
    if (peek() == '.' && isdigit(source[position + 1])) {
        isFloat = true;
        number += advance();   // 添加小数点

        while (isdigit(peek())) {
            number += advance();
        }
    }

    if (isFloat) {
        return Token(TokenType::FLOAT_LITERAL, std::stof(number), startLine, startColumn);
    }
    else {
        return Token(TokenType::INT_LITERAL, std::stoi(number), startLine, startColumn);
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
        return Token(TokenType::ERROR, "Unterminated string", startLine, startColumn);
    }

    // 跳过结束的引号
    advance();

    return Token(TokenType::STRING_LITERAL, str, startLine, startColumn);
}

Token Lexer::nextToken()
{
    skipWhitespace();
    skipComment();

    int currentLine   = line;
    int currentColumn = column;

    if (position >= source.length()) {
        return Token(TokenType::END_OF_FILE, currentLine, currentColumn);
    }

    char c = peek();

    // 标识符或关键字
    if (isalpha(c) || c == '_') {
        return scanIdentifier();
    }

    // 数字
    if (isdigit(c)) {
        return scanNumber();
    }

    // 字符串
    if (c == '"') {
        return scanString();
    }

    // 运算符和标点符号
    switch (c) {
    case '=':
        advance();
        if (match('=')) {
            return Token(TokenType::EQ, currentLine, currentColumn);
        }
        return Token(TokenType::ASSIGN, currentLine, currentColumn);

    case '!':
        advance();
        if (match('=')) {
            return Token(TokenType::NEQ, currentLine, currentColumn);
        }
        return Token(TokenType::NOT, currentLine, currentColumn);

    case '<':
        advance();
        if (match('=')) {
            return Token(TokenType::LE, currentLine, currentColumn);
        }
        return Token(TokenType::LT, currentLine, currentColumn);

    case '>':
        advance();
        if (match('=')) {
            return Token(TokenType::GE, currentLine, currentColumn);
        }
        return Token(TokenType::GT, currentLine, currentColumn);

    case '+': advance(); return Token(TokenType::PLUS, currentLine, currentColumn);
    case '-':
        advance();
        if (match('>')) {
            return Token(TokenType::ARROW, currentLine, currentColumn);
        }
        return Token(TokenType::MINUS, currentLine, currentColumn);

    case '*': advance(); return Token(TokenType::MULT, currentLine, currentColumn);
    case '/': advance(); return Token(TokenType::DIV, currentLine, currentColumn);
    case '%': advance(); return Token(TokenType::MOD, currentLine, currentColumn);

    case '&':
        advance();
        if (match('&')) {
            return Token(TokenType::AND, currentLine, currentColumn);
        }
        return Token(TokenType::ERROR, "Unexpected character '&'", currentLine, currentColumn);

    case '|':
        advance();
        if (match('|')) {
            return Token(TokenType::OR, currentLine, currentColumn);
        }
        return Token(TokenType::ERROR, "Unexpected character '|'", currentLine, currentColumn);

    case ':': advance(); return Token(TokenType::COLON, currentLine, currentColumn);
    // case ';': advance(); return Token(TokenType::SEMICOLON, currentLine, currentColumn);
    case ',': advance(); return Token(TokenType::COMMA, currentLine, currentColumn);
    case '.': advance(); return Token(TokenType::DOT, currentLine, currentColumn);

    case '(': advance(); return Token(TokenType::LPAREN, currentLine, currentColumn);
    case ')': advance(); return Token(TokenType::RPAREN, currentLine, currentColumn);
    case '{': advance(); return Token(TokenType::LBRACE, currentLine, currentColumn);
    case '}': advance(); return Token(TokenType::RBRACE, currentLine, currentColumn);
    case '[': advance(); return Token(TokenType::LBRACKET, currentLine, currentColumn);
    case ']': advance(); return Token(TokenType::RBRACKET, currentLine, currentColumn);

    default:
        advance();
        return Token(TokenType::ERROR,
                     std::string("Unexpected character '") + c + "'",
                     currentLine,
                     currentColumn);
    }
}

std::vector<Token> Lexer::tokenize()
{
    std::vector<Token> tokens;

    while (true) {
        Token token = nextToken();
        tokens.push_back(token);

        if (token.type == TokenType::END_OF_FILE || token.type == TokenType::ERROR) {
            break;
        }
    }

    return tokens;
}