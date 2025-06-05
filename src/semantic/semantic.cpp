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

void ClassTable::add(const std::string& className)
{
    classes.emplace_back(className);
}

const std::string* ClassTable::find(const std::string& className)
{
    for (const std::string& i : classes) {
        if (i == className) {
            return &i;
        }
    }
    return nullptr;
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
                              classDecl->get_location())};
            }
            classTable.add(classDecl->name);
        }
    }

    if (!mainFlag) {
        return {nullptr, Error("Your program is missing the Main function!!!")};
    }

    return {std::move(program), std::nullopt};
}