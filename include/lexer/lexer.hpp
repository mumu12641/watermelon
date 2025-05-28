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
    std::string filename;

    LexerError(const std::string& msg, int l, int c, const std::string& fname = "")
        : message(msg)
        , line(l)
        , column(c)
        , filename(fname)
    {
    }
};

class Lexer
{
private:
    std::string source;
    std::string filename;
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
    explicit Lexer(const std::string& source, const std::string& filename = "")
        : source(source)
        , filename(filename)
    {
    }
    explicit Lexer(std::ifstream& file, const std::string& filename = "");

    Token                                                    nextToken();
    std::pair<std::vector<Token>, std::optional<LexerError>> tokenize();
};


#endif   // LEXER_HPP