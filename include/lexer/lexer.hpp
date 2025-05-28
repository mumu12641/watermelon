#ifndef LEXER_HPP
#define LEXER_HPP

#include "token.hpp"

#include <fstream>
#include <map>
#include <optional>
#include <string>
#include <vector>

struct LexerError
{
    std::string message;
    int         line;
    int         column;

    LexerError(const std::string& msg, int l, int c)
        : message(msg)
        , line(l)
        , column(c)
    {
    }
};

class Lexer
{
private:
    std::string source;
    // std::string filepath;
    size_t position = 0;
    int    line     = 1;
    int    column   = 1;

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
    // , filepath(filepath)
    {
    }
    explicit Lexer(std::ifstream& file);

    Token                                                    nextToken();
    std::pair<std::vector<Token>, std::optional<LexerError>> tokenize();
};


#endif   // LEXER_HPP