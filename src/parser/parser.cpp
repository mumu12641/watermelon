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
    hasError     = true;
    currentError = createError(peek(), message);
    return peek();
}

ParseError Parser::createError(const Token& token, const std::string& message)
{
    return ParseError(message, token.line, token.column, token.filename);
}


std::unique_ptr<Expression> Parser::expression()
{
    auto expr = assignment();
    if (hasError) return nullptr;
    return expr;
}

std::unique_ptr<Expression> Parser::assignment()
{
    auto expr = logicalOr();
    if (hasError) return nullptr;

    if (match(TokenType::ASSIGN)) {
        auto value = assignment();
        if (hasError) return nullptr;

        if (auto* identExpr = dynamic_cast<IdentifierExpression*>(expr.get())) {
            return std::make_unique<BinaryExpression>(
                BinaryExpression::Operator::ASSIGN, std::move(expr), std::move(value));
        }

        hasError     = true;
        currentError = createError(previous(), "Invalid assignment target.");
        return nullptr;
    }

    return expr;
}

std::unique_ptr<Expression> Parser::logicalOr()
{
    auto expr = logicalAnd();
    if (hasError) return nullptr;

    while (match(TokenType::OR)) {
        auto op    = BinaryExpression::Operator::OR;
        auto right = logicalAnd();
        if (hasError) return nullptr;

        expr = std::make_unique<BinaryExpression>(op, std::move(expr), std::move(right));
    }

    return expr;
}

std::unique_ptr<Expression> Parser::logicalAnd()
{
    auto expr = equality();
    if (hasError) return nullptr;

    while (match(TokenType::AND)) {
        auto op    = BinaryExpression::Operator::AND;
        auto right = equality();
        if (hasError) return nullptr;

        expr = std::make_unique<BinaryExpression>(op, std::move(expr), std::move(right));
    }

    return expr;
}

std::unique_ptr<Expression> Parser::equality()
{
    auto expr = comparison();
    if (hasError) return nullptr;

    while (match({TokenType::EQ, TokenType::NEQ})) {
        auto op    = previous().type == TokenType::EQ ? BinaryExpression::Operator::EQ
                                                      : BinaryExpression::Operator::NEQ;
        auto right = comparison();
        if (hasError) return nullptr;

        expr = std::make_unique<BinaryExpression>(op, std::move(expr), std::move(right));
    }

    return expr;
}

std::unique_ptr<Expression> Parser::comparison()
{
    auto expr = addition();
    if (hasError) return nullptr;

    while (match({TokenType::LT, TokenType::LE, TokenType::GT, TokenType::GE})) {
        BinaryExpression::Operator op;

        switch (previous().type) {
            case TokenType::LT: op = BinaryExpression::Operator::LT; break;
            case TokenType::LE: op = BinaryExpression::Operator::LE; break;
            case TokenType::GT: op = BinaryExpression::Operator::GT; break;
            case TokenType::GE: op = BinaryExpression::Operator::GE; break;
        }

        auto right = addition();
        if (hasError) return nullptr;

        expr = std::make_unique<BinaryExpression>(op, std::move(expr), std::move(right));
    }

    return expr;
}

std::unique_ptr<Expression> Parser::addition()
{
    auto expr = multiplication();
    if (hasError) return nullptr;

    while (match({TokenType::PLUS, TokenType::MINUS})) {
        auto op    = previous().type == TokenType::PLUS ? BinaryExpression::Operator::ADD
                                                        : BinaryExpression::Operator::SUB;
        auto right = multiplication();
        if (hasError) return nullptr;

        expr = std::make_unique<BinaryExpression>(op, std::move(expr), std::move(right));
    }

    return expr;
}

std::unique_ptr<Expression> Parser::multiplication()
{
    auto expr = unary();
    if (hasError) return nullptr;

    while (match({TokenType::MULT, TokenType::DIV, TokenType::MOD})) {
        BinaryExpression::Operator op;

        switch (previous().type) {
            case TokenType::MULT: op = BinaryExpression::Operator::MUL; break;
            case TokenType::DIV: op = BinaryExpression::Operator::DIV; break;
            case TokenType::MOD: op = BinaryExpression::Operator::MOD; break;
        }

        auto right = unary();
        if (hasError) return nullptr;

        expr = std::make_unique<BinaryExpression>(op, std::move(expr), std::move(right));
    }

    return expr;
}

std::unique_ptr<Expression> Parser::unary()
{
    if (match({TokenType::MINUS, TokenType::NOT})) {
        auto op    = previous().type == TokenType::MINUS ? UnaryExpression::Operator::NEG
                                                         : UnaryExpression::Operator::NOT;
        auto right = unary();
        if (hasError) return nullptr;

        return std::make_unique<UnaryExpression>(op, std::move(right));
    }

    return call();
}

std::unique_ptr<Expression> Parser::call()
{
    auto expr = primary();
    if (hasError) return nullptr;

    while (true) {
        if (match(TokenType::LPAREN)) {
            expr = finishCall(std::move(expr));
            if (hasError) return nullptr;
        }
        else if (match(TokenType::DOT)) {
            auto name = consume(TokenType::IDENTIFIER, "Expect property name after '.'.");
            if (hasError) return nullptr;

            expr = std::make_unique<MemberExpression>(std::move(expr),
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
            if (hasError) return nullptr;
        } while (match(TokenType::COMMA));
    }

    consume(TokenType::RPAREN, "Expect ')' after arguments.");
    if (hasError) return nullptr;

    return std::make_unique<CallExpression>(std::move(callee), std::move(arguments));
}

std::unique_ptr<Expression> Parser::primary()
{
    if (match(TokenType::BOOL_LITERAL)) {
        return std::make_unique<LiteralExpression>(LiteralExpression::Kind::BOOL,
                                                   std::get<bool>(previous().value));
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
        if (hasError) return nullptr;

        consume(TokenType::RPAREN, "Expect ')' after expression.");
        if (hasError) return nullptr;

        return expr;
    }

    if (match(TokenType::LBRACKET)) {
        std::vector<std::unique_ptr<Expression>> elements;

        if (!check(TokenType::RBRACKET)) {
            do {
                elements.push_back(expression());
                if (hasError) return nullptr;
            } while (match(TokenType::COMMA));
        }

        consume(TokenType::RBRACKET, "Expect ']' after array elements.");
        if (hasError) return nullptr;

        return std::make_unique<ArrayExpression>(std::move(elements));
    }

    if (match(TokenType::LBRACE)) {
        auto body = expression();
        if (hasError) return nullptr;

        consume(TokenType::RBRACE, "Expect '}' after lambda body.");
        if (hasError) return nullptr;

        return std::make_unique<LambdaExpression>(std::vector<LambdaExpression::Parameter>(),
                                                  std::move(body));
    }

    hasError     = true;
    currentError = createError(peek(), "Expect expression.");
    return nullptr;
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
    if (match({TokenType::VAL, TokenType::VAR})) {
        return variableStatment();
    }

    return expressionStatement();
}

std::unique_ptr<Statement> Parser::expressionStatement()
{
    auto expr = expression();
    if (hasError) return nullptr;

    return std::make_unique<ExpressionStatement>(std::move(expr));
}

std::unique_ptr<Statement> Parser::blockStatement()
{
    std::vector<std::unique_ptr<Statement>> statements;

    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        statements.push_back(statement());
        if (hasError) return nullptr;
    }

    consume(TokenType::RBRACE, "Expect '}' after block.");
    if (hasError) return nullptr;

    return std::make_unique<BlockStatement>(std::move(statements));
}

std::unique_ptr<Statement> Parser::ifStatement()
{
    consume(TokenType::LPAREN, "Expect '(' after 'if'.");
    if (hasError) return nullptr;

    auto condition = expression();
    if (hasError) return nullptr;

    consume(TokenType::RPAREN, "Expect ')' after if condition.");
    if (hasError) return nullptr;

    auto thenBranch = statement();
    if (hasError) return nullptr;

    std::unique_ptr<Statement> elseBranch = nullptr;

    if (match(TokenType::ELSE)) {
        elseBranch = statement();
        if (hasError) return nullptr;
    }

    return std::make_unique<IfStatement>(
        std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<Statement> Parser::whenStatement()
{
    consume(TokenType::LPAREN, "Expect '(' after 'when'.");
    if (hasError) return nullptr;

    auto subject = expression();
    if (hasError) return nullptr;

    consume(TokenType::RPAREN, "Expect ')' after when subject.");
    if (hasError) return nullptr;

    consume(TokenType::LBRACE, "Expect '{' before when cases.");
    if (hasError) return nullptr;

    std::vector<WhenStatement::Case> cases;

    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        auto value = expression();
        if (hasError) return nullptr;

        consume(TokenType::ARROW, "Expect '->' after case value.");
        if (hasError) return nullptr;

        auto body = statement();
        if (hasError) return nullptr;

        cases.push_back({std::move(value), std::move(body)});
    }

    consume(TokenType::RBRACE, "Expect '}' after when cases.");
    if (hasError) return nullptr;

    return std::make_unique<WhenStatement>(std::move(subject), std::move(cases));
}

std::unique_ptr<Statement> Parser::forStatement()
{
    consume(TokenType::LPAREN, "Expect '(' after 'for'.");
    if (hasError) return nullptr;

    std::string variable;
    if (match(TokenType::IDENTIFIER)) {
        variable = std::get<std::string>(previous().value);
    }
    else {
        hasError     = true;
        currentError = createError(peek(), "Expect variable name in for loop.");
        return nullptr;
    }

    consume(TokenType::IN, "Expect 'in' after for loop variable.");
    if (hasError) return nullptr;

    auto iterable = expression();
    if (hasError) return nullptr;

    consume(TokenType::RPAREN, "Expect ')' after for loop condition.");
    if (hasError) return nullptr;

    auto body = statement();
    if (hasError) return nullptr;

    return std::make_unique<ForStatement>(
        std::move(variable), std::move(iterable), std::move(body));
}

std::unique_ptr<Statement> Parser::returnStatement()
{
    auto value = expression();
    if (hasError) return nullptr;

    return std::make_unique<ReturnStatement>(std::move(value));
}

std::unique_ptr<Statement> Parser::variableStatment()
{
    VariableStatement::Kind kind;

    if (previous().type == TokenType::VAR) {
        kind = VariableStatement::Kind::VAR;
    }
    else if (previous().type == TokenType::VAL) {
        kind = VariableStatement::Kind::VAL;
    }

    auto name = consume(TokenType::IDENTIFIER, "Expect variable name.").value;
    if (hasError) return nullptr;

    std::unique_ptr<Type> varType = nullptr;
    if (match(TokenType::COLON)) {
        varType = type();
        if (hasError) return nullptr;
    }

    std::unique_ptr<Expression> initializer = nullptr;
    if (match(TokenType::ASSIGN)) {
        initializer = expression();
        if (hasError) return nullptr;
    }

    return std::make_unique<VariableStatement>(
        kind, std::get<std::string>(name), std::move(varType), std::move(initializer));
}

std::unique_ptr<Declaration> Parser::declaration()
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

    hasError     = true;
    currentError = createError(peek(), "Expect declaration.");
    return nullptr;
}

std::unique_ptr<Declaration> Parser::functionDeclaration()
{
    bool isOperator = previous().type == TokenType::OPERATOR;

    auto name = consume(TokenType::IDENTIFIER, "Expect function name.").value;
    if (hasError) return nullptr;

    consume(TokenType::LPAREN, "Expect '(' after function name.");
    if (hasError) return nullptr;

    std::vector<FunctionParameter> parameters;
    if (!check(TokenType::RPAREN)) {
        do {
            auto paramName = consume(TokenType::IDENTIFIER, "Expect parameter name.").value;
            if (hasError) return nullptr;

            std::unique_ptr<Type> paramType = nullptr;
            if (match(TokenType::COLON)) {
                paramType = type();
                if (hasError) return nullptr;
            }

            std::unique_ptr<Expression> defaultValue = nullptr;
            if (match(TokenType::ASSIGN)) {
                defaultValue = expression();
                if (hasError) return nullptr;
            }

            parameters.push_back(FunctionParameter(
                std::get<std::string>(paramName), std::move(paramType), std::move(defaultValue)));
        } while (match(TokenType::COMMA));
    }

    consume(TokenType::RPAREN, "Expect ')' after parameters.");
    if (hasError) return nullptr;

    std::unique_ptr<Type> returnType = nullptr;
    if (match(TokenType::ARROW)) {
        returnType = type();
        if (hasError) return nullptr;
    }

    std::unique_ptr<Statement> body = nullptr;
    if (match(TokenType::ASSIGN)) {
        auto expr = expression();
        if (hasError) return nullptr;

        auto returnStmt = std::make_unique<ReturnStatement>(std::move(expr));
        std::vector<std::unique_ptr<Statement>> statements;
        statements.push_back(std::move(returnStmt));
        body = std::make_unique<BlockStatement>(std::move(statements));
    }
    else if (match(TokenType::LBRACE)) {
        std::vector<std::unique_ptr<Statement>> statements;

        while (!check(TokenType::RBRACE) && !isAtEnd()) {
            statements.push_back(statement());
            if (hasError) return nullptr;
        }

        consume(TokenType::RBRACE, "Expect '}' after block.");
        if (hasError) return nullptr;

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
    if (hasError) return nullptr;

    consume(TokenType::LBRACE, "Expect '{' before enum values.");
    if (hasError) return nullptr;

    std::vector<std::string> values;

    if (!check(TokenType::RBRACE)) {
        do {
            auto value = consume(TokenType::IDENTIFIER, "Expect enum value name.").value;
            if (hasError) return nullptr;

            values.push_back(std::get<std::string>(value));
        } while (match(TokenType::COMMA));
    }

    consume(TokenType::RBRACE, "Expect '}' after enum values.");
    if (hasError) return nullptr;

    return std::make_unique<EnumDeclaration>(std::get<std::string>(name), std::move(values));
}

std::unique_ptr<Declaration> Parser::classDeclaration()
{
    ClassDeclaration::Kind kind;

    if (previous().type == TokenType::DATA) {
        kind = ClassDeclaration::Kind::DATA;
        consume(TokenType::CLASS, "Expect class");
        if (hasError) return nullptr;
    }
    else if (previous().type == TokenType::BASE) {
        kind = ClassDeclaration::Kind::BASE;
        consume(TokenType::CLASS, "Expect class");
        if (hasError) return nullptr;
    }
    else {
        kind = ClassDeclaration::Kind::NORMAL;
    }

    auto name = consume(TokenType::IDENTIFIER, "Expect class name.").value;
    if (hasError) return nullptr;

    std::vector<std::string> typeParameters;
    if (match(TokenType::LT)) {
        do {
            auto param = consume(TokenType::IDENTIFIER, "Expect type parameter name.").value;
            if (hasError) return nullptr;

            typeParameters.push_back(std::get<std::string>(param));
        } while (match(TokenType::COMMA));

        consume(TokenType::GT, "Expect '>' after type parameters.");
        if (hasError) return nullptr;
    }

    consume(TokenType::LPAREN, "Expect '(' after class name.");
    if (hasError) return nullptr;

    std::vector<FunctionParameter> constructorParameters;
    if (!check(TokenType::RPAREN)) {
        do {
            auto paramName = consume(TokenType::IDENTIFIER, "Expect parameter name.").value;
            if (hasError) return nullptr;

            std::unique_ptr<Type> paramType = nullptr;
            if (match(TokenType::COLON)) {
                paramType = type();
                if (hasError) return nullptr;
            }

            std::unique_ptr<Expression> defaultValue = nullptr;
            if (match(TokenType::ASSIGN)) {
                defaultValue = expression();
                if (hasError) return nullptr;
            }

            constructorParameters.push_back(FunctionParameter(
                std::get<std::string>(paramName), std::move(paramType), std::move(defaultValue)));
        } while (match(TokenType::COMMA));
    }

    consume(TokenType::RPAREN, "Expect ')' after constructor parameters.");
    if (hasError) return nullptr;

    std::string                              baseClass;
    std::vector<std::unique_ptr<Type>>       baseTypeArguments;
    std::vector<std::unique_ptr<Expression>> baseConstructorArgs;

    if (match(TokenType::INHERITS)) {
        auto baseClassName = consume(TokenType::IDENTIFIER, "Expect base class name.").value;
        if (hasError) return nullptr;

        baseClass = std::get<std::string>(baseClassName);

        if (match(TokenType::LT)) {
            do {
                baseTypeArguments.push_back(type());
                if (hasError) return nullptr;
            } while (match(TokenType::COMMA));

            consume(TokenType::GT, "Expect '>' after base class type arguments.");
            if (hasError) return nullptr;
        }

        consume(TokenType::LPAREN, "Expect '(' after base class name.");
        if (hasError) return nullptr;

        if (!check(TokenType::RPAREN)) {
            do {
                baseConstructorArgs.push_back(expression());
                if (hasError) return nullptr;
            } while (match(TokenType::COMMA));
        }

        consume(TokenType::RPAREN, "Expect ')' after base constructor arguments.");
        if (hasError) return nullptr;
    }

    consume(TokenType::LBRACE, "Expect '{' before class body.");
    if (hasError) return nullptr;

    std::vector<std::unique_ptr<ClassMember>> members;
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        auto member = classMember();
        if (hasError) return nullptr;

        if (member) {
            members.push_back(std::move(member));
        }
    }

    consume(TokenType::RBRACE, "Expect '}' after class body.");
    if (hasError) return nullptr;

    return std::make_unique<ClassDeclaration>(kind,
                                              std::get<std::string>(name),
                                              std::move(typeParameters),
                                              std::move(constructorParameters),
                                              std::move(baseClass),
                                              std::move(baseTypeArguments),
                                              std::move(baseConstructorArgs),
                                              std::move(members));
}

std::unique_ptr<ClassMember> Parser::classMember()
{
    if (match({TokenType::VAR, TokenType::VAL})) {
        auto name = consume(TokenType::IDENTIFIER, "Expect property name.").value;
        if (hasError) return nullptr;

        std::unique_ptr<Type> propType = nullptr;
        if (match(TokenType::COLON)) {
            propType = type();
            if (hasError) return nullptr;
        }

        std::unique_ptr<Expression> initializer = nullptr;
        if (match(TokenType::ASSIGN)) {
            initializer = expression();
            if (hasError) return nullptr;
        }

        return std::make_unique<PropertyMember>(
            std::get<std::string>(name), std::move(propType), std::move(initializer));
    }
    else if (match(TokenType::INIT)) {
        consume(TokenType::LBRACE, "Expect '{' after 'init'.");
        if (hasError) return nullptr;

        std::vector<std::unique_ptr<Statement>> statements;
        while (!check(TokenType::RBRACE) && !isAtEnd()) {
            statements.push_back(statement());
            if (hasError) return nullptr;
        }

        consume(TokenType::RBRACE, "Expect '}' after init block.");
        if (hasError) return nullptr;

        return std::make_unique<InitBlockMember>(
            std::make_unique<BlockStatement>(std::move(statements)));
    }
    else if (match(TokenType::FN) || (match(TokenType::OPERATOR) && match(TokenType::FUN))) {
        auto functionDecl = functionDeclaration();
        if (hasError) return nullptr;

        return std::make_unique<MethodMember>(std::unique_ptr<FunctionDeclaration>(
            static_cast<FunctionDeclaration*>(functionDecl.release())));
    }

    hasError     = true;
    currentError = createError(peek(), "Expect class member.");
    return nullptr;
}

std::unique_ptr<Type> Parser::type()
{
    if (match(TokenType::VOID)) {
        return std::make_unique<PrimitiveType>(PrimitiveType::Kind::VOID);
    }
    else if (match(TokenType::INT_TYPE)) {
        return std::make_unique<PrimitiveType>(PrimitiveType::Kind::INT);
    }
    else if (match(TokenType::FLOAT_TYPE)) {
        return std::make_unique<PrimitiveType>(PrimitiveType::Kind::FLOAT);
    }
    else if (match(TokenType::BOOL_TYPE)) {
        return std::make_unique<PrimitiveType>(PrimitiveType::Kind::BOOL);
    }
    else if (match(TokenType::STRING_TYPE)) {
        return std::make_unique<PrimitiveType>(PrimitiveType::Kind::STRING);
    }
    else if (match(TokenType::IDENTIFIER)) {
        std::string typeName = std::get<std::string>(previous().value);

        if (match(TokenType::LT)) {
            std::vector<std::unique_ptr<Type>> typeArguments;

            do {
                typeArguments.push_back(type());
                if (hasError) return nullptr;
            } while (match(TokenType::COMMA));

            consume(TokenType::GT, "Expect '>' after type arguments.");
            if (hasError) return nullptr;

            return std::make_unique<GenericType>(std::move(typeName), std::move(typeArguments));
        }

        return std::make_unique<NamedType>(std::move(typeName));
    }

    hasError     = true;
    currentError = createError(peek(), "Expect type.");
    return nullptr;
}

std::variant<std::unique_ptr<Program>, ParseError> Parser::parse()
{
    hasError = false;
    current  = 0;

    std::vector<std::unique_ptr<Declaration>> declarations;

    while (!isAtEnd()) {
        auto decl = declaration();

        if (hasError) {
            return currentError;
        }

        if (decl) {
            declarations.push_back(std::move(decl));
        }
        else {
            if (isAtEnd()) break;

            hasError     = true;
            currentError = createError(peek(), "Expect declaration.");
            return currentError;
        }
    }

    return std::make_unique<Program>(std::move(declarations));
}