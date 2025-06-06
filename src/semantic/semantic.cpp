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

void SymbolTable::enter_scope()
{
    scopes.emplace_back(Scope());
}

void SymbolTable::exit_scope()
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

void SymbolTable::add(const std::string& key, SymbolType type)
{
    if (!scopes.empty()) {
        scopes.back().add(key, type);
    }
}

void ClassTable::add(const std::string& className, const ClassDeclaration* classDecl)
{
    // classes.emplace_back(className, classDecl);
    classes.insert(std::pair<std::string, const ClassDeclaration*>(className, classDecl));
}

const ClassDeclaration* ClassTable::find(const std::string& className)
{
    // for (const auto& i : classes) {
    //     if (i.first == className) {
    //         return i.second;
    //     }
    // }
    // return nullptr;
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

            if (classTable.find(classDecl->name)) {
                return {nullptr,
                        Error(Format("Class {0} has been defined!", classDecl->name),
                              classDecl->getLocation())};
            }
            classTable.add(classDecl->name, classDecl);
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
                    auto decl = classTable.find(currParent);
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
            classTable.addInheritMap(classDecl->name, std::move(parents));
        }
    }
    // Cat, Dog, Animal
    for (const auto& decl : program->declarations) {
        if (const auto classDecl = dynamic_cast<const ClassDeclaration*>(decl.get())) {
            auto parents = classTable.getInheritMap(classDecl->name);
            for (const auto& memeber : classDecl->members) {

                for (const auto& curr_parent : *parents) {
                    const ClassMember* parentMemeber = curr_parent->containMember(memeber.get());
                    if (parentMemeber != nullptr) {
                        if (const auto method = dynamic_cast<const MethodMember*>(memeber.get())) {
                            const FunctionDeclaration* parentMethod =
                                dynamic_cast<const MethodMember*>(parentMemeber)->function.get();
                            if (!method->function->checkParam(parentMethod)) {
                                return {nullptr,
                                        Error(Format("An error occurred in the parameter type of "
                                                     "the method <{0}> overridden by Class {1}!",
                                                     method->getName(),
                                                     classDecl->name),
                                              method->getLocation())};
                            }
                            if (!method->function->checkReturnType(parentMethod)) {
                                return {
                                    nullptr,
                                    Error(
                                        Format("An error occurred in the return type of the method "
                                               "<{0}> overridden by Class {1}!",
                                               method->getName(),
                                               classDecl->name),
                                        method->getLocation())};
                            }
                        }
                        else if (const auto property =
                                     dynamic_cast<const PropertyMember*>(memeber.get())) {
                            const PropertyMember* parentProperty =
                                dynamic_cast<const PropertyMember*>(parentMemeber);
                            if (parentProperty->getName() == property->getName()) {
                                return {
                                    nullptr,
                                    Error(Format("You cannot define the same property <{0}> in the "
                                                 "subclass {1} as the superclass {2}",
                                                 property->getName(),
                                                 classDecl->name,
                                                 curr_parent->name),
                                          property->getLocation())};
                            }
                        }
                    }
                }
            }
        }
    }
    return {std::move(program), std::nullopt};
}