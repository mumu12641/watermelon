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

void SymbolTable::add(const std::string& key, const std::string& type)
{
    if (!scopes.empty()) {
        scopes.back().add(key, SymbolType(type));
    }
}

void SymbolTable::debug() const
{
    std::cout << "SymbolTable Debug Information:\n";
    for (size_t i = 0; i < scopes.size(); ++i) {
        std::cout << "Scope " << i << scopes[i].getName() << ":\n";
        for (const auto& pair : scopes[i].getMap()) {
            std::cout << "  Key: " << pair.first << ", Symbol Type: " << pair.second.getType()
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
    for (const auto* inherit : *getInheritMap(child)) {
        if (inherit->name == parent) return true;
    }
    return false;
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
    else if (dynamic_cast<const Declaration*>(&stmt)) {
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
        return Error(Format("The type in your If condition is not BOOL"),
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
    if (this->currentFunctionReturnTypes.empty()) {
        return Error("Return statement outside of function", stmt.getLocation());
    }

    if (stmt.value) {
        auto [actualReturnType, errorReturn] = analyzeExpression(*stmt.value);
        if (errorReturn) return errorReturn;
        this->currentFunctionReturnTypes.push({actualReturnType.get(), stmt.getLocation()});
    }
    return std::nullopt;
}
std::optional<Error> SemanticAnalyzer::analyzeVariableStatement(const VariableStatement& stmt)
{
    std::unique_ptr<Type> initType = nullptr;
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
            return Error(
                Format("The case type in the when statement does not match the declaration type"),
                caseItem.value->getLocation());
        }
        this->symbolTable.enterScope("when-case");
        auto errorBody = analyzeStatement(*caseItem.body);
        if (errorBody) return errorBody;
        this->symbolTable.exitScope();
    }
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
        if (const auto classDecl = dynamic_cast<const ClassDeclaration*>(decl.get())) {
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
        if (const auto classDecl = dynamic_cast<const ClassDeclaration*>(decl.get())) {
            this->symbolTable.enterScope(Format("class {0}", classDecl->name));
            this->symbolTable.add("self", classDecl->name);
            auto parents = this->classTable.getInheritMap(classDecl->name);
            for (const auto& parentClass : *parents) {
                for (const auto& parentMember : parentClass->members) {
                    if (const auto property =
                            dynamic_cast<const PropertyMember*>(parentMember.get())) {
                        this->symbolTable.add(property->getName(), property->type->getName());
                    }
                }
            }
            for (const auto& member : classDecl->members) {
                if (const auto property = dynamic_cast<const PropertyMember*>(member.get())) {
                    this->symbolTable.add(property->getName(), property->type->getName());
                }
                else if (const auto method = dynamic_cast<const MethodMember*>(member.get())) {
                    this->symbolTable.enterScope(
                        Format("class {0} method {1}", classDecl->name, method->getName()));

                    for (const auto& param : method->function->parameters) {
                        this->symbolTable.add(param.name, param.type->getName());
                    }
                    auto error = analyzeStatement(*method->function->body);
                    if (error) {
                        return {nullptr, *error};
                    }
                    if (this->currentFunctionReturnTypes.empty()) {
                        if (!method->function->returnType->isVoid()) {
                            return {nullptr,
                                    Error(Format("You need return something in {0}.{1}",
                                                 classDecl->name,
                                                 method->getName()),
                                          method->getLocation())};
                        }
                    }
                    else {
                        while (!this->currentFunctionReturnTypes.empty()) {
                            auto [type, returnLocation] = this->currentFunctionReturnTypes.top();
                            if (!this->classTable.checkInherit(
                                    type->getName(), method->function->returnType->getName())) {
                                return {nullptr,
                                        Error(Format("The return statment of your {0}.{1} is "
                                                     "different from the declared type!",
                                                     classDecl->name,
                                                     method->getName()),
                                              returnLocation)};
                            }
                            this->currentFunctionReturnTypes.pop();
                        }
                    }
                    this->symbolTable.debug();
                    this->symbolTable.exitScope();
                }
            }
            this->symbolTable.debug();
            this->symbolTable.exitScope();
        }
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