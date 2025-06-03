#ifndef LEXER_HPP
#define LEXER_HPP

#include "token.hpp"
#include "../utils/error.hpp"

#include <fstream>
#include <map>
#include <optional>
#include <string>
#include <vector>

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
    std::pair<std::vector<Token>, std::optional<Error>> tokenize();
};


#endif   // LEXER_HPP