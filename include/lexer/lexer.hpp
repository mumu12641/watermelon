#ifndef LEXER_HPP
#define LEXER_HPP

#include "token.hpp"

#include <fstream>
#include <map>
#include <string>
#include <vector>

class Lexer
{
private:
    std::string source;
    size_t      position = 0;
    int         line     = 1;
    int         column   = 1;

    static const std::map<std::string, TokenType> keywords;

    char peek() const;
    char advance();
    bool match(char expected);
    void skipWhitespace();
    void skipComment();

    Token scanIdentifier();
    Token scanNumber();
    Token scanString();

public:
    explicit Lexer(const std::string& source)
        : source(source)
    {}
    explicit Lexer(std::ifstream& file);

    Token              nextToken();
    std::vector<Token> tokenize();
};

#endif   // LEXER_HPP