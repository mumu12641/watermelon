#include "../include/semantic/semantic.hpp"
#include "../include/utils/format.hpp"

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
            if (method->function->body) {
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
                        if (!method->function->returnType) {}
                        else {
                            if (!this->classTable.checkInherit(
                                    type.getName(), method->function->returnType->getName())) {
                                return Error(Format("The return statment of your {0}.{1} is "
                                                    "different from the declared type!",
                                                    classDecl.name,
                                                    method->getName()),
                                             returnLocation);
                            }
                        }
                        this->currentFunctionReturnTypes.pop();
                    }
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