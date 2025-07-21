#include "../../include/parser/parser.hpp"

#include <iostream>

std::pair<std::unique_ptr<Statement>, std::optional<Error>> Parser::statement()
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
    if (match({TokenType::VAL, TokenType::VAR})) {
        return variableStatment();
    }

    return expressionStatement();
}

std::pair<std::unique_ptr<Statement>, std::optional<Error>> Parser::expressionStatement()
{
    auto [expr, exprErr] = expression();
    if (exprErr) return {nullptr, exprErr};

    auto [_, semicolonErr] = consume(TokenType::SEMICOLON, "Expect ';' after expression.");
    if (semicolonErr) return {nullptr, semicolonErr};

    return {std::make_unique<ExpressionStatement>(expr->getLocation(), std::move(expr)),
            std::nullopt};
}

std::pair<std::unique_ptr<Statement>, std::optional<Error>> Parser::blockStatement()
{
    std::vector<std::unique_ptr<Statement>> statements;
    Location                                l;

    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        auto [stmt, stmtErr] = statement();
        l                    = stmt->getLocation();
        if (stmtErr) return {nullptr, stmtErr};
        statements.push_back(std::move(stmt));
    }

    auto [_, rbraceErr] = consume(TokenType::RBRACE, "Expect '}' after block.");
    if (rbraceErr) return {nullptr, rbraceErr};

    return {std::make_unique<BlockStatement>(l, std::move(statements)), std::nullopt};
}

std::pair<std::unique_ptr<Statement>, std::optional<Error>> Parser::ifStatement()
{
    auto [ifToken, lparenErr] = consume(TokenType::LPAREN, "Expect '(' after 'if'.");
    if (lparenErr) return {nullptr, lparenErr};

    auto [condition, conditionErr] = expression();
    if (conditionErr) return {nullptr, conditionErr};

    auto [__, rparenErr] = consume(TokenType::RPAREN, "Expect ')' after if condition.");
    if (rparenErr) return {nullptr, rparenErr};

    auto [thenBranch, thenErr] = statement();
    if (thenErr) return {nullptr, thenErr};

    std::unique_ptr<Statement> elseBranch = nullptr;

    if (match(TokenType::ELSE)) {
        auto [elseStmt, elseErr] = statement();
        if (elseErr) return {nullptr, elseErr};
        elseBranch = std::move(elseStmt);
    }

    return {
        std::make_unique<IfStatement>(
            ifToken.location, std::move(condition), std::move(thenBranch), std::move(elseBranch)),
        std::nullopt};
}

std::pair<std::unique_ptr<Statement>, std::optional<Error>> Parser::whenStatement()
{
    auto [whenToken, lparenErr] = consume(TokenType::LPAREN, "Expect '(' after 'when'.");
    if (lparenErr) return {nullptr, lparenErr};

    auto [subject, subjectErr] = expression();
    if (subjectErr) return {nullptr, subjectErr};

    auto [__, rparenErr] = consume(TokenType::RPAREN, "Expect ')' after when subject.");
    if (rparenErr) return {nullptr, rparenErr};

    auto [___, lbraceErr] = consume(TokenType::LBRACE, "Expect '{' before when cases.");
    if (lbraceErr) return {nullptr, lbraceErr};

    std::vector<WhenStatement::Case> cases;

    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        auto [value, valueErr] = expression();
        if (valueErr) return {nullptr, valueErr};

        auto [____, arrowErr] = consume(TokenType::ARROW, "Expect '->' after case value.");
        if (arrowErr) return {nullptr, arrowErr};

        auto [body, bodyErr] = statement();
        if (bodyErr) return {nullptr, bodyErr};

        cases.push_back({std::move(value), std::move(body)});
    }

    auto [_____, rbraceErr] = consume(TokenType::RBRACE, "Expect '}' after when cases.");
    if (rbraceErr) return {nullptr, rbraceErr};

    return {
        std::make_unique<WhenStatement>(whenToken.location, std::move(subject), std::move(cases)),
        std::nullopt};
}

std::pair<std::unique_ptr<Statement>, std::optional<Error>> Parser::forStatement()
{
    auto [forToken, lparenErr] = consume(TokenType::LPAREN, "Expect '(' after 'for'.");
    if (lparenErr) return {nullptr, lparenErr};

    std::string variable;
    if (match(TokenType::IDENTIFIER)) {
        variable = std::get<std::string>(previous().value);
    }
    else {
        return {nullptr, createError(peek(), "Expect variable name in for loop.")};
    }

    auto [__, inErr] = consume(TokenType::IN, "Expect 'in' after for loop variable.");
    if (inErr) return {nullptr, inErr};

    auto [iterable, iterableErr] = expression();
    if (iterableErr) return {nullptr, iterableErr};

    auto [___, rparenErr] = consume(TokenType::RPAREN, "Expect ')' after for loop condition.");
    if (rparenErr) return {nullptr, rparenErr};

    auto [body, bodyErr] = statement();
    if (bodyErr) return {nullptr, bodyErr};

    return {std::make_unique<ForStatement>(
                forToken.location, std::move(variable), std::move(iterable), std::move(body)),
            std::nullopt};
}

std::pair<std::unique_ptr<Statement>, std::optional<Error>> Parser::returnStatement()
{
    Token                       returnToken = previous();
    std::unique_ptr<Expression> value       = nullptr;

    if (!check(TokenType::SEMICOLON)) {
        auto [expr, exprErr] = expression();
        if (exprErr) return {nullptr, exprErr};
        value = std::move(expr);
    }

    auto [_, semicolonErr] = consume(TokenType::SEMICOLON, "Expect ';' after return statement.");
    if (semicolonErr) return {nullptr, semicolonErr};

    return {std::make_unique<ReturnStatement>(returnToken.location, std::move(value)),
            std::nullopt};
}

std::pair<std::unique_ptr<Statement>, std::optional<Error>> Parser::variableStatment()
{
    bool     immutable   = previous().type == TokenType::VAL;
    Location l           = previous().location;
    auto [name, nameErr] = consume(TokenType::IDENTIFIER, "Expect variable name.");
    if (nameErr) return {nullptr, nameErr};

    std::unique_ptr<Type> declType = nullptr;
    if (match(TokenType::COLON)) {
        auto [typeVal, typeErr] = type();
        if (typeErr) return {nullptr, typeErr};
        declType = std::move(typeVal);
    }

    std::unique_ptr<Expression> initializer = nullptr;
    if (match(TokenType::ASSIGN)) {
        auto [initExpr, initErr] = expression();
        if (initErr) return {nullptr, initErr};
        initializer = std::move(initExpr);
    }
    if (declType == nullptr && initializer == nullptr) {
        return {nullptr,
                createError(
                    previous(),
                    "Variable declaration must have either a type annotation or an initializer.")};
    }
    auto [_, semicolonErr] =
        consume(TokenType::SEMICOLON, "Expect ';' after variable declaration.");
    if (semicolonErr) return {nullptr, semicolonErr};
    return {std::make_unique<VariableStatement>(l,
                                                immutable,
                                                std::get<std::string>(name.value),
                                                std::move(declType),
                                                nullptr,
                                                std::move(initializer)),
            std::nullopt};
}