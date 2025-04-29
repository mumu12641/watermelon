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

Token Parser::consume(TokenType type, const std::string& message)
{
    if (check(type)) {
        return advance();
    }

    throw error(peek(), message);
}

ParseError Parser::error(const Token& token, const std::string& message)
{
    std::cerr << "Error at ";
    if (token.type == TokenType::END_OF_FILE) {
        std::cerr << "end";
    }
    else {
        std::cerr << "'" << token.toString() << "'";
    }
    std::cerr << ": " << message << std::endl;

    return ParseError(message);
}

void Parser::synchronize()
{
    advance();

    while (!isAtEnd()) {
        // if (previous().type == TokenType::SEMICOLON) {
        //     return;
        // }

        switch (peek().type) {
            case TokenType::CLASS:
            case TokenType::FN:
            case TokenType::VAR:
            case TokenType::FOR:
            case TokenType::IF:
            case TokenType::WHEN:
            case TokenType::RETURN: return;
            default: break;
        }

        advance();
    }
}

std::unique_ptr<Expression> Parser::expression()
{
    return assignment();
}

std::unique_ptr<Expression> Parser::assignment()
{
    auto expr = logicalOr();

    if (match(TokenType::ASSIGN)) {
        auto value = assignment();

        if (auto* identExpr = dynamic_cast<IdentifierExpression*>(expr.get())) {
            return std::make_unique<BinaryExpression>(
                BinaryExpression::Operator::ASSIGN, std::move(expr), std::move(value));
        }

        error(previous(), "Invalid assignment target.");
    }

    return expr;
}

std::unique_ptr<Expression> Parser::logicalOr()
{
    auto expr = logicalAnd();

    while (match(TokenType::OR)) {
        auto op    = BinaryExpression::Operator::OR;
        auto right = logicalAnd();
        expr       = std::make_unique<BinaryExpression>(op, std::move(expr), std::move(right));
    }

    return expr;
}

std::unique_ptr<Expression> Parser::logicalAnd()
{
    auto expr = equality();

    while (match(TokenType::AND)) {
        auto op    = BinaryExpression::Operator::AND;
        auto right = equality();
        expr       = std::make_unique<BinaryExpression>(op, std::move(expr), std::move(right));
    }

    return expr;
}

std::unique_ptr<Expression> Parser::equality()
{
    auto expr = comparison();

    while (match({TokenType::EQ, TokenType::NEQ})) {
        auto op    = previous().type == TokenType::EQ ? BinaryExpression::Operator::EQ
                                                      : BinaryExpression::Operator::NEQ;
        auto right = comparison();
        expr       = std::make_unique<BinaryExpression>(op, std::move(expr), std::move(right));
    }

    return expr;
}

std::unique_ptr<Expression> Parser::comparison()
{
    auto expr = addition();

    while (match({TokenType::LT, TokenType::LE, TokenType::GT, TokenType::GE})) {
        BinaryExpression::Operator op;

        switch (previous().type) {
            case TokenType::LT: op = BinaryExpression::Operator::LT; break;
            case TokenType::LE: op = BinaryExpression::Operator::LE; break;
            case TokenType::GT: op = BinaryExpression::Operator::GT; break;
            case TokenType::GE: op = BinaryExpression::Operator::GE; break;
            default: throw std::runtime_error("Unexpected operator");
        }

        auto right = addition();
        expr       = std::make_unique<BinaryExpression>(op, std::move(expr), std::move(right));
    }

    return expr;
}

std::unique_ptr<Expression> Parser::addition()
{
    auto expr = multiplication();

    while (match({TokenType::PLUS, TokenType::MINUS})) {
        auto op    = previous().type == TokenType::PLUS ? BinaryExpression::Operator::ADD
                                                        : BinaryExpression::Operator::SUB;
        auto right = multiplication();
        expr       = std::make_unique<BinaryExpression>(op, std::move(expr), std::move(right));
    }

    return expr;
}

std::unique_ptr<Expression> Parser::multiplication()
{
    auto expr = unary();

    while (match({TokenType::MULT, TokenType::DIV, TokenType::MOD})) {
        BinaryExpression::Operator op;

        switch (previous().type) {
            case TokenType::MULT: op = BinaryExpression::Operator::MUL; break;
            case TokenType::DIV: op = BinaryExpression::Operator::DIV; break;
            case TokenType::MOD: op = BinaryExpression::Operator::MOD; break;
            default: throw std::runtime_error("Unexpected operator");
        }

        auto right = unary();
        expr       = std::make_unique<BinaryExpression>(op, std::move(expr), std::move(right));
    }

    return expr;
}

std::unique_ptr<Expression> Parser::unary()
{
    if (match({TokenType::MINUS, TokenType::NOT})) {
        auto op    = previous().type == TokenType::MINUS ? UnaryExpression::Operator::NEG
                                                         : UnaryExpression::Operator::NOT;
        auto right = unary();
        return std::make_unique<UnaryExpression>(op, std::move(right));
    }

    return call();
}

std::unique_ptr<Expression> Parser::call()
{
    auto expr = primary();

    while (true) {
        if (match(TokenType::LPAREN)) {
            expr = finishCall(std::move(expr));
        }
        else if (match(TokenType::DOT)) {
            auto name = consume(TokenType::IDENTIFIER, "Expect property name after '.'.");
            expr      = std::make_unique<MemberExpression>(std::move(expr),
                                                      std::get<std::string>(name.value));
        }
        else {
            break;
        }
    }

    return expr;
}

std::unique_ptr<Expression> Parser::finishCall(std::unique_ptr<Expression> callee)
{
    std::vector<std::unique_ptr<Expression>> arguments;

    if (!check(TokenType::RPAREN)) {
        do {
            arguments.push_back(expression());
        } while (match(TokenType::COMMA));
    }

    consume(TokenType::RPAREN, "Expect ')' after arguments.");

    return std::make_unique<CallExpression>(std::move(callee), std::move(arguments));
}

std::unique_ptr<Expression> Parser::primary()
{
    if (match(TokenType::FALSE)) {
        return std::make_unique<LiteralExpression>(LiteralExpression::Kind::BOOL, false);
    }
    if (match(TokenType::TRUE)) {
        return std::make_unique<LiteralExpression>(LiteralExpression::Kind::BOOL, true);
    }

    if (match(TokenType::INT_LITERAL)) {
        return std::make_unique<LiteralExpression>(LiteralExpression::Kind::INT,
                                                   std::get<int>(previous().value));
    }
    if (match(TokenType::FLOAT_LITERAL)) {
        return std::make_unique<LiteralExpression>(LiteralExpression::Kind::FLOAT,
                                                   std::get<float>(previous().value));
    }
    if (match(TokenType::STRING_LITERAL)) {
        return std::make_unique<LiteralExpression>(LiteralExpression::Kind::STRING,
                                                   std::get<std::string>(previous().value));
    }

    if (match(TokenType::IDENTIFIER)) {
        return std::make_unique<IdentifierExpression>(std::get<std::string>(previous().value));
    }

    if (match(TokenType::SELF)) {
        return std::make_unique<IdentifierExpression>("self");
    }

    if (match(TokenType::LPAREN)) {
        auto expr = expression();
        consume(TokenType::RPAREN, "Expect ')' after expression.");
        return expr;
    }

    if (match(TokenType::LBRACKET)) {
        std::vector<std::unique_ptr<Expression>> elements;

        if (!check(TokenType::RBRACKET)) {
            do {
                elements.push_back(expression());
            } while (match(TokenType::COMMA));
        }

        consume(TokenType::RBRACKET, "Expect ']' after array elements.");

        return std::make_unique<ArrayExpression>(std::move(elements));
    }

    if (match(TokenType::LBRACE)) {
        // Lambda表达式
        // 这里简化了实现，实际上需要处理参数列表和箭头
        auto body = expression();
        consume(TokenType::RBRACE, "Expect '}' after lambda body.");

        return std::make_unique<LambdaExpression>(std::vector<LambdaExpression::Parameter>(),
                                                  std::move(body));
    }

    throw error(peek(), "Expect expression.");
}

std::unique_ptr<Statement> Parser::statement()
{
    if (match(TokenType::IF)) {
        return ifStatement();
    }
    if (match(TokenType::WHEN)) {
        return whenStatement();
    }
    if (match(TokenType::FOR)) {
        return forStatement();
    }
    if (match(TokenType::RETURN)) {
        return returnStatement();
    }
    if (match(TokenType::LBRACE)) {
        return blockStatement();
    }

    return expressionStatement();
}

std::unique_ptr<Statement> Parser::expressionStatement()
{
    auto expr = expression();
    consume(TokenType::SEMICOLON, "Expect ';' after expression.");
    return std::make_unique<ExpressionStatement>(std::move(expr));
}

std::unique_ptr<Statement> Parser::blockStatement()
{
    std::vector<std::unique_ptr<Statement>> statements;

    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        statements.push_back(declaration());
    }

    consume(TokenType::RBRACE, "Expect '}' after block.");

    return std::make_unique<BlockStatement>(std::move(statements));
}

std::unique_ptr<Statement> Parser::ifStatement()
{
    consume(TokenType::LPAREN, "Expect '(' after 'if'.");
    auto condition = expression();
    consume(TokenType::RPAREN, "Expect ')' after if condition.");

    auto                       thenBranch = statement();
    std::unique_ptr<Statement> elseBranch = nullptr;

    if (match(TokenType::ELSE)) {
        elseBranch = statement();
    }

    return std::make_unique<IfStatement>(
        std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<Statement> Parser::whenStatement()
{
    consume(TokenType::LPAREN, "Expect '(' after 'when'.");
    auto subject = expression();
    consume(TokenType::RPAREN, "Expect ')' after when subject.");

    consume(TokenType::LBRACE, "Expect '{' before when cases.");

    std::vector<WhenStatement::Case> cases;

    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        auto value = expression();
        consume(TokenType::ARROW, "Expect '->' after case value.");
        auto body = statement();

        cases.push_back({std::move(value), std::move(body)});
    }

    consume(TokenType::RBRACE, "Expect '}' after when cases.");

    return std::make_unique<WhenStatement>(std::move(subject), std::move(cases));
}

std::unique_ptr<Statement> Parser::forStatement()
{
    consume(TokenType::LPAREN, "Expect '(' after 'for'.");

    std::string variable;
    if (match(TokenType::IDENTIFIER)) {
        variable = std::get<std::string>(previous().value);
    }
    else {
        throw error(peek(), "Expect variable name in for loop.");
    }

    consume(TokenType::IN, "Expect 'in' after for loop variable.");
    auto iterable = expression();

    consume(TokenType::RPAREN, "Expect ')' after for loop condition.");

    auto body = statement();

    return std::make_unique<ForStatement>(
        std::move(variable), std::move(iterable), std::move(body));
}

std::unique_ptr<Statement> Parser::returnStatement()
{
    std::unique_ptr<Expression> value = nullptr;

    if (!check(TokenType::SEMICOLON)) {
        value = expression();
    }

    consume(TokenType::SEMICOLON, "Expect ';' after return value.");

    return std::make_unique<ReturnStatement>(std::move(value));
}

std::unique_ptr<Declaration> Parser::declaration()
{
    try {
        if (match(TokenType::VAR) || match(TokenType::VAL) || match(TokenType::LET)) {
            return variableDeclaration();
        }
        if (match(TokenType::FN) || (match(TokenType::OPERATOR) && match(TokenType::FUN))) {
            return functionDeclaration();
        }
        if (match(TokenType::ENUM) && match(TokenType::CLASS)) {
            return enumDeclaration();
        }
        if (match({TokenType::CLASS, TokenType::DATA, TokenType::BASE})) {
            return classDeclaration();
        }

        return statement();
    }
    catch (const ParseError& error) {
        synchronize();
        return nullptr;
    }
}

std::unique_ptr<Declaration> Parser::variableDeclaration()
{
    VariableDeclaration::Kind kind;

    if (previous().type == TokenType::VAR) {
        kind = VariableDeclaration::Kind::VAR;
    }
    else if (previous().type == TokenType::VAL) {
        kind = VariableDeclaration::Kind::VAL;
    }
    else {
        kind = VariableDeclaration::Kind::LET;
    }

    auto name = consume(TokenType::IDENTIFIER, "Expect variable name.").value;

    std::unique_ptr<Type> varType = nullptr;
    if (match(TokenType::COLON)) {
        varType = type();
    }

    std::unique_ptr<Expression> initializer = nullptr;
    if (match(TokenType::ASSIGN)) {
        initializer = expression();
    }

    consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");

    return std::make_unique<VariableDeclaration>(
        kind, std::get<std::string>(name), std::move(varType), std::move(initializer));
}

std::unique_ptr<Declaration> Parser::functionDeclaration()
{
    bool isOperator = previous().type == TokenType::OPERATOR;

    auto name = consume(TokenType::IDENTIFIER, "Expect function name.").value;

    consume(TokenType::LPAREN, "Expect '(' after function name.");

    std::vector<FunctionParameter> parameters;
    if (!check(TokenType::RPAREN)) {
        do {
            auto paramName = consume(TokenType::IDENTIFIER, "Expect parameter name.").value;

            std::unique_ptr<Type> paramType = nullptr;
            if (match(TokenType::COLON)) {
                paramType = type();
            }

            std::unique_ptr<Expression> defaultValue = nullptr;
            if (match(TokenType::ASSIGN)) {
                defaultValue = expression();
            }

            parameters.push_back(FunctionParameter(
                std::get<std::string>(paramName), std::move(paramType), std::move(defaultValue)));
        } while (match(TokenType::COMMA));
    }

    consume(TokenType::RPAREN, "Expect ')' after parameters.");

    std::unique_ptr<Type> returnType = nullptr;
    if (match(TokenType::ARROW)) {
        returnType = type();
    }

    std::unique_ptr<Statement> body = nullptr;
    if (match(TokenType::ASSIGN)) {
        // 单表达式函数体
        auto expr = expression();
        consume(TokenType::SEMICOLON, "Expect ';' after function expression.");

        auto returnStmt = std::make_unique<ReturnStatement>(std::move(expr));
        std::vector<std::unique_ptr<Statement>> statements;
        statements.push_back(std::move(returnStmt));

        body = std::make_unique<BlockStatement>(std::move(statements));
    }
    else if (match(TokenType::LBRACE)) {
        // 块函数体
        std::vector<std::unique_ptr<Statement>> statements;

        while (!check(TokenType::RBRACE) && !isAtEnd()) {
            statements.push_back(declaration());
        }

        consume(TokenType::RBRACE, "Expect '}' after block.");

        body = std::make_unique<BlockStatement>(std::move(statements));
    }

    return std::make_unique<FunctionDeclaration>(std::get<std::string>(name),
                                                 std::move(parameters),
                                                 std::move(returnType),
                                                 std::move(body),
                                                 isOperator);
}

std::unique_ptr<Declaration> Parser::enumDeclaration()
{
    auto name = consume(TokenType::IDENTIFIER, "Expect enum name.").value;

    consume(TokenType::LBRACE, "Expect '{' before enum values.");

    std::vector<std::string> values;

    if (!check(TokenType::RBRACE)) {
        do {
            auto value = consume(TokenType::IDENTIFIER, "Expect enum value name.").value;
            values.push_back(std::get<std::string>(value));
        } while (match(TokenType::COMMA));
    }
