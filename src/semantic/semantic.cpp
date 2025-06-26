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
                        Error(Format("Class {0} has been defined!", classDecl->name),
                              classDecl->getLocation())};
            }
            this->classTable.add(classDecl->name, classDecl);
            this->symbolTable.add(classDecl->name, classDecl->name, SymbolType::SymbolKind::CLASS);
        }
    }
    if (!mainFlag) {
        return {nullptr, Error("Your program is missing the Main function!!!")};
    }

    for (const auto& decl : program->declarations) {
        if (const auto funcDecl = dynamic_cast<const FunctionDeclaration*>(decl.get())) {
            this->functionTable.add(funcDecl->name, funcDecl);
            // key = value = funcDecl -> name
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
    this->symbolTable.exitScope();
    return {std::move(program), std::nullopt};
}

std::optional<Error> SemanticAnalyzer::validateMethodOverride(const MethodMember*     method,
                                                              const ClassMember*      parentMember,
                                                              const ClassDeclaration* classDecl,
                                                              const ClassDeclaration* parentClass)
{
    const auto parentMethod = dynamic_cast<const MethodMember*>(parentMember);
    if (!parentMethod) {
        return Error(Format("Parent's {0} is a property, but {1}'s {2} is a method",
                            method->getName(),
                            classDecl->name,
                            method->getName()),
                     method->getLocation());
        ;
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
        return Error(Format("Parent's {0} is a method, but {1}'s {2} is a property",
                            property->getName(),
                            classDecl->name,
                            property->getName()),
                     property->getLocation());
        ;
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