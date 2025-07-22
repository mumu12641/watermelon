#include "../../include/parser/parser.hpp"

#include <iostream>

std::pair<std::unique_ptr<Expression>, std::optional<Error>> Parser::expression()
{
    return assignment();
}

std::pair<std::unique_ptr<Expression>, std::optional<Error>> Parser::assignment()
{
    auto [expr, exprErr] = logicalOr();
    if (exprErr) return {nullptr, exprErr};

    Location l = expr->getLocation();


    if (match(TokenType::ASSIGN)) {
        auto [value, valueErr] = assignment();
        if (valueErr) return {nullptr, valueErr};

        if (auto* identExpr = dynamic_cast<IdentifierExpression*>(expr.get())) {
            return {std::make_unique<BinaryExpression>(
                        l, BinaryExpression::Operator::ASSIGN, std::move(expr), std::move(value)),
                    std::nullopt};
        }
        else if (auto* memberExpr = dynamic_cast<MemberExpression*>(expr.get())) {
            if (memberExpr->kind == MemberExpression::Kind::METHOD) {
                return {nullptr, createError(previous(), "Cannot assign method.")};
            }
            return {std::make_unique<BinaryExpression>(
                        l, BinaryExpression::Operator::ASSIGN, std::move(expr), std::move(value)),
                    std::nullopt};
        }

        return {nullptr, createError(previous(), "Invalid assignment target.")};
    }
    return {std::move(expr), std::nullopt};
}

std::pair<std::unique_ptr<Expression>, std::optional<Error>> Parser::logicalOr()
{
    auto [expr, exprErr] = logicalAnd();
    if (exprErr) return {nullptr, exprErr};

    Location l = expr->getLocation();

    while (match(TokenType::OR)) {
        auto op                = BinaryExpression::Operator::OR;
        auto [right, rightErr] = logicalAnd();
        if (rightErr) return {nullptr, rightErr};

        expr = std::make_unique<BinaryExpression>(l, op, std::move(expr), std::move(right));
    }

    return {std::move(expr), std::nullopt};
}

std::pair<std::unique_ptr<Expression>, std::optional<Error>> Parser::logicalAnd()
{
    auto [expr, exprErr] = equality();
    if (exprErr) return {nullptr, exprErr};

    Location l = expr->getLocation();

    while (match(TokenType::AND)) {
        auto op                = BinaryExpression::Operator::AND;
        auto [right, rightErr] = equality();
        if (rightErr) return {nullptr, rightErr};

        expr = std::make_unique<BinaryExpression>(l, op, std::move(expr), std::move(right));
    }

    return {std::move(expr), std::nullopt};
}

std::pair<std::unique_ptr<Expression>, std::optional<Error>> Parser::equality()
{
    auto [expr, exprErr] = comparison();
    if (exprErr) return {nullptr, exprErr};

    Location l = expr->getLocation();

    while (match({TokenType::EQ, TokenType::NEQ})) {
        auto op                = previous().type == TokenType::EQ ? BinaryExpression::Operator::EQ
                                                                  : BinaryExpression::Operator::NEQ;
        auto [right, rightErr] = comparison();
        if (rightErr) return {nullptr, rightErr};

        expr = std::make_unique<BinaryExpression>(l, op, std::move(expr), std::move(right));
    }

    return {std::move(expr), std::nullopt};
}

std::pair<std::unique_ptr<Expression>, std::optional<Error>> Parser::comparison()
{
    auto [expr, exprErr] = addition();
    if (exprErr) return {nullptr, exprErr};

    Location l = expr->getLocation();

    while (match({TokenType::LT, TokenType::LE, TokenType::GT, TokenType::GE})) {
        BinaryExpression::Operator op;

        switch (previous().type) {
            case TokenType::LT: op = BinaryExpression::Operator::LT; break;
            case TokenType::LE: op = BinaryExpression::Operator::LE; break;
            case TokenType::GT: op = BinaryExpression::Operator::GT; break;
            case TokenType::GE: op = BinaryExpression::Operator::GE; break;
            default: break;
        }

        auto [right, rightErr] = addition();
        if (rightErr) return {nullptr, rightErr};

        expr = std::make_unique<BinaryExpression>(l, op, std::move(expr), std::move(right));
    }

    return {std::move(expr), std::nullopt};
}

std::pair<std::unique_ptr<Expression>, std::optional<Error>> Parser::addition()
{
    auto [expr, exprErr] = multiplication();
    if (exprErr) return {nullptr, exprErr};

    Location l = expr->getLocation();

    while (match({TokenType::PLUS, TokenType::MINUS})) {
        auto op = previous().type == TokenType::PLUS ? BinaryExpression::Operator::ADD
                                                     : BinaryExpression::Operator::SUB;

        auto [right, rightErr] = multiplication();
        if (rightErr) return {nullptr, rightErr};

        expr = std::make_unique<BinaryExpression>(l, op, std::move(expr), std::move(right));
    }

    return {std::move(expr), std::nullopt};
}

std::pair<std::unique_ptr<Expression>, std::optional<Error>> Parser::multiplication()
{
    auto [expr, exprErr] = unary();
    if (exprErr) return {nullptr, exprErr};

    Location l = expr->getLocation();

    while (match({TokenType::MULT, TokenType::DIV, TokenType::MOD})) {
        BinaryExpression::Operator op;

        switch (previous().type) {
            case TokenType::MULT: op = BinaryExpression::Operator::MUL; break;
            case TokenType::DIV: op = BinaryExpression::Operator::DIV; break;
            case TokenType::MOD: op = BinaryExpression::Operator::MOD; break;
            default: break;
        }

        auto [right, rightErr] = unary();
        if (rightErr) return {nullptr, rightErr};

        expr = std::make_unique<BinaryExpression>(l, op, std::move(expr), std::move(right));
    }

    return {std::move(expr), std::nullopt};
}

std::pair<std::unique_ptr<Expression>, std::optional<Error>> Parser::unary()
{
    if (match({TokenType::MINUS, TokenType::NOT})) {
        auto op = previous().type == TokenType::MINUS ? UnaryExpression::Operator::NEG
                                                      : UnaryExpression::Operator::NOT;

        auto [right, rightErr] = unary();
        if (rightErr) return {nullptr, rightErr};

        return {std::make_unique<UnaryExpression>(right->getLocation(), op, std::move(right)),
                std::nullopt};
    }

    return call();
}

std::pair<std::unique_ptr<Expression>, std::optional<Error>> Parser::call()
{
    auto [expr, exprErr] = primary();
    if (exprErr) return {nullptr, exprErr};

    Location l = expr->getLocation();

    while (true) {
        if (match(TokenType::LPAREN)) {
            return finishCall(std::move(expr));
        }
        else if (match(TokenType::DOT)) {
            auto [name, nameErr] =
                consume(TokenType::IDENTIFIER, "Expect property/method name after '.'.");
            if (nameErr) return {nullptr, nameErr};
            if (match(TokenType::LPAREN)) {
                std::vector<std::unique_ptr<Expression>> arguments;
                if (!check(TokenType::RPAREN)) {
                    do {
                        auto [expr, exprErr] = expression();
                        if (exprErr) return {nullptr, exprErr};

                        arguments.push_back(std::move(expr));
                    } while (match(TokenType::COMMA));
                }
                auto [_, rparenErr] = consume(TokenType::RPAREN, "Expect ')' after arguments.");
                if (rparenErr) return {nullptr, rparenErr};
                expr = std::make_unique<MemberExpression>(
                    l, std::move(expr), std::get<std::string>(name.value), std::move(arguments));
            }
            else {
                expr = std::make_unique<MemberExpression>(
                    l, std::move(expr), std::get<std::string>(name.value));
            }
        }
        else {
            break;
        }
    }

    return {std::move(expr), std::nullopt};
}

std::pair<std::unique_ptr<Expression>, std::optional<Error>> Parser::finishCall(
    std::unique_ptr<Expression> callee)
{
    std::vector<std::unique_ptr<Expression>> arguments;
    Location                                 l = callee->getLocation();

    if (!check(TokenType::RPAREN)) {
        do {
            auto [expr, exprErr] = expression();
            if (exprErr) return {nullptr, exprErr};

            l = expr->getLocation();
            arguments.push_back(std::move(expr));
        } while (match(TokenType::COMMA));
    }

    auto [_, rparenErr] = consume(TokenType::RPAREN, "Expect ')' after arguments.");
    if (rparenErr) return {nullptr, rparenErr};


    return {std::make_unique<CallExpression>(l, std::move(callee), std::move(arguments)),
            std::nullopt};
}

std::pair<std::unique_ptr<Expression>, std::optional<Error>> Parser::primary()
{
    Location l = peek().location;
    if (match(TokenType::BOOL_LITERAL)) {
        return {std::make_unique<LiteralExpression>(
                    l, Type::builtinBool(), std::get<bool>(previous().value)),
                std::nullopt};
    }

    if (match(TokenType::INT_LITERAL)) {
        return {std::make_unique<LiteralExpression>(
                    l, Type::builtinInt(), std::get<int>(previous().value)),
                std::nullopt};
    }
    if (match(TokenType::FLOAT_LITERAL)) {
        return {std::make_unique<LiteralExpression>(
                    l, Type::builtinFloat(), std::get<float>(previous().value)),
                std::nullopt};
    }
    if (match(TokenType::STRING_LITERAL)) {
        return {std::make_unique<LiteralExpression>(
                    l, Type::builtinString(), std::get<std::string>(previous().value)),
                std::nullopt};
    }

    if (match(TokenType::IDENTIFIER)) {
        return {std::make_unique<IdentifierExpression>(l, std::get<std::string>(previous().value)),
                std::nullopt};
    }

    if (match(TokenType::SELF)) {
        return {std::make_unique<IdentifierExpression>(l, "self"), std::nullopt};
    }

    if (match(TokenType::LPAREN)) {
        auto [expr, exprErr] = expression();
        if (exprErr) return {nullptr, exprErr};

        auto [_, rparenErr] = consume(TokenType::RPAREN, "Expect ')' after expression.");
        if (rparenErr) return {nullptr, rparenErr};

        return {std::move(expr), std::nullopt};
    }

    if (match(TokenType::LBRACKET)) {
        std::vector<std::unique_ptr<Expression>> elements;

        if (!check(TokenType::RBRACKET)) {
            do {
                auto [expr, exprErr] = expression();
                if (exprErr) return {nullptr, exprErr};
                elements.push_back(std::move(expr));
            } while (match(TokenType::COMMA));
        }

        auto [_, rbracketErr] = consume(TokenType::RBRACKET, "Expect ']' after array elements.");
        if (rbracketErr) return {nullptr, rbracketErr};

        return {std::make_unique<ArrayExpression>(Location(), std::move(elements)), std::nullopt};
    }

    if (match(TokenType::LBRACE)) {
        auto [body, bodyErr] = expression();
        if (bodyErr) return {nullptr, bodyErr};

        auto [_, rbraceErr] = consume(TokenType::RBRACE, "Expect '}' after lambda body.");
        if (rbraceErr) return {nullptr, rbraceErr};

        return {std::make_unique<LambdaExpression>(
                    Location(), std::vector<LambdaExpression::Parameter>(), std::move(body)),
                std::nullopt};
    }
    return {nullptr, createError(peek(), "Expect expression.")};
}
