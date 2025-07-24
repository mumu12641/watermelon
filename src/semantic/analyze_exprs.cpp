#include "../include/semantic/semantic.hpp"
#include "../include/utils/format.hpp"

#include <typeindex>

std::pair<std::unique_ptr<Type>, std::optional<Error>> SemanticAnalyzer::analyzeExpression(
    Expression& expr)
{
    auto analyzeAndSetType =
        [&](auto&& analyzeFunc,
            auto&  specificExpr) -> std::pair<std::unique_ptr<Type>, std::optional<Error>> {
        auto [type, err] = analyzeFunc(specificExpr);
        if (err) return {nullptr, err};

        expr.setType(std::move(*type));
        return {std::make_unique<Type>(expr.getType()), std::nullopt};
    };

    if (auto* arrayExpr = dynamic_cast<ArrayExpression*>(&expr)) {
        return analyzeAndSetType([this](auto& e) { return analyzeArrayExpression(e); }, *arrayExpr);
    }
    else if (auto* binaryExpr = dynamic_cast<BinaryExpression*>(&expr)) {
        return analyzeAndSetType([this](auto& e) { return analyzeBinaryExpression(e); },
                                 *binaryExpr);
    }
    else if (auto* callExpr = dynamic_cast<CallExpression*>(&expr)) {
        return analyzeAndSetType([this](auto& e) { return analyzeCallExpression(e); }, *callExpr);
    }
    else if (auto* idExpr = dynamic_cast<IdentifierExpression*>(&expr)) {
        return analyzeAndSetType([this](auto& e) { return analyzeIdentifierExpression(e); },
                                 *idExpr);
    }
    else if (auto* lambdaExpr = dynamic_cast<LambdaExpression*>(&expr)) {
        return analyzeAndSetType([this](auto& e) { return analyzeLambdaExpression(e); },
                                 *lambdaExpr);
    }
    else if (auto* literalExpr = dynamic_cast<LiteralExpression*>(&expr)) {
        return analyzeAndSetType([this](auto& e) { return analyzeLiteralExpression(e); },
                                 *literalExpr);
    }
    else if (auto* memberExpr = dynamic_cast<MemberExpression*>(&expr)) {
        return analyzeAndSetType([this](auto& e) { return analyzeMemberExpression(e); },
                                 *memberExpr);
    }
    else if (auto* typeCheckExpr = dynamic_cast<TypeCheckExpression*>(&expr)) {
        return analyzeAndSetType([this](auto& e) { return analyzeTypeCheckExpression(e); },
                                 *typeCheckExpr);
    }
    else if (auto* unaryExpr = dynamic_cast<UnaryExpression*>(&expr)) {
        return analyzeAndSetType([this](auto& e) { return analyzeUnaryExpression(e); }, *unaryExpr);
    }

    return {nullptr, std::nullopt};
}


std::pair<std::unique_ptr<Type>, std::optional<Error>> SemanticAnalyzer::analyzeArrayExpression(
    ArrayExpression& expr)
{
    // TODO analyzeArrayExpression
    return {nullptr, Error("Not yet implemented ArrayExpression Semantic Check")};
}

std::pair<std::unique_ptr<Type>, std::optional<Error>> SemanticAnalyzer::analyzeBinaryExpression(
    BinaryExpression& expr)
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
            if (leftType->canMathOp() && rightType->canMathOp() && *leftType == *rightType) {
                return {std::make_unique<Type>(std::move(*leftType)), std::nullopt};
            }
            else {
                return {nullptr,
                        Error(Format("Incompatible types: '{0}' and '{1}' in arithmetic operation",
                                     leftType->getName(),
                                     rightType->getName()),
                              expr.getLocation())};
            }
            break;

        case BinaryExpression::Operator::EQ:
        case BinaryExpression::Operator::NEQ:
        case BinaryExpression::Operator::LT:
        case BinaryExpression::Operator::LE:
        case BinaryExpression::Operator::GT:
        case BinaryExpression::Operator::GE:
            if (leftType->canCompare() && rightType->canCompare() && *leftType == *rightType) {
                return {std::make_unique<Type>(Type::builtinBool()), std::nullopt};
            }
            else {
                return {nullptr,
                        Error(Format("Incompatible types: '{0}' and '{1}' in comparison operation",
                                     leftType->getName(),
                                     rightType->getName()),
                              expr.getLocation())};
            }
            break;

        case BinaryExpression::Operator::AND:
        case BinaryExpression::Operator::OR:
            if (leftType->isBool() && rightType->isBool()) {
                return {std::make_unique<Type>(Type::builtinBool()), std::nullopt};
            }
            else {
                return {nullptr,
                        Error(Format("Incompatible types: '{0}' and '{1}' in logical operation",
                                     leftType->getName(),
                                     rightType->getName()),
                              expr.getLocation())};
            }
            break;


        case BinaryExpression::Operator::ASSIGN:
            // TODO: 检查右侧类型是否可以赋值给左侧
            // TODO: 左侧的变量是否可以可变
            // directly cast
            if (const auto* leftExpr = dynamic_cast<const IdentifierExpression*>(expr.left.get())) {
                if (*(this->symbolTable.findKind(leftExpr->name)) == SymbolKind::VAL) {
                    return {
                        nullptr,
                        Error(Format("Cannot assign to immutable variable '{0}'", leftExpr->name),
                              expr.getLocation())};
                }
            }
            else if (const auto* memberExpr =
                         dynamic_cast<const MemberExpression*>(expr.left.get())) {
                if (*(this->symbolTable.findKind(Format("self_{0}", memberExpr->property))) ==
                    SymbolKind::VAL) {
                    return {
                        nullptr,
                        Error(Format("Cannot assign to immutable variable '{0}'", leftExpr->name),
                              expr.getLocation())};
                }
            }


            if (!this->classTable.checkInherit(rightType->getName(), leftType->getName())) {
                return {nullptr,
                        Error(Format("Cannot assign value of type '{0}' to variable of type '{1}'",
                                     rightType->getName(),
                                     leftType->getName()),
                              expr.getLocation())};
            }


            return {std::make_unique<Type>(std::move(*leftType)), std::nullopt};
    }
    return {nullptr, std::nullopt};
}

std::pair<std::unique_ptr<Type>, std::optional<Error>> SemanticAnalyzer::analyzeCallExpression(
    CallExpression& expr)
{
    if (dynamic_cast<IdentifierExpression*>(expr.callee.get()) == nullptr) {
        return {nullptr,
                Error("The callee in a call expression must be an identifier",
                      expr.callee->getLocation())};
    }

    auto [calleeType, errorCallee] = analyzeExpression(*expr.callee);
    if (errorCallee) return {nullptr, errorCallee};

    auto validateArguments = [this, &expr](const std::vector<FunctionParameter>& params,
                                           const std::string&                    callType,
                                           const std::string& name) -> std::optional<Error> {
        int requiredParamCount = 0;
        for (const auto& param : params) {
            if (param.defaultValue == nullptr) requiredParamCount++;
        }
        if (expr.arguments.size() < requiredParamCount) {
            return Error(
                Format("{0} '{1}' requires at least {2} arguments, but only {3} were provided",
                       callType,
                       name,
                       requiredParamCount,
                       expr.arguments.size()),
                expr.getLocation());
        }
        if (expr.arguments.size() > params.size()) {
            return Error(Format("{0} '{1}' accepts at most {2} arguments, but {3} were provided",
                                callType,
                                name,
                                params.size(),
                                expr.arguments.size()),
                         expr.getLocation());
        }
        size_t i = 0;
        for (; i < expr.arguments.size(); i++) {
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
        while (i < params.size()) {
            auto [defaultType, errorDefault] = analyzeExpression(*(params[i].defaultValue));
            if (errorDefault) return errorDefault;
            const std::string& declDefaultValType = params[i].type->getName();
            if (!this->classTable.checkInherit(defaultType->getName(), declDefaultValType)) {
                return Error(Format("Argument {0}: cannot convert from '{1}' to '{2}' in {3} '{4}'",
                                    i + 1,
                                    defaultType->getName(),
                                    declDefaultValType,
                                    callType,
                                    name),
                             params[i].defaultValue->getLocation());
            }
            i++;
        }
        return std::nullopt;
    };

    const auto* cls  = this->classTable.find(calleeType->getName());
    const auto* func = this->functionTable.find(calleeType->getName());

    if (cls) {
        if (auto error = validateArguments(cls->constructorParameters, "constructor", cls->name)) {
            return {nullptr, error};
        }
        return {std::make_unique<Type>(Type::classType(cls->name)), std::nullopt};
    }

    if (func) {
        if (auto error = validateArguments(func->parameters, "function", func->name)) {
            return {nullptr, error};
        }
        return {std::make_unique<Type>(*func->returnType), std::nullopt};
    }

    return {nullptr, Error("Failed to process call", expr.getLocation())};
}

std::pair<std::unique_ptr<Type>, std::optional<Error>>
SemanticAnalyzer::analyzeIdentifierExpression(IdentifierExpression& expr)
{
    auto symbol = this->symbolTable.findType(expr.name);
    if (symbol == nullptr)
        return {nullptr,
                Error(Format("Undefined identifier '{0}'", expr.name), expr.getLocation())};
    return {std::make_unique<Type>(*symbol), std::nullopt};
}

std::pair<std::unique_ptr<Type>, std::optional<Error>> SemanticAnalyzer::analyzeLambdaExpression(
    LambdaExpression& expr)
{
    // TODO analyzeLambdaExpression
    return {nullptr, Error("Not yet implemented LambdaExpression Semantic Check")};
}

std::pair<std::unique_ptr<Type>, std::optional<Error>> SemanticAnalyzer::analyzeLiteralExpression(
    LiteralExpression& expr)
{
    switch (expr.getType().kind) {
        case Type::Kind::INT: return {std::make_unique<Type>(Type::builtinInt()), std::nullopt};
        case Type::Kind::FLOAT: return {std::make_unique<Type>(Type::builtinFloat()), std::nullopt};
        case Type::Kind::BOOL: return {std::make_unique<Type>(Type::builtinBool()), std::nullopt};
        case Type::Kind::STRING:
            return {std::make_unique<Type>(Type::builtinString()), std::nullopt};
    }
    return {nullptr, std::nullopt};
}

std::pair<std::unique_ptr<Type>, std::optional<Error>> SemanticAnalyzer::analyzeMemberExpression(
    MemberExpression& expr)
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

        // property return directly
        return {std::make_unique<Type>(classMember->getType()), std::nullopt};
    }
    else {
        return {nullptr,
                Error("This expr is not a class and cannot access members", expr.getLocation())};
    }
}

std::pair<std::unique_ptr<Type>, std::optional<Error>> SemanticAnalyzer::analyzeTypeCheckExpression(
    TypeCheckExpression& expr)
{
    // TODO analyzeTypeCheckExpression
    return {nullptr, Error("Not yet implemented TypeCheckExpression Semantic Check")};
}

std::pair<std::unique_ptr<Type>, std::optional<Error>> SemanticAnalyzer::analyzeUnaryExpression(
    UnaryExpression& expr)
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
            return {std::make_unique<Type>(std::move(*operandType)), std::nullopt};

        case UnaryExpression::Operator::NOT:
            if (!operandType->isBool()) {
                return {
                    nullptr,
                    Error(Format("Logical negation operator '!' cannot be applied to type '{0}'",
                                 operandType->getName()),
                          expr.getLocation())};
            }
            return {std::make_unique<Type>(std::move(*operandType)), std::nullopt};
    }

    return {nullptr, std::nullopt};
}