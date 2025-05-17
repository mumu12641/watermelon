#ifndef PARSER_HPP
#define PARSER_HPP

#include "../ast/ast.hpp"
#include "../lexer/token.hpp"

#include <memory>
#include <stdexcept>
#include <vector>

class ParseError : public std::runtime_error
{
public:
    explicit ParseError(const std::string& message)
        : std::runtime_error(message)
    {}
};

class Parser
{
private:
    std::vector<Token> tokens;
    size_t             current = 0;

    Token      peek() const;
    Token      previous() const;
    bool       isAtEnd() const;
    Token      advance();
    bool       check(TokenType type) const;
    bool       match(TokenType type);
    bool       match(std::initializer_list<TokenType> types);
    Token      consume(TokenType type, const std::string& message);
    ParseError error(const Token& token, const std::string& message);
    void       synchronize();

    // 解析表达式
    std::unique_ptr<Expression> expression();
    std::unique_ptr<Expression> assignment();
    std::unique_ptr<Expression> logicalOr();
    std::unique_ptr<Expression> logicalAnd();
    std::unique_ptr<Expression> equality();
    std::unique_ptr<Expression> comparison();
    std::unique_ptr<Expression> addition();
    std::unique_ptr<Expression> multiplication();
    std::unique_ptr<Expression> unary();
    std::unique_ptr<Expression> call();
    std::unique_ptr<Expression> primary();
    std::unique_ptr<Expression> finishCall(std::unique_ptr<Expression> callee);

    // 解析语句
    std::unique_ptr<Statement> statement();
    std::unique_ptr<Statement> expressionStatement();
    std::unique_ptr<Statement> blockStatement();
    std::unique_ptr<Statement> ifStatement();
    std::unique_ptr<Statement> whenStatement();
    std::unique_ptr<Statement> forStatement();
    std::unique_ptr<Statement> returnStatement();
    std::unique_ptr<Statement> variableStatment();


    // 解析声明
    std::unique_ptr<Declaration> declaration();
    std::unique_ptr<Declaration> functionDeclaration();
    std::unique_ptr<Declaration> enumDeclaration();
    std::unique_ptr<Declaration> classDeclaration();

    // 解析类成员
    std::unique_ptr<ClassMember> classMember();

    // 解析类型
    std::unique_ptr<Type> type();

public:
    explicit Parser(const std::vector<Token>& tokens)
        : tokens(tokens)
    {}

    std::unique_ptr<Program> parse();
};

#endif   // PARSER_HPP