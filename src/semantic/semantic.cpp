#include "../include/semantic/semantic.hpp"

#include "../include/utils/format.hpp"

void Scope::add(const std::string& key, SymbolType type)
{
    this->map[key] = type;
}

const SymbolType* Scope::find(const std::string& key)
{
    auto it = map.find(key);
    if (it != map.end()) {
        return &it->second;
    }
    return nullptr;
}

void SymbolTable::enterScope(const std::string& name = "anonymous")
{
    scopes.emplace_back(Scope(name));
}

void SymbolTable::exitScope()
{
    if (!scopes.empty()) {
        scopes.pop_back();
    }
}

const SymbolType* SymbolTable::find(const std::string& key)
{
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        const SymbolType* type = it->find(key);
        if (type) {
            return type;
        }
    }
    return nullptr;
}

void SymbolTable::add(const std::string& key, const std::string& type, SymbolType::SymbolKind kind)
{
    if (!scopes.empty()) {
        scopes.back().add(key, SymbolType(type, kind));
    }
}

void SymbolTable::debug() const
{
    std::cout << "SymbolTable Debug Information:\n";
    for (size_t i = 0; i < scopes.size(); ++i) {
        std::cout << "Scope " << i << scopes[i].getName() << ":\n";
        for (const auto& pair : scopes[i].getMap()) {
            std::cout << "  Key: " << pair.first << ", Symbol Type: " << pair.second.getName()
                      << "\n";
        }
    }
    std::cout << "\n";
}

void ClassTable::add(const std::string& className, const ClassDeclaration* classDecl)
{
    // classes.emplace_back(className, classDecl);
    classes.insert(std::pair<std::string, const ClassDeclaration*>(className, classDecl));
}

const ClassDeclaration* ClassTable::find(const std::string& className)
{
    auto iter = classes.find(className);
    return iter != classes.end() ? iter->second : nullptr;
}

void ClassTable::addInheritMap(const std::string&                   className,
                               std::vector<const ClassDeclaration*> parents)
{
    inheritMap.insert(
        std::pair<std::string, std::vector<const ClassDeclaration*>>(className, parents));
}

const std::vector<const ClassDeclaration*>* ClassTable::getInheritMap(const std::string& className)
{
    auto iter = inheritMap.find(className);
    return iter != inheritMap.end() ? &(iter->second) : nullptr;
}

const bool ClassTable::checkInherit(const std::string& child, const std::string& parent)
{
    if (child == parent) return true;
    const auto* inheritMap = getInheritMap(child);
    if (inheritMap == nullptr) return false;
    for (const auto* inherit : *inheritMap) {
        if (inherit->name == parent) return true;
    }
    return false;
}

void FunctionTable::add(const std::string& functionName, const FunctionDeclaration* functionDecl)
{
    functions.insert(
        std::pair<std::string, const FunctionDeclaration*>(functionName, functionDecl));
}

const FunctionDeclaration* FunctionTable::find(const std::string& functionName)
{
    auto iter = functions.find(functionName);
    return iter != functions.end() ? iter->second : nullptr;
}

std::optional<Error> SemanticAnalyzer::analyzeDeclaration(const Declaration& decl)
{
    if (const auto* classDecl = dynamic_cast<const ClassDeclaration*>(&decl)) {
        return analyzeClassDeclaration(*classDecl);
    }
    else if (const auto* enumDecl = dynamic_cast<const EnumDeclaration*>(&decl)) {
        return analyzeEnumDeclaration(*enumDecl);
    }
    else if (const auto* funcDecl = dynamic_cast<const FunctionDeclaration*>(&decl)) {
        return analyzeFunctionDeclaration(*funcDecl);
    }
    return std::nullopt;
}

std::optional<Error> SemanticAnalyzer::analyzeClassDeclaration(const ClassDeclaration& classDecl)
{
    this->symbolTable.enterScope(Format("class {0}", classDecl.name));
    this->symbolTable.add("self", classDecl.name);
    auto parents = this->classTable.getInheritMap(classDecl.name);
    for (const auto& parentClass : *parents) {
        for (const auto& parentMember : parentClass->members) {
            if (const auto property = dynamic_cast<const PropertyMember*>(parentMember.get())) {
                this->symbolTable.add(property->getName(), property->type->getName());
            }
        }
    }
    for (const auto& member : classDecl.members) {
        if (const auto property = dynamic_cast<const PropertyMember*>(member.get())) {
            this->symbolTable.add(property->getName(), property->type->getName());
        }
        else if (const auto method = dynamic_cast<const MethodMember*>(member.get())) {
            this->symbolTable.enterScope(
                Format("class {0} method {1}", classDecl.name, method->getName()));

            for (const auto& param : method->function->parameters) {
                this->symbolTable.add(param.name, param.type->getName());
            }

            auto error     = analyzeStatement(*method->function->body);
            auto [type, l] = this->currentFunctionReturnTypes.top();
            if (error) return error;
            if (this->currentFunctionReturnTypes.empty()) {
                if (method->function->returnType->getName() != "void") {
                    return Error(Format("You need return something in {0}.{1}",
                                        classDecl.name,
                                        method->getName()),
                                 method->getLocation());
                }
            }
            else {
                while (!this->currentFunctionReturnTypes.empty()) {
                    auto [type, returnLocation] = this->currentFunctionReturnTypes.top();
                    if (!this->classTable.checkInherit(type.getName(),
                                                       method->function->returnType->getName())) {
                        return Error(Format("The return statment of your {0}.{1} is "
                                            "different from the declared type!",
                                            classDecl.name,
                                            method->getName()),
                                     returnLocation);
                    }
                    this->currentFunctionReturnTypes.pop();
                }
            }
            this->symbolTable.exitScope();
        }
    }
    this->symbolTable.exitScope();
    return std::nullopt;
}

std::optional<Error> SemanticAnalyzer::analyzeEnumDeclaration(const EnumDeclaration& decl)
{
    return std::nullopt;
}

std::optional<Error> SemanticAnalyzer::analyzeFunctionDeclaration(const FunctionDeclaration& decl)
{
    return std::nullopt;
}
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

    this->symbolTable.enterScope("for-loop");
    // TODO: get variable type from iterableType
    this->symbolTable.add(stmt.variable, "int");

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
        return Error("The type in your If condition is not BOOL", stmt.condition->getLocation());
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
    auto [type, l] = this->currentFunctionReturnTypes.top();
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
            return Error(Format("The initialization statement type and declaration type of the "
                                "{0} do not match",
                                stmt.name),
                         stmt.getLocation());
        }
        this->symbolTable.add(stmt.name, stmt.type->getName());
    }
    else if (initType) {
        this->symbolTable.add(stmt.name, initType->getName());
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
            return Error("The case type in the when statement does not match the declaration type",
                         caseItem.value->getLocation());
        }
        this->symbolTable.enterScope("when-case");
        auto errorBody = analyzeStatement(*caseItem.body);
        if (errorBody) return errorBody;
        this->symbolTable.exitScope();
    }
    return std::nullopt;
}

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
        return analyzeCallExpression(*callExpr);
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
SemanticAnalyzer::analyzeCallExpression(const CallExpression& expr)
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


std::pair<std::unique_ptr<Program>, std::optional<Error>> SemanticAnalyzer::analyze()
{
    bool mainFlag = false;
    for (const auto& decl : program->declarations) {
        if (const auto funcDecl = dynamic_cast<const FunctionDeclaration*>(decl.get())) {
            if (funcDecl->name == "main") {
                mainFlag = true;
            }
        }
        else if (const auto classDecl = dynamic_cast<const ClassDeclaration*>(decl.get())) {
            if (this->classTable.find(classDecl->name)) {
                return {nullptr,
                        Error(Format("Class {0} has been defined!", classDecl->name),
                              classDecl->getLocation())};
            }
            this->classTable.add(classDecl->name, classDecl);
        }
    }

    if (!mainFlag) {
        return {nullptr, Error("Your program is missing the Main function!!!")};
    }

    for (const auto& decl : program->declarations) {
        if (const auto funcDecl = dynamic_cast<const FunctionDeclaration*>(decl.get())) {
            this->functionTable.add(funcDecl->name, funcDecl);
            this->symbolTable.add(funcDecl->name, funcDecl->name, SymbolType::SymbolKind::FUNC);
        }
        else if (const auto classDecl = dynamic_cast<const ClassDeclaration*>(decl.get())) {
            std::vector<const ClassDeclaration*> parents;
            std::string                          currParent = classDecl->baseClass;
            while (1) {
                if (currParent == classDecl->name) {
                    return {nullptr,
                            Error(Format("There is an inheritance cycle about Class {0}!",
                                         classDecl->name),
                                  classDecl->getLocation())};
                }
                else if (currParent.empty()) {
                    break;
                }
                else {
                    auto decl = this->classTable.find(currParent);
                    if (decl == nullptr) {
                        return {nullptr,
                                Error(Format("Your Class {0} inherits an undefined Class !",
                                             classDecl->name),
                                      classDecl->getLocation())};
                    }
                    else {
                        parents.push_back(decl);
                        currParent = decl->baseClass;
                    }
                }
            }
            this->classTable.addInheritMap(classDecl->name, std::move(parents));
        }
    }

    // cat -> Dog -> animal
    for (const auto& decl : program->declarations) {
        if (const auto classDecl = dynamic_cast<const ClassDeclaration*>(decl.get())) {
            auto parents = this->classTable.getInheritMap(classDecl->name);
            if (!parents || parents->empty()) {
                continue;
            }

            for (const auto& member : classDecl->members) {
                for (const auto& parentClass : *parents) {
                    const ClassMember* parentMember = parentClass->containMember(member.get());
                    if (!parentMember) {
                        continue;
                    }

                    if (const auto method = dynamic_cast<const MethodMember*>(member.get())) {
                        if (auto error = validateMethodOverride(
                                method, parentMember, classDecl, parentClass)) {
                            return {nullptr, *error};
                        }
                    }
                    else if (const auto property =
                                 dynamic_cast<const PropertyMember*>(member.get())) {
                        if (auto error = validatePropertyOverride(
                                property, parentMember, classDecl, parentClass)) {
                            return {nullptr, *error};
                        }
                    }
                }
            }
        }
    }

    for (const auto& decl : program->declarations) {
        auto errorDecl = analyzeDeclaration(*decl);
        if (errorDecl) return {nullptr, errorDecl};
    }
    return {std::move(program), std::nullopt};
}

std::optional<Error> SemanticAnalyzer::validateMethodOverride(const MethodMember*     method,
                                                              const ClassMember*      parentMember,
                                                              const ClassDeclaration* classDecl,
                                                              const ClassDeclaration* parentClass)
{
    const auto parentMethod = dynamic_cast<const MethodMember*>(parentMember);
    if (!parentMethod) {
        return std::nullopt;
    }

    if (!method->function->checkParam(parentMethod->function.get())) {
        return Error(Format("An error occurred in the parameter type of the method <{0}> "
                            "overridden by Class {1}!",
                            method->getName(),
                            classDecl->name),
                     method->getLocation());
    }

    if (!method->function->checkReturnType(parentMethod->function.get())) {
        return Error(Format("An error occurred in the return type of the method <{0}> "
                            "overridden by Class {1}!",
                            method->getName(),
                            classDecl->name),
                     method->getLocation());
    }

    return std::nullopt;
}

std::optional<Error> SemanticAnalyzer::validatePropertyOverride(const PropertyMember* property,
                                                                const ClassMember*    parentMember,
                                                                const ClassDeclaration* classDecl,
                                                                const ClassDeclaration* parentClass)
{
    const auto parentProperty = dynamic_cast<const PropertyMember*>(parentMember);
    if (!parentProperty) {
        return std::nullopt;
    }

    if (parentProperty->getName() == property->getName()) {
        return Error(Format("You cannot define the same property <{0}> in the subclass {1} "
                            "as the superclass {2}",
                            property->getName(),
                            classDecl->name,
                            parentClass->name),
                     property->getLocation());
    }

    return std::nullopt;
}