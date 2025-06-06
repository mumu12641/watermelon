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


std::pair<std::unique_ptr<Expression>, std::optional<Error>> Parser::expression()
{
    return assignment();
}

std::pair<std::unique_ptr<Expression>, std::optional<Error>> Parser::assignment()
{
    auto [expr, exprErr] = logicalOr();
    Location l           = expr->getLocation();

    if (exprErr) return {nullptr, exprErr};

    if (match(TokenType::ASSIGN)) {
        auto [value, valueErr] = assignment();
        if (valueErr) return {nullptr, valueErr};

        if (auto* identExpr = dynamic_cast<IdentifierExpression*>(expr.get())) {
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
    Location l           = expr->getLocation();

    if (exprErr) return {nullptr, exprErr};

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
    Location l           = expr->getLocation();

    if (exprErr) return {nullptr, exprErr};

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
    Location l           = expr->getLocation();

    if (exprErr) return {nullptr, exprErr};

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
    Location l           = expr->getLocation();
    if (exprErr) return {nullptr, exprErr};

    while (match({TokenType::LT, TokenType::LE, TokenType::GT, TokenType::GE})) {
        BinaryExpression::Operator op;

        switch (previous().type) {
            case TokenType::LT: op = BinaryExpression::Operator::LT; break;
            case TokenType::LE: op = BinaryExpression::Operator::LE; break;
            case TokenType::GT: op = BinaryExpression::Operator::GT; break;
            case TokenType::GE: op = BinaryExpression::Operator::GE; break;
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
    Location l           = expr->getLocation();

    if (exprErr) return {nullptr, exprErr};

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
    Location l           = expr->getLocation();

    if (exprErr) return {nullptr, exprErr};

    while (match({TokenType::MULT, TokenType::DIV, TokenType::MOD})) {
        BinaryExpression::Operator op;

        switch (previous().type) {
            case TokenType::MULT: op = BinaryExpression::Operator::MUL; break;
            case TokenType::DIV: op = BinaryExpression::Operator::DIV; break;
            case TokenType::MOD: op = BinaryExpression::Operator::MOD; break;
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
        // Location l  = previous().location;

        auto [right, rightErr] = unary();
        if (rightErr) return {nullptr, rightErr};

        return {std::make_unique<UnaryExpression>(Location(), op, std::move(right)), std::nullopt};
    }

    return call();
}

std::pair<std::unique_ptr<Expression>, std::optional<Error>> Parser::call()
{
    auto [expr, exprErr] = primary();
    Location l           = expr->getLocation();

    if (exprErr) return {nullptr, exprErr};

    while (true) {
        if (match(TokenType::LPAREN)) {
            return finishCall(std::move(expr));
        }
        else if (match(TokenType::DOT)) {
            auto [name, nameErr] =
                consume(TokenType::IDENTIFIER, "Expect property name after '.'.");
            if (nameErr) return {nullptr, nameErr};

            expr = std::make_unique<MemberExpression>(
                l, std::move(expr), std::get<std::string>(name.value));
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
    Location                                 l;

    if (!check(TokenType::RPAREN)) {
        do {
            auto [expr, exprErr] = expression();
            l                    = expr->getLocation();
            if (exprErr) return {nullptr, exprErr};
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
                    l, LiteralExpression::Kind::BOOL, std::get<bool>(previous().value)),
                std::nullopt};
    }

    if (match(TokenType::INT_LITERAL)) {
        return {std::make_unique<LiteralExpression>(
                    l, LiteralExpression::Kind::INT, std::get<int>(previous().value)),
                std::nullopt};
    }
    if (match(TokenType::FLOAT_LITERAL)) {
        return {std::make_unique<LiteralExpression>(
                    l, LiteralExpression::Kind::FLOAT, std::get<float>(previous().value)),
                std::nullopt};
    }
    if (match(TokenType::STRING_LITERAL)) {
        return {std::make_unique<LiteralExpression>(
                    l, LiteralExpression::Kind::STRING, std::get<std::string>(previous().value)),
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
    auto [value, valueErr] = expression();
    if (valueErr) return {nullptr, valueErr};

    return {std::make_unique<ReturnStatement>(value->getLocation(), std::move(value)),
            std::nullopt};
}

std::pair<std::unique_ptr<Statement>, std::optional<Error>> Parser::variableStatment()
{
    VariableStatement::Kind kind;
    Location                l = previous().location;
    if (previous().type == TokenType::VAR) {
        kind = VariableStatement::Kind::VAR;
    }
    else if (previous().type == TokenType::VAL) {
        kind = VariableStatement::Kind::VAL;
    }

    auto [name, nameErr] = consume(TokenType::IDENTIFIER, "Expect variable name.");
    if (nameErr) return {nullptr, nameErr};

    std::unique_ptr<Type> varType = nullptr;
    if (match(TokenType::COLON)) {
        auto [typeVal, typeErr] = type();
        if (typeErr) return {nullptr, typeErr};
        varType = std::move(typeVal);
    }

    std::unique_ptr<Expression> initializer = nullptr;
    if (match(TokenType::ASSIGN)) {
        auto [initExpr, initErr] = expression();
        if (initErr) return {nullptr, initErr};
        initializer = std::move(initExpr);
    }

    return {
        std::make_unique<VariableStatement>(
            l, kind, std::get<std::string>(name.value), std::move(varType), std::move(initializer)),
        std::nullopt};
}

std::pair<std::unique_ptr<Declaration>, std::optional<Error>> Parser::declaration()
{
    if (match(TokenType::FN) || (match(TokenType::OPERATOR) && match(TokenType::FUN))) {
        return functionDeclaration();
    }
    if (match(TokenType::ENUM) && match(TokenType::CLASS)) {
        return enumDeclaration();
    }
    if (match({TokenType::CLASS, TokenType::DATA, TokenType::BASE})) {
        return classDeclaration();
    }

    return {nullptr, createError(peek(), "Expect declaration.")};
}

std::pair<std::unique_ptr<Declaration>, std::optional<Error>> Parser::functionDeclaration()
{
    bool     isOperator  = previous().type == TokenType::OPERATOR;
    Location l           = previous().location;
    auto [name, nameErr] = consume(TokenType::IDENTIFIER, "Expect function name.");
    if (nameErr) return {nullptr, nameErr};

    auto [_, lparenErr] = consume(TokenType::LPAREN, "Expect '(' after function name.");
    if (lparenErr) return {nullptr, lparenErr};

    std::vector<FunctionParameter> parameters;
    if (!check(TokenType::RPAREN)) {
        do {
            auto [paramName, paramErr] = consume(TokenType::IDENTIFIER, "Expect parameter name.");
            if (paramErr) return {nullptr, paramErr};

            std::unique_ptr<Type> paramType = nullptr;
            if (match(TokenType::COLON)) {
                auto [typeVal, typeErr] = type();
                if (typeErr) return {nullptr, typeErr};
                paramType = std::move(typeVal);
            }

            std::unique_ptr<Expression> defaultValue = nullptr;
            if (match(TokenType::ASSIGN)) {
                auto [defaultVal, defaultErr] = expression();
                if (defaultErr) return {nullptr, defaultErr};
                defaultValue = std::move(defaultVal);
            }

            parameters.push_back(FunctionParameter(std::get<std::string>(paramName.value),
                                                   std::move(paramType),
                                                   std::move(defaultValue)));
        } while (match(TokenType::COMMA));
    }

    auto [__, rparenErr] = consume(TokenType::RPAREN, "Expect ')' after parameters.");
    if (rparenErr) return {nullptr, rparenErr};

    std::unique_ptr<Type> returnType = nullptr;
    if (match(TokenType::ARROW)) {
        auto [returnTypeVal, returnTypeErr] = type();
        if (returnTypeErr) return {nullptr, returnTypeErr};
        returnType = std::move(returnTypeVal);
    }

    std::unique_ptr<Statement> body = nullptr;
    if (match(TokenType::ASSIGN)) {
        auto [expr, exprErr] = expression();
        Location l           = expr->getLocation();
        if (exprErr) return {nullptr, exprErr};
        auto returnStmt = std::make_unique<ReturnStatement>(l, std::move(expr));
        std::vector<std::unique_ptr<Statement>> statements;
        statements.push_back(std::move(returnStmt));
        body = std::make_unique<BlockStatement>(l, std::move(statements));
    }
    else if (match(TokenType::LBRACE)) {
        std::vector<std::unique_ptr<Statement>> statements;
        while (!check(TokenType::RBRACE) && !isAtEnd()) {
            auto [stmt, stmtErr] = statement();
            if (stmtErr) return {nullptr, stmtErr};
            statements.push_back(std::move(stmt));
        }

        auto [rbrace, rbraceErr] = consume(TokenType::RBRACE, "Expect '}' after block.");
        if (rbraceErr) return {nullptr, rbraceErr};

        body = std::make_unique<BlockStatement>(rbrace.location, std::move(statements));
    }

    return {std::make_unique<FunctionDeclaration>(l,
                                                  std::get<std::string>(name.value),
                                                  std::move(parameters),
                                                  std::move(returnType),
                                                  std::move(body),
                                                  isOperator),
            std::nullopt};
}

std::pair<std::unique_ptr<Declaration>, std::optional<Error>> Parser::enumDeclaration()
{
    auto [name, nameErr] = consume(TokenType::IDENTIFIER, "Expect enum name.");
    if (nameErr) return {nullptr, nameErr};

    auto [_, lbraceErr] = consume(TokenType::LBRACE, "Expect '{' before enum values.");
    if (lbraceErr) return {nullptr, lbraceErr};

    std::vector<std::string> values;

    if (!check(TokenType::RBRACE)) {
        do {
            auto [value, valueErr] = consume(TokenType::IDENTIFIER, "Expect enum value name.");
            if (valueErr) return {nullptr, valueErr};

            values.push_back(std::get<std::string>(value.value));
        } while (match(TokenType::COMMA));
    }

    auto [__, rbraceErr] = consume(TokenType::RBRACE, "Expect '}' after enum values.");
    if (rbraceErr) return {nullptr, rbraceErr};

    return {std::make_unique<EnumDeclaration>(
                name.location, std::get<std::string>(name.value), std::move(values)),
            std::nullopt};
}

std::pair<std::unique_ptr<Declaration>, std::optional<Error>> Parser::classDeclaration()
{
    ClassDeclaration::Kind kind;
    Location               l = previous().location;
    if (previous().type == TokenType::DATA) {
        kind               = ClassDeclaration::Kind::DATA;
        auto [_, classErr] = consume(TokenType::CLASS, "Expect class");
        if (classErr) return {nullptr, classErr};
    }
    else if (previous().type == TokenType::BASE) {
        kind               = ClassDeclaration::Kind::BASE;
        auto [_, classErr] = consume(TokenType::CLASS, "Expect class");
        if (classErr) return {nullptr, classErr};
    }
    else {
        kind = ClassDeclaration::Kind::NORMAL;
    }

    auto [name, nameErr] = consume(TokenType::IDENTIFIER, "Expect class name.");
    if (nameErr) return {nullptr, nameErr};


    auto [__, lparenErr] = consume(TokenType::LPAREN, "Expect '(' after class name.");
    if (lparenErr) return {nullptr, lparenErr};

    std::vector<FunctionParameter> constructorParameters;
    if (!check(TokenType::RPAREN)) {
        do {
            auto [paramName, paramErr] = consume(TokenType::IDENTIFIER, "Expect parameter name.");
            if (paramErr) return {nullptr, paramErr};

            std::unique_ptr<Type> paramType = nullptr;
            if (match(TokenType::COLON)) {
                auto [typeVal, typeErr] = type();
                if (typeErr) return {nullptr, typeErr};
                paramType = std::move(typeVal);
            }

            std::unique_ptr<Expression> defaultValue = nullptr;
            if (match(TokenType::ASSIGN)) {
                auto [defaultVal, defaultErr] = expression();
                if (defaultErr) return {nullptr, defaultErr};
                defaultValue = std::move(defaultVal);
            }

            constructorParameters.push_back(
                FunctionParameter(std::get<std::string>(paramName.value),
                                  std::move(paramType),
                                  std::move(defaultValue)));
        } while (match(TokenType::COMMA));
    }

    auto [___, rparenErr] = consume(TokenType::RPAREN, "Expect ')' after constructor parameters.");
    if (rparenErr) return {nullptr, rparenErr};

    std::string                              baseClass;
    std::vector<std::unique_ptr<Expression>> baseConstructorArgs;

    if (match(TokenType::INHERITS)) {
        auto [baseClassName, baseClassErr] =
            consume(TokenType::IDENTIFIER, "Expect base class name.");
        if (baseClassErr) return {nullptr, baseClassErr};

        baseClass = std::get<std::string>(baseClassName.value);

        auto [_____, lparenErr] = consume(TokenType::LPAREN, "Expect '(' after base class name.");
        if (lparenErr) return {nullptr, lparenErr};

        if (!check(TokenType::RPAREN)) {
            do {
                auto [expr, exprErr] = expression();
                if (exprErr) return {nullptr, exprErr};
                baseConstructorArgs.push_back(std::move(expr));
            } while (match(TokenType::COMMA));
        }

        auto [______, rparenErr] =
            consume(TokenType::RPAREN, "Expect ')' after base constructor arguments.");
        if (rparenErr) return {nullptr, rparenErr};
    }

    auto [_______, lbraceErr] = consume(TokenType::LBRACE, "Expect '{' before class body.");
    if (lbraceErr) return {nullptr, lbraceErr};

    std::vector<std::unique_ptr<ClassMember>> members;
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        auto [member, memberErr] = classMember();
        if (memberErr) return {nullptr, memberErr};

        if (member) {
            members.push_back(std::move(member));
        }
    }

    auto [________, rbraceErr] = consume(TokenType::RBRACE, "Expect '}' after class body.");
    if (rbraceErr) return {nullptr, rbraceErr};

    return {std::make_unique<ClassDeclaration>(l,
                                               kind,
                                               std::get<std::string>(name.value),
                                               std::move(constructorParameters),
                                               std::move(baseClass),
                                               std::move(baseConstructorArgs),
                                               std::move(members)),
            std::nullopt};
}

std::pair<std::unique_ptr<ClassMember>, std::optional<Error>> Parser::classMember()
{
    if (match({TokenType::VAR, TokenType::VAL})) {
        auto [name, nameErr] = consume(TokenType::IDENTIFIER, "Expect property name.");
        if (nameErr) return {nullptr, nameErr};

        std::unique_ptr<Type> propType = nullptr;
        if (match(TokenType::COLON)) {
            auto [typeVal, typeErr] = type();
            if (typeErr) return {nullptr, typeErr};
            propType = std::move(typeVal);
        }

        std::unique_ptr<Expression> initializer = nullptr;
        if (match(TokenType::ASSIGN)) {
            auto [initExpr, initErr] = expression();
            if (initErr) return {nullptr, initErr};
            initializer = std::move(initExpr);
        }

        return {std::make_unique<PropertyMember>(name.location,
                                                 std::get<std::string>(name.value),
                                                 std::move(propType),
                                                 std::move(initializer)),
                std::nullopt};
    }
    else if (match(TokenType::INIT)) {
        auto [initToken, lbraceErr] = consume(TokenType::LBRACE, "Expect '{' after 'init'.");
        if (lbraceErr) return {nullptr, lbraceErr};

        std::vector<std::unique_ptr<Statement>> statements;
        while (!check(TokenType::RBRACE) && !isAtEnd()) {
            auto [stmt, stmtErr] = statement();
            if (stmtErr) return {nullptr, stmtErr};

            statements.push_back(std::move(stmt));
        }

        auto [__, rbraceErr] = consume(TokenType::RBRACE, "Expect '}' after init block.");
        if (rbraceErr) return {nullptr, rbraceErr};

        return {std::make_unique<InitBlockMember>(
                    initToken.location,
                    std::make_unique<BlockStatement>(Location(), std::move(statements))),
                std::nullopt};
    }
    else if (match(TokenType::FN) || (match(TokenType::OPERATOR) && match(TokenType::FUN))) {
        auto [functionDecl, funcErr] = functionDeclaration();
        Location l                   = functionDecl->getLocation();
        if (funcErr) return {nullptr, funcErr};

        return {std::make_unique<MethodMember>(
                    l,
                    std::unique_ptr<FunctionDeclaration>(
                        static_cast<FunctionDeclaration*>(functionDecl.release()))),
                std::nullopt};
    }

    return {nullptr, createError(peek(), "Expect class member.")};
}

std::pair<std::unique_ptr<Type>, std::optional<Error>> Parser::type()
{
    if (match(TokenType::VOID)) {
        return {std::make_unique<PrimitiveType>(PrimitiveType::Kind::VOID), std::nullopt};
    }
    else if (match(TokenType::INT_TYPE)) {
        return {std::make_unique<PrimitiveType>(PrimitiveType::Kind::INT), std::nullopt};
    }
    else if (match(TokenType::FLOAT_TYPE)) {
        return {std::make_unique<PrimitiveType>(PrimitiveType::Kind::FLOAT), std::nullopt};
    }
    else if (match(TokenType::BOOL_TYPE)) {
        return {std::make_unique<PrimitiveType>(PrimitiveType::Kind::BOOL), std::nullopt};
    }
    else if (match(TokenType::STRING_TYPE)) {
        return {std::make_unique<PrimitiveType>(PrimitiveType::Kind::STRING), std::nullopt};
    }
    else if (match(TokenType::IDENTIFIER)) {
        std::string typeName = std::get<std::string>(previous().value);

        return {std::make_unique<CustomType>(std::move(typeName)), std::nullopt};
    }

    return {nullptr, createError(peek(), "Expect type.")};
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