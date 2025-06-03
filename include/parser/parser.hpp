#ifndef PARSER_HPP
#define PARSER_HPP

#include "../ast/ast.hpp"
#include "../lexer/token.hpp"
#include "../utils/error.hpp"

#include <memory>
#include <stdexcept>
#include <vector>


class Parser
{
private:
    std::vector<Token> tokens;
    size_t             current = 0;

    Token                                       peek() const;
    Token                                       previous() const;
    bool                                        isAtEnd() const;
    Token                                       advance();
    bool                                        check(TokenType type) const;
    bool                                        match(TokenType type);
    bool                                        match(std::initializer_list<TokenType> types);
    std::pair<Token, std::optional<Error>> consume(TokenType type, const std::string& message);

    Error currentError = Error("", 0, 0);
    Error createError(const Token& token, const std::string& message);

    std::pair<std::unique_ptr<Expression>, std::optional<Error>> expression();
    std::pair<std::unique_ptr<Expression>, std::optional<Error>> assignment();
    std::pair<std::unique_ptr<Expression>, std::optional<Error>> logicalOr();
    std::pair<std::unique_ptr<Expression>, std::optional<Error>> logicalAnd();
    std::pair<std::unique_ptr<Expression>, std::optional<Error>> equality();
    std::pair<std::unique_ptr<Expression>, std::optional<Error>> comparison();
    std::pair<std::unique_ptr<Expression>, std::optional<Error>> addition();
    std::pair<std::unique_ptr<Expression>, std::optional<Error>> multiplication();
    std::pair<std::unique_ptr<Expression>, std::optional<Error>> unary();
    std::pair<std::unique_ptr<Expression>, std::optional<Error>> call();
    std::pair<std::unique_ptr<Expression>, std::optional<Error>> primary();
    std::pair<std::unique_ptr<Expression>, std::optional<Error>> finishCall(
        std::unique_ptr<Expression> callee);

    std::pair<std::unique_ptr<Statement>, std::optional<Error>> statement();
    std::pair<std::unique_ptr<Statement>, std::optional<Error>> expressionStatement();
    std::pair<std::unique_ptr<Statement>, std::optional<Error>> blockStatement();
    std::pair<std::unique_ptr<Statement>, std::optional<Error>> ifStatement();
    std::pair<std::unique_ptr<Statement>, std::optional<Error>> whenStatement();
    std::pair<std::unique_ptr<Statement>, std::optional<Error>> forStatement();
    std::pair<std::unique_ptr<Statement>, std::optional<Error>> returnStatement();
    std::pair<std::unique_ptr<Statement>, std::optional<Error>> variableStatment();


    std::pair<std::unique_ptr<Declaration>, std::optional<Error>> declaration();
    std::pair<std::unique_ptr<Declaration>, std::optional<Error>> functionDeclaration();
    std::pair<std::unique_ptr<Declaration>, std::optional<Error>> enumDeclaration();
    std::pair<std::unique_ptr<Declaration>, std::optional<Error>> classDeclaration();

    // 解析类成员
    std::pair<std::unique_ptr<ClassMember>, std::optional<Error>> classMember();

    // 解析类型
    std::pair<std::unique_ptr<Type>, std::optional<Error>> type();

public:
    explicit Parser(std::vector<Token> tokens)
        : tokens(std::move(tokens))
    {
    }

    std::pair<std::unique_ptr<Program>, std::optional<Error>> parse();
};

#endif