#include "../../include/parser/parser.hpp"

#include <iostream>

Token Parser::peek() const
{
    return tokens[current];
}

Token Parser::previous() const
{
    return tokens[current - 1];
}

bool Parser::isAtEnd() const
{
    return peek().type == TokenType::END_OF_FILE;
}

Token Parser::advance()
{
    if (!isAtEnd()) {
        current++;
    }
    return previous();
}

bool Parser::check(TokenType type) const
{
    if (isAtEnd()) {
        return false;
    }
    return peek().type == type;
}

bool Parser::match(TokenType type)
{
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::match(std::initializer_list<TokenType> types)
{
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

std::pair<Token, std::optional<Error>> Parser::consume(TokenType type, const std::string& message)
{
    if (check(type)) {
        return {advance(), std::nullopt};
    }
    return {peek(), createError(peek(), message)};
}

Error Parser::createError(const Token& token, const std::string& message)
{
    return Error(message, token.location);
}

std::pair<std::unique_ptr<Program>, std::optional<Error>> Parser::parse()
{
    current = 0;

    std::vector<std::unique_ptr<Declaration>> declarations;

    while (!isAtEnd()) {
        auto [decl, declErr] = declaration();

        if (declErr) {
            return {nullptr, declErr};
        }

        if (decl) {
            declarations.push_back(std::move(decl));
        }
        else {
            if (isAtEnd()) break;
            return {nullptr, createError(peek(), "Expect declaration.")};
        }
    }

    return {std::make_unique<Program>(std::move(declarations)), std::nullopt};
}