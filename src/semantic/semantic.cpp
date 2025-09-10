#include "../include/semantic/semantic.hpp"

#include "../include/utils/builtin.hpp"
#include "../include/utils/format.hpp"

void Scope::add(const std::string& key, Type type, SymbolKind kind)
{
    this->map[key] = {type, kind};
}

const Type* Scope::findType(const std::string& key)
{
    auto it = map.find(key);
    if (it != map.end()) {
        return &it->second.first;
    }
    return nullptr;
}

const SymbolKind* Scope::findKind(const std::string& key)
{
    auto it = map.find(key);
    if (it != map.end()) {
        return &it->second.second;
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

const Type* SymbolTable::findType(const std::string& key)
{
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        const Type* type = it->findType(key);
        if (type) {
            return type;
        }
    }
    return nullptr;
}

const SymbolKind* SymbolTable::findKind(const std::string& key)
{
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        const SymbolKind* kind = it->findKind(key);
        if (kind) {
            return kind;
        }
    }
    return nullptr;
}

void SymbolTable::add(const std::string& key, const Type& type, SymbolKind kind)
{
    if (!scopes.empty()) {
        scopes.back().add(key, Type(type), kind);
    }
}
void SymbolTable::add(const std::string& key, const Type& type, bool immutable)
{
    if (!scopes.empty()) {
        scopes.back().add(key, Type(type), immutable ? SymbolKind::VAL : SymbolKind::VAR);
    }
}

void SymbolTable::debug() const
{
    std::cout << "SymbolTable Debug Information:\n";
    for (size_t i = 0; i < scopes.size(); ++i) {
        std::cout << "Scope " << i << " " << scopes[i].getName() << ":\n";
        for (const auto& pair : scopes[i].getMap()) {
            std::cout << "  Key: " << pair.first << ", Symbol Type: " << pair.second.first.getName()
                      << "\n";
        }
    }
    std::cout << "\n";
}

void ClassTable::add(const std::string& className, const ClassDeclaration* classDecl)
{
    classes.insert(std::pair<std::string, const ClassDeclaration*>(className, classDecl));
    classIterableMap[className] = {false, Type()};
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

const std::vector<const ClassDeclaration*>* ClassTable::getInheritMap(
    const std::string& className) const
{
    auto iter = inheritMap.find(className);
    return iter != inheritMap.end() ? &(iter->second) : nullptr;
}

bool ClassTable::checkInherit(const std::string& child, const std::string& parent) const
{
    if (child == parent) return true;
    const auto* inheritMap = getInheritMap(child);
    if (inheritMap == nullptr) return false;
    for (const auto* inherit : *inheritMap) {
        if (inherit->name == parent) return true;
    }
    return false;
}

void ClassTable::setClassIterableMap(const std::string& className, bool iterable, const Type& type)
{
    classIterableMap[className] = {iterable, type};
}

const std::pair<bool, Type>* ClassTable::isClassIterable(const std::string& className) const
{
    auto it = classIterableMap.find(className);
    if (it != classIterableMap.end() && it->second.first == true) {
        return &it->second;
    }
    else {
        return nullptr;
    }
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

std::pair<std::unique_ptr<Program>, std::optional<Error>> SemanticAnalyzer::analyze()
{
    bool mainFlag = false;
    this->symbolTable.enterScope("global");
    for (const auto& decl : program->declarations) {
        if (const auto funcDecl = dynamic_cast<const FunctionDeclaration*>(decl.get())) {
            if (funcDecl->name == "main") {
                mainFlag = true;
            }
        }
        else if (const auto classDecl = dynamic_cast<const ClassDeclaration*>(decl.get())) {
            if (this->classTable.find(classDecl->name)) {
                return {nullptr,
                        Error(Format("Class '{0}' is already defined", classDecl->name),
                              classDecl->getLocation())};
            }
            this->classTable.add(classDecl->name, classDecl);
            this->symbolTable.add(
                classDecl->name, Type::classType(classDecl->name), SymbolKind::CLASS);
        }
    }
    if (!mainFlag) {
        return {nullptr, Error("Program requires a 'main' function")};
    }

    for (auto& decl : program->declarations) {
        if (auto classDecl = dynamic_cast<ClassDeclaration*>(decl.get())) {
            if (classDecl->baseClass.empty() && classDecl->name != BUILTIN::BUILTIN_CLASS[0]) {
                classDecl->baseClass = BUILTIN::BUILTIN_CLASS[0];
            }
        }
    }

    // check inheritance
    for (const auto& decl : program->declarations) {
        if (const auto funcDecl = dynamic_cast<const FunctionDeclaration*>(decl.get())) {
            this->functionTable.add(funcDecl->name, funcDecl);
            this->symbolTable.add(
                funcDecl->name, Type::functionType(funcDecl->name), SymbolKind::FUNC);
        }
        else if (const auto classDecl = dynamic_cast<const ClassDeclaration*>(decl.get())) {
            std::vector<const ClassDeclaration*> parents;
            std::string                          currParent = classDecl->baseClass;
            while (1) {
                if (currParent == classDecl->name) {
                    return {nullptr,
                            Error(Format("Circular inheritance detected in class '{0}'",
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
                                Error(Format("Class '{0}' inherits from undefined class '{1}'",
                                             classDecl->name,
                                             currParent),
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
            for (const auto& member : classDecl->members) {
                if (const auto property = dynamic_cast<const PropertyMember*>(member.get())) {
                    if (auto error = checkPropertyConstructorConflict(property, classDecl)) {
                        return {nullptr, *error};
                    }
                }
            }
            auto checkOpErr = checkClassOperator(classDecl);
            if (checkOpErr) return {nullptr, checkOpErr};
            auto parents = this->classTable.getInheritMap(classDecl->name);
            if (!parents || parents->empty()) {
                continue;
            }
            for (const auto& member : classDecl->members) {
                for (const auto& parentClass : *parents) {
                    const ClassMember* parentMember = parentClass->containMember(member.get());
                    if (const auto method = dynamic_cast<const MethodMember*>(member.get())) {
                        if (auto error = validateMethodOverride(
                                method, parentMember, classDecl, parentClass)) {
                            return {nullptr, *error};
                        }
                    }
                    else if (const auto property =
                                 dynamic_cast<const PropertyMember*>(member.get())) {
                        if (auto error = checkPropertyConstructorConflict(property, parentClass)) {
                            return {nullptr, *error};
                        }
                        if (auto error = validatePropertyOverride(
                                property, parentMember, classDecl, parentClass)) {
                            return {nullptr, *error};
                        }
                    }
                }
            }
        }
    }

    for (auto& decl : program->declarations) {
        auto errorDecl = analyzeDeclaration(*decl);
        if (errorDecl) return {nullptr, errorDecl};
    }
    this->symbolTable.exitScope();
    return {std::move(program), std::nullopt};
}

std::optional<Error> SemanticAnalyzer::validateMethodOverride(const MethodMember*     method,
                                                              const ClassMember*      parentMember,
                                                              const ClassDeclaration* classDecl,
                                                              const ClassDeclaration* parentClass)
{
    if (!parentMember) return std::nullopt;
    const auto parentMethod = dynamic_cast<const MethodMember*>(parentMember);
    if (!parentMethod) {
        return Error(
            Format("Type mismatch: '{0}' is a property in parent class but a method in class '{1}'",
                   method->getName(),
                   classDecl->name),
            method->getLocation());
    }
    if (method->function->isOperator != parentMethod->function->isOperator) {
        return Error(Format("Operator status mismatch: '{0}::{1}' cannot override parent method in "
                            "class '{2}' with different operator status",
                            classDecl->name,
                            method->getName(),
                            parentClass->name),
                     method->getLocation());
    }
    if (!method->function->checkParam(parentMethod->function.get())) {
        return Error(Format("Method override error: parameter types in '{0}::{1}' don't match "
                            "parent class '{2}'",
                            classDecl->name,
                            method->getName(),
                            parentClass->name),
                     method->getLocation());
    }

    if (!method->function->checkReturnType(parentMethod->function.get())) {
        return Error(
            Format(
                "Method override error: return type of '{0}::{1}' doesn't match parent class '{2}'",
                classDecl->name,
                method->getName(),
                parentClass->name),
            method->getLocation());
    }

    return std::nullopt;
}

std::optional<Error> SemanticAnalyzer::validatePropertyOverride(const PropertyMember* property,
                                                                const ClassMember*    parentMember,
                                                                const ClassDeclaration* classDecl,
                                                                const ClassDeclaration* parentClass)
{
    if (!parentMember) return std::nullopt;
    const auto parentProperty = dynamic_cast<const PropertyMember*>(parentMember);
    if (!parentProperty) {
        return Error(
            Format("Type mismatch: '{0}' is a method in parent class but a property in class '{1}'",
                   property->getName(),
                   classDecl->name),
            property->getLocation());
    }

    if (parentProperty->getName() == property->getName()) {
        return Error(Format("Property '{0}' in class '{1}' cannot override property with same name "
                            "from parent class '{2}'",
                            property->getName(),
                            classDecl->name,
                            parentClass->name),
                     property->getLocation());
    }


    return std::nullopt;
}

std::optional<Error> SemanticAnalyzer::checkClassOperator(const ClassDeclaration* classDecl)
{
    bool hasFirst   = false;
    bool hasNext    = false;
    bool hasEnd     = false;
    bool hasCurrent = false;
    Type firstReturnType;
    Type nextReturnType;
    Type endReturnType;
    Type currentReturnType;

    for (const auto& member : classDecl->members) {
        if (const auto method = dynamic_cast<const MethodMember*>(member.get())) {
            if (method->function->isOperator) {
                if (method->function->name == "_first") {
                    hasFirst        = true;
                    firstReturnType = *method->function->returnType;
                }
                else if (method->function->name == "_next") {
                    hasNext        = true;
                    nextReturnType = *method->function->returnType;
                }
                else if (method->function->name == "_end") {
                    hasEnd        = true;
                    endReturnType = *method->function->returnType;
                }
                else if (method->function->name == "_current") {
                    hasCurrent        = true;
                    currentReturnType = *method->function->returnType;
                }
            }
        }
    }

    if (!hasFirst && !hasNext && !hasEnd && !hasCurrent) {
        return std::nullopt;
    }

    if (!hasEnd) {
        return Error(
            Format("Class '{0}' implements iterator operators but is missing '_end' operator",
                   classDecl->name),
            classDecl->getLocation());
    }

    if (!hasFirst) {
        return Error(
            Format("Class '{0}' implements iterator operators but is missing '_first' operator",
                   classDecl->name),
            classDecl->getLocation());
    }

    if (!hasNext) {
        return Error(
            Format("Class '{0}' implements iterator operators but is missing '_next' operator",
                   classDecl->name),
            classDecl->getLocation());
    }
    if (!hasCurrent) {
        return Error(
            Format("Class '{0}' implements iterator operators but is missing '_current' operator",
                   classDecl->name),
            classDecl->getLocation());
    }
    if (!firstReturnType.isVoid() || !nextReturnType.isVoid()) {
        return Error(Format("Iterator operators '_first' and '_next' in class '{0}' must have "
                            "'void' return type",
                            classDecl->name),
                     classDecl->getLocation());
    }
    if (!endReturnType.isBool()) {
        return Error(
            Format("Iterator operator '_end' in class '{0}' must return bool, but returns '{1}'",
                   classDecl->name,
                   endReturnType.getName()),
            classDecl->getLocation());
    }
    this->classTable.setClassIterableMap(classDecl->name, true, currentReturnType);
    return std::nullopt;
}

std::optional<Error> SemanticAnalyzer::checkPropertyConstructorConflict(
    const PropertyMember* property, const ClassDeclaration* classDecl)
{
    for (const auto& constructorParameter : classDecl->constructorParameters) {
        if (constructorParameter.name == property->getName()) {
            return Error(Format("Property name '{0}' conflicts with "
                                "constructor parameter in class '{1}'",
                                property->getName(),
                                classDecl->name),
                         property->getLocation());
        }
    }

    return std::nullopt;
}