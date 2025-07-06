#include "../../include/parser/parser.hpp"

#include <iostream>

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
    bool isOperator = false;
    if (previous().type == TokenType::OPERATOR) {
        isOperator = true;
        advance();
    }
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
            std::cout << paramName.toString();


            std::unique_ptr<Type> paramType = nullptr;
            if (match(TokenType::COLON)) {
                auto [typeVal, typeErr] = type();
                if (typeErr) return {nullptr, typeErr};
                paramType = std::move(typeVal);
                std::cout << paramType->getName() << "\n";
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

    std::unique_ptr<Type> returnType = std::make_unique<PrimitiveType>(PrimitiveType::Kind::VOID);
    if (match(TokenType::ARROW)) {
        auto [returnTypeVal, returnTypeErr] = type();
        if (returnTypeErr) return {nullptr, returnTypeErr};
        returnType = std::move(returnTypeVal);
    }

    std::unique_ptr<Statement> body = nullptr;
    if (match(TokenType::ASSIGN)) {
        auto [expr, exprErr] = expression();
        if (exprErr) return {nullptr, exprErr};
        Location l = expr->getLocation();

        auto [_, semicolonErr] =
            consume(TokenType::SEMICOLON, "Expect ';' after function expression.");
        if (semicolonErr) return {nullptr, semicolonErr};

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
            auto [_, colonErr]              = consume(TokenType::COLON, "Expect parameter type.");
            if (colonErr) return {nullptr, colonErr};

            auto [typeVal, typeErr] = type();
            if (typeErr) return {nullptr, typeErr};
            paramType = std::move(typeVal);


            std::unique_ptr<Expression> defaultValue = nullptr;
            if (match(TokenType::ASSIGN)) {
                auto [defaultVal, defaultErr] = expression();
                if (defaultErr) return {nullptr, defaultErr};
                defaultValue = std::move(defaultVal);
            }

            constructorParameters.emplace_back(std::get<std::string>(paramName.value),
                                               std::move(paramType),
                                               std::move(defaultValue));
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
        auto [_, semicolonErr] =
            consume(TokenType::SEMICOLON, "Expect ';' after property declaration.");
        if (semicolonErr) return {nullptr, semicolonErr};

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
    else if (match(TokenType::FN) || match(TokenType::OPERATOR) || match(TokenType::FUN)) {
        auto [functionDecl, funcErr] = functionDeclaration();
        if (funcErr) return {nullptr, funcErr};
        Location l = functionDecl->getLocation();

        return {std::make_unique<MethodMember>(
                    l,
                    std::unique_ptr<FunctionDeclaration>(
                        dynamic_cast<FunctionDeclaration*>(functionDecl.release()))),
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
