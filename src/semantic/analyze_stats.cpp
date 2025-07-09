#include "../include/semantic/semantic.hpp"
#include "../include/utils/format.hpp"
std::optional<Error> SemanticAnalyzer::analyzeStatement(const Statement& stmt)
{
    if (const auto* blockStmt = dynamic_cast<const BlockStatement*>(&stmt)) {
        return analyzeBlockStatement(*blockStmt);
    }
    else if (const auto* exprStmt = dynamic_cast<const ExpressionStatement*>(&stmt)) {
        return analyzeExpressionStatement(*exprStmt);
    }
    else if (const auto* forStmt = dynamic_cast<const ForStatement*>(&stmt)) {
        return analyzeForStatement(*forStmt);
    }
    else if (const auto* ifStmt = dynamic_cast<const IfStatement*>(&stmt)) {
        return analyzeIfStatement(*ifStmt);
    }
    else if (const auto* returnStmt = dynamic_cast<const ReturnStatement*>(&stmt)) {
        return analyzeReturnStatement(*returnStmt);
    }
    else if (const auto* varStmt = dynamic_cast<const VariableStatement*>(&stmt)) {
        return analyzeVariableStatement(*varStmt);
    }
    else if (const auto* whenStmt = dynamic_cast<const WhenStatement*>(&stmt)) {
        return analyzeWhenStatement(*whenStmt);
    }
    return std::nullopt;
}

std::optional<Error> SemanticAnalyzer::analyzeBlockStatement(const BlockStatement& stmt)
{
    this->symbolTable.enterScope("block");
    for (const auto& s : stmt.statements) {
        auto error = analyzeStatement(*s);
        if (error) return error;
    }
    this->symbolTable.exitScope();
    return std::nullopt;
}

std::optional<Error> SemanticAnalyzer::analyzeExpressionStatement(const ExpressionStatement& stmt)
{
    auto [_, error] = analyzeExpression(*stmt.expression);
    return error;
}

std::optional<Error> SemanticAnalyzer::analyzeForStatement(const ForStatement& stmt)
{
    auto [iterableType, errorIterableExpr] = analyzeExpression(*stmt.iterable);
    if (errorIterableExpr) return errorIterableExpr;
    // TODO: 检查iterableType是否可迭代
    auto iterable = this->classTable.isClassIterable(iterableType->getName());
    if (iterable == nullptr) {
        return Error(Format("Type '{0}' is not iterable - must implement both '_first' and '_next' "
                            "operator methods with matching return types",
                            iterableType->getName()),
                     stmt.iterable->getLocation());
    }

    this->symbolTable.enterScope("for-loop");
    // TODO: get variable type from iterableType
    this->symbolTable.add(stmt.variable, iterable->second, SymbolType::SymbolKind::VAL);

    auto errorBody = analyzeStatement(*stmt.body);
    if (errorBody) return errorBody;
    this->symbolTable.exitScope();
    return std::nullopt;
}

std::optional<Error> SemanticAnalyzer::analyzeIfStatement(const IfStatement& stmt)
{
    auto [conditionType, errorCondition] = analyzeExpression(*stmt.condition);
    if (errorCondition) return errorCondition;

    if (!conditionType->isBool()) {
        return Error(
            Format("If condition must be of type 'bool', got '{0}'", conditionType->getName()),
            stmt.condition->getLocation());
    }

    this->symbolTable.enterScope("if-then");
    auto errorThen = analyzeStatement(*stmt.thenBranch);
    if (errorThen) return errorThen;
    this->symbolTable.exitScope();

    if (stmt.elseBranch) {
        this->symbolTable.enterScope("if-else");
        auto errorElse = analyzeStatement(*stmt.elseBranch);
        if (errorElse) return errorElse;

        this->symbolTable.exitScope();
    }
    return std::nullopt;
}

std::optional<Error> SemanticAnalyzer::analyzeReturnStatement(const ReturnStatement& stmt)
{
    if (stmt.value) {
        auto [actualReturnType, errorReturn] = analyzeExpression(*stmt.value);
        if (errorReturn) return errorReturn;
        this->currentFunctionReturnTypes.push(
            std::make_pair(SymbolType(actualReturnType->getName()), stmt.getLocation()));
    }
    return std::nullopt;
}

std::optional<Error> SemanticAnalyzer::analyzeVariableStatement(const VariableStatement& stmt)
{
    std::unique_ptr<SymbolType> initType = nullptr;
    if (stmt.initializer) {
        auto [analyzedType, initError] = analyzeExpression(*stmt.initializer);
        if (initError) return initError;
        initType = std::move(analyzedType);
    }

    if (stmt.type) {
        if (initType != nullptr &&
            !this->classTable.checkInherit(initType->getName(), stmt.type->getName())) {
            return Error(
                Format("Cannot initialize variable '{0}' of type '{1}' with value of type '{2}'",
                       stmt.name,
                       stmt.type->getName(),
                       initType->getName()),
                stmt.getLocation());
        }
        this->symbolTable.add(stmt.name,
                              stmt.type->getName(),
                              stmt.immutable);
    }
    else if (initType) {
        this->symbolTable.add(stmt.name,
                              initType->getName(),
                              stmt.immutable);
    }
    return std::nullopt;
}

std::optional<Error> SemanticAnalyzer::analyzeWhenStatement(const WhenStatement& stmt)
{
    auto [subjectType, errorSubject] = analyzeExpression(*stmt.subject);
    if (errorSubject) return errorSubject;

    for (const auto& caseItem : stmt.cases) {
        auto [caseValueType, errorCase] = analyzeExpression(*caseItem.value);
        if (errorCase) return errorCase;

        if (!this->classTable.checkInherit(caseValueType->getName(), subjectType->getName())) {
            return Error(Format("Type mismatch in when case: expected '{0}', got '{1}'",
                                subjectType->getName(),
                                caseValueType->getName()),
                         caseItem.value->getLocation());
        }
        this->symbolTable.enterScope("when-case");
        auto errorBody = analyzeStatement(*caseItem.body);
        if (errorBody) return errorBody;
        this->symbolTable.exitScope();
    }
    return std::nullopt;
}
