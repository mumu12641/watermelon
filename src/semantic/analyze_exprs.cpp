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
    else if (const auto* callExpr = dynamic_cast<const CallExpression*>(&expr)) {
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
    // TODO
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
            if (leftType->canMathOp() && rightType->canMathOp() &&
                leftType->getName() == rightType->getName()) {
                return {std::make_unique<SymbolType>(leftType->getName()), std::nullopt};
            }
            else {
                return {nullptr,
                        Error(Format("Incompatible types: '{0}' and '{1}' in arithmetic operation",
                                     leftType->getName(),
                                     rightType->getName()),
                              expr.getLocation())};
            }

        case BinaryExpression::Operator::EQ:
        case BinaryExpression::Operator::NEQ:
        case BinaryExpression::Operator::LT:
        case BinaryExpression::Operator::LE:
        case BinaryExpression::Operator::GT:
        case BinaryExpression::Operator::GE:
            if (leftType->canCompare() && rightType->canCompare() &&
                leftType->getName() == rightType->getName()) {
                return {std::make_unique<SymbolType>("bool"), std::nullopt};
            }
            else {
                return {nullptr,
                        Error(Format("Incompatible types: '{0}' and '{1}' in comparison operation",
                                     leftType->getName(),
                                     rightType->getName()),
                              expr.getLocation())};
            }
        case BinaryExpression::Operator::AND:
        case BinaryExpression::Operator::OR:
            if (leftType->isBool() && rightType->isBool()) {
                return {std::make_unique<SymbolType>("bool"), std::nullopt};
            }
            else {
                return {nullptr,
                        Error(Format("Incompatible types: '{0}' and '{1}' in logical operation",
                                     leftType->getName(),
                                     rightType->getName()),
                              expr.getLocation())};
            }

        case BinaryExpression::Operator::ASSIGN:
            // TODO: 检查右侧类型是否可以赋值给左侧
            if (!this->classTable.checkInherit(rightType->getName(), leftType->getName())) {
                return {nullptr,
                        Error(Format("Cannot assign value of type '{0}' to variable of type '{1}'",
                                     rightType->getName(),
                                     leftType->getName()),
                              expr.getLocation())};
            }
            return {std::make_unique<SymbolType>(leftType->getName()), std::nullopt};
    }
    return {nullptr, std::nullopt};
}

std::pair<std::unique_ptr<SymbolType>, std::optional<Error>>
SemanticAnalyzer::analyzeFunctionCallExpression(const CallExpression& expr)
{
    auto [calleeType, errorCallee] = analyzeExpression(*expr.callee);
    if (errorCallee) return {nullptr, errorCallee};

    if (calleeType->getKind() == SymbolType::SymbolKind::VAR) {
        return {nullptr,
                Error("Cannot call a variable as a function or constructor", expr.getLocation())};
    }

    auto validateArguments = [this, &expr](const std::vector<FunctionParameter>& params,
                                           const std::string&                    callType,
                                           const std::string& name) -> std::optional<Error> {
        if (params.size() != expr.arguments.size()) {
            return Error(Format("{0} '{1}' expects {2} arguments, but {3} were provided",
                                callType,
                                name,
                                params.size(),
                                expr.arguments.size()),
                         expr.getLocation());
        }

        for (size_t i = 0; i < params.size(); i++) {
            auto [argType, errorArg] = analyzeExpression(*(expr.arguments[i]));
            if (errorArg) return errorArg;

            const std::string& expectedType = params[i].type->getName();
            const std::string& actualType   = argType->getName();

            if (!this->classTable.checkInherit(actualType, expectedType)) {
                return Error(Format("Argument {0}: cannot convert from '{1}' to '{2}' in {3} '{4}'",
                                    i + 1,
                                    actualType,
                                    expectedType,
                                    callType,
                                    name),
                             expr.arguments[i]->getLocation());
            }
        }

        return std::nullopt;
    };

    if (calleeType->getKind() == SymbolType::SymbolKind::CLASS) {
        const auto* cls = this->classTable.find(calleeType->getName());
        if (!cls) {
            return {
                nullptr,
                Error(Format("Class '{0}' not found", calleeType->getName()), expr.getLocation())};
        }

        if (auto error = validateArguments(cls->constructorParameters, "constructor", cls->name)) {
            return {nullptr, error};
        }

        return {std::make_unique<SymbolType>(cls->name), std::nullopt};
    }
    else {
        const auto* func = this->functionTable.find(calleeType->getName());
        if (!func) {
            return {nullptr,
                    Error(Format("Function '{0}' not found", calleeType->getName()),
                          expr.getLocation())};
        }

        if (auto error = validateArguments(func->parameters, "function", func->name)) {
            return {nullptr, error};
        }

        return {std::make_unique<SymbolType>(func->returnType->getName()), std::nullopt};
    }
}

std::pair<std::unique_ptr<SymbolType>, std::optional<Error>>
SemanticAnalyzer::analyzeIdentifierExpression(const IdentifierExpression& expr)
{
    auto symbol = this->symbolTable.find(expr.name);
    if (symbol == nullptr)
        return {nullptr,
                Error(Format("Undefined identifier '{0}'", expr.name), expr.getLocation())};
    return {std::make_unique<SymbolType>(*symbol), std::nullopt};
}

std::pair<std::unique_ptr<SymbolType>, std::optional<Error>>
SemanticAnalyzer::analyzeLambdaExpression(const LambdaExpression& expr)
{
    // TODO
    return {nullptr, Error("Not yet implemented LambdaExpression Semantic Check")};
}

std::pair<std::unique_ptr<SymbolType>, std::optional<Error>>
SemanticAnalyzer::analyzeLiteralExpression(const LiteralExpression& expr)
{
    switch (expr.kind) {
        case LiteralExpression::Kind::INT:
            return {std::make_unique<SymbolType>("int"), std::nullopt};
        case LiteralExpression::Kind::FLOAT:
            return {std::make_unique<SymbolType>("float"), std::nullopt};
        case LiteralExpression::Kind::BOOL:
            return {std::make_unique<SymbolType>("bool"), std::nullopt};
        case LiteralExpression::Kind::STRING:
            return {std::make_unique<SymbolType>("String"), std::nullopt};
    }
    return {nullptr, std::nullopt};
}

std::pair<std::unique_ptr<SymbolType>, std::optional<Error>>
SemanticAnalyzer::analyzeMemberExpression(const MemberExpression& expr)
{

    auto [objectType, errorObject] = analyzeExpression(*expr.object);
    if (errorObject) return {nullptr, errorObject};

    if (const auto* objectClass = this->classTable.find(objectType->getName())) {
        auto* classMember = objectClass->containMember(
            expr.kind == MemberExpression::Kind::PROPERTY ? expr.property : expr.methodName);
        if (classMember == nullptr) {
            return {nullptr,
                    Error(Format("This class {0} does not have a member named {1}",
                                 objectClass->name,
                                 expr.property),
                          expr.getLocation())};
        }
        if (expr.kind == MemberExpression::Kind::METHOD) {
            const auto* classMethod = dynamic_cast<const MethodMember*>(classMember);
            if (expr.arguments.size() != classMethod->function->parameters.size()) {
                return {
                    nullptr,
                    Error(Format(
                              "Method <{0}> in class <{1}> expects {2} arguments, but {3} provided",
                              expr.methodName,
                              objectClass->name,
                              classMethod->function->parameters.size(),
                              expr.arguments.size()),
                          expr.getLocation())};
            };
            for (int i = 0; i < expr.arguments.size(); i++) {
                const auto& exprArg         = expr.arguments[i];
                const auto& methodParam     = classMethod->function->parameters[i];
                auto [exprArgType, exprErr] = analyzeExpression(*exprArg);
                if (exprErr) return {nullptr, exprErr};
                if (!this->classTable.checkInherit(exprArgType->getName(),
                                                   methodParam.type->getName())) {
                    return {
                        nullptr,
                        Error(Format(
                                  "Cannot convert argument {0} from <{1}> to <{2}> in method <{3}> "
                                  "of class <{4}>",
                                  i + 1,
                                  exprArgType->getName(),
                                  methodParam.type->getName(),
                                  expr.methodName,
                                  objectClass->name),
                              exprArg->getLocation())};
                }
            }
        }

        return {std::make_unique<SymbolType>(classMember->getType()), std::nullopt};
    }
    else {
        return {nullptr,
                Error("This expr is not a class and cannot access members", expr.getLocation())};
    }
}

std::pair<std::unique_ptr<SymbolType>, std::optional<Error>>
SemanticAnalyzer::analyzeTypeCheckExpression(const TypeCheckExpression& expr)
{
    // TODO
    return {nullptr, Error("Not yet implemented TypeCheckExpression Semantic Check")};
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
                        Error(Format("Unary operator '-' cannot be applied to type '{0}'",
                                     operandType->getName()),
                              expr.getLocation())};
            }
            return {std::make_unique<SymbolType>(operandType->getName()), std::nullopt};

        case UnaryExpression::Operator::NOT:
            if (!operandType->isBool()) {
                return {
                    nullptr,
                    Error(Format("Logical negation operator '!' cannot be applied to type '{0}'",
                                 operandType->getName()),
                          expr.getLocation())};
            }
            return {std::make_unique<SymbolType>(operandType->getName()), std::nullopt};
    }

    return {nullptr, std::nullopt};
}