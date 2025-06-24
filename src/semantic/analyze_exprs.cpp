#include "../include/semantic/semantic.hpp"
#include "../include/utils/format.hpp"

std::pair<std::unique_ptr<SymbolType>, std::optional<Error>> SemanticAnalyzer::analyzeExpression(
    const Expression& expr)
{
    if (const auto* arrayExpr = dynamic_cast<const ArrayExpression*>(&expr)) {
        return analyzeArrayExpression(*arrayExpr);
    }
    else if (const auto* binaryExpr = dynamic_cast<const BinaryExpression*>(&expr)) {
        return analyzeBinaryExpression(*binaryExpr);
    }
    else if (const auto* callExpr = dynamic_cast<const FunctionCallExpression*>(&expr)) {
        return analyzeFunctionCallExpression(*callExpr);
    }
    else if (const auto* idExpr = dynamic_cast<const IdentifierExpression*>(&expr)) {
        return analyzeIdentifierExpression(*idExpr);
    }
    else if (const auto* lambdaExpr = dynamic_cast<const LambdaExpression*>(&expr)) {
        return analyzeLambdaExpression(*lambdaExpr);
    }
    else if (const auto* literalExpr = dynamic_cast<const LiteralExpression*>(&expr)) {
        return analyzeLiteralExpression(*literalExpr);
    }
    else if (const auto* memberExpr = dynamic_cast<const MemberExpression*>(&expr)) {
        return analyzeMemberExpression(*memberExpr);
    }
    else if (const auto* typeCheckExpr = dynamic_cast<const TypeCheckExpression*>(&expr)) {
        return analyzeTypeCheckExpression(*typeCheckExpr);
    }
    else if (const auto* unaryExpr = dynamic_cast<const UnaryExpression*>(&expr)) {
        return analyzeUnaryExpression(*unaryExpr);
    }
    return {nullptr, std::nullopt};
}

std::pair<std::unique_ptr<SymbolType>, std::optional<Error>>
SemanticAnalyzer::analyzeArrayExpression(const ArrayExpression& expr)
{
    return {nullptr, Error("Not yet implemented ArrayExpression Semantic Check")};
}

std::pair<std::unique_ptr<SymbolType>, std::optional<Error>>
SemanticAnalyzer::analyzeBinaryExpression(const BinaryExpression& expr)
{
    auto [leftType, errorLeft] = analyzeExpression(*expr.left);
    if (errorLeft) return {nullptr, errorLeft};
    auto [rightType, errorRight] = analyzeExpression(*expr.right);
    if (errorRight) return {nullptr, errorRight};

    switch (expr.op) {
        case BinaryExpression::Operator::ADD:
        case BinaryExpression::Operator::SUB:
        case BinaryExpression::Operator::MUL:
        case BinaryExpression::Operator::DIV:
        case BinaryExpression::Operator::MOD:
            // TODO: 检查操作数类型是否兼容
            // return {std::make_unique<PrimitiveType>(PrimitiveType::Kind::INT), std::nullopt};


        case BinaryExpression::Operator::EQ:
        case BinaryExpression::Operator::NEQ:
        case BinaryExpression::Operator::LT:
        case BinaryExpression::Operator::LE:
        case BinaryExpression::Operator::GT:
        case BinaryExpression::Operator::GE:
        case BinaryExpression::Operator::AND:
        case BinaryExpression::Operator::OR:
            // return {std::make_unique<PrimitiveType>(PrimitiveType::Kind::BOOL), std::nullopt};

        case BinaryExpression::Operator::ASSIGN:
            // 赋值运算符
            // TODO: 检查右侧类型是否可以赋值给左侧
            // 返回左侧类型
            if (!this->classTable.checkInherit(rightType->getName(), leftType->getName())) {
                return {nullptr,
                        Error("The left and right types in your assignment expression "
                              "do not match",
                              expr.getLocation())};
            }
            // return {std::make_unique<Type>(leftType.get()), std::nullopt};
    }
    return {nullptr, std::nullopt};
}

std::pair<std::unique_ptr<SymbolType>, std::optional<Error>>
SemanticAnalyzer::analyzeFunctionCallExpression(const FunctionCallExpression& expr)
{
    auto [calleeType, errorCallee] = analyzeExpression(*expr.callee);
    if (errorCallee) return {nullptr, errorCallee};
    if (calleeType->getKind() != SymbolType::SymbolKind::FUNC) {
        return {nullptr,
                Error("The callee of CallExpression must be a function", expr.getLocation())};
    }
    if (const auto* func = this->functionTable.find(calleeType->getName())) {
        if (func->parameters.size() != expr.arguments.size()) {
            return {nullptr,
                    Error("The actual number of arguments in this function call expression "
                          "does not match the number of arguments declared",
                          expr.getLocation())};
        }
        std::vector<std::unique_ptr<SymbolType>> argTypes;
        for (size_t i = 0; i < func->parameters.size(); i++) {
            auto [currArgType, errorArg] = analyzeExpression(*(expr.arguments[i]));
            if (errorArg) return {nullptr, errorArg};
            if (!this->classTable.checkInherit(currArgType->getName(),
                                               func->parameters[i].type->getName())) {
                return {nullptr,
                        Error(Format("The type of the {0}th parameter in this function call "
                                     "expression does not match the declared type.",
                                     i + 1),
                              expr.getLocation())};
            }
        }
        return {std::make_unique<SymbolType>(func->returnType->getName()), std::nullopt};
    }
    return {nullptr, std::nullopt};
}

std::pair<std::unique_ptr<SymbolType>, std::optional<Error>>
SemanticAnalyzer::analyzeIdentifierExpression(const IdentifierExpression& expr)
{
    auto symbol = this->symbolTable.find(expr.name);
    if (symbol == nullptr)
        return {nullptr,
                Error(Format("This Identifier {0} is not defined", expr.name), expr.getLocation())};
    return {std::make_unique<SymbolType>(*symbol), std::nullopt};
}

std::pair<std::unique_ptr<SymbolType>, std::optional<Error>>
SemanticAnalyzer::analyzeLambdaExpression(const LambdaExpression& expr)
{
    return {nullptr, std::nullopt};
}

std::pair<std::unique_ptr<SymbolType>, std::optional<Error>>
SemanticAnalyzer::analyzeLiteralExpression(const LiteralExpression& expr)
{
    switch (expr.kind) {
        case LiteralExpression::Kind::INT:
            return {std::make_unique<SymbolType>("int"), std::nullopt};
        case LiteralExpression::Kind::FLOAT:
            return {std::make_unique<SymbolType>("int"), std::nullopt};
        case LiteralExpression::Kind::BOOL:
            return {std::make_unique<SymbolType>("int"), std::nullopt};
        case LiteralExpression::Kind::STRING:
            return {std::make_unique<SymbolType>("int"), std::nullopt};
    }
    return {nullptr, std::nullopt};
}

std::pair<std::unique_ptr<SymbolType>, std::optional<Error>>
SemanticAnalyzer::analyzeMemberExpression(const MemberExpression& expr)
{
    auto [objectType, errorObject] = analyzeExpression(*expr.object);
    if (errorObject) return {nullptr, errorObject};
    if (const auto* objectClass = this->classTable.find(objectType->getName())) {
        auto* classMember = objectClass->containMember(expr.property);
        if (classMember == nullptr) {
            return {nullptr,
                    Error(Format("This class {0} does not have a member named {1}",
                                 objectClass->name,
                                 expr.property),
                          expr.getLocation())};
        }
        return {std::make_unique<SymbolType>(classMember->getType()), std::nullopt};
    }
    else {
        return {nullptr,
                Error("This expr is not a class and cannot access properties", expr.getLocation())};
    }
}

std::pair<std::unique_ptr<SymbolType>, std::optional<Error>>
SemanticAnalyzer::analyzeTypeCheckExpression(const TypeCheckExpression& expr)
{
    return {nullptr, std::nullopt};
}

std::pair<std::unique_ptr<SymbolType>, std::optional<Error>>
SemanticAnalyzer::analyzeUnaryExpression(const UnaryExpression& expr)
{
    auto [operandType, errorOp] = analyzeExpression(*expr.operand);
    if (errorOp) return {nullptr, errorOp};
    switch (expr.op) {
        case UnaryExpression::Operator::NEG:
            if (!operandType->canMathOp()) {
                return {nullptr,
                        Error("This expression cannot use the NEG operator", expr.getLocation())};
            }
            return {std::make_unique<SymbolType>(operandType->getName()), std::nullopt};

        case UnaryExpression::Operator::NOT:
            if (!operandType->isBool()) {
                return {nullptr,
                        Error("This expression cannot use the NOT operator", expr.getLocation())};
            }
            return {std::make_unique<SymbolType>(operandType->getName()), std::nullopt};
    }
    return {nullptr, std::nullopt};
}
