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
    this->symbolTable.add("self", classDecl.name, SymbolType::SymbolKind::VAL);
    for (const auto& constructorParam : classDecl.constructorParameters) {
        this->symbolTable.add(constructorParam.name, constructorParam.type->getName());
    }
    if (!classDecl.baseClass.empty()) {
        auto parent = this->classTable.find(classDecl.baseClass);
        if (classDecl.baseConstructorArgs.size() != parent->constructorParameters.size()) {
            return Error(
                Format("Base class '{0}' constructor expects {1} arguments, but {2} provided",
                       classDecl.baseClass,
                       parent->constructorParameters.size(),
                       classDecl.baseConstructorArgs.size()),
                classDecl.getLocation());
        }
        for (int i = 0; i < classDecl.baseConstructorArgs.size(); i++) {
            const auto& baseArg         = classDecl.baseConstructorArgs[i];
            const auto& parentParam     = parent->constructorParameters[i];
            auto [baseArgType, exprErr] = analyzeExpression(*baseArg);
            if (exprErr) return exprErr;
            if (!this->classTable.checkInherit(baseArgType->getName(),
                                               parentParam.type->getName())) {
                return Error(Format("Cannot convert argument {0} from '{1}' to '{2}' in base class "
                                    "'{3}' constructor",
                                    i + 1,
                                    baseArgType->getName(),
                                    parentParam.type->getName(),
                                    classDecl.baseClass),
                             baseArg->getLocation());
            }
        }
    }
    auto parents = this->classTable.getInheritMap(classDecl.name);
    for (const auto& parentClass : *parents) {
        for (const auto& parentMember : parentClass->members) {
            if (const auto property = dynamic_cast<const PropertyMember*>(parentMember.get())) {
                this->symbolTable.add(property->getName(),
                                      property->type->getName(),
                                      property->immutable);
            }
        }
    }
    for (const auto& member : classDecl.members) {
        if (const auto property = dynamic_cast<const PropertyMember*>(member.get())) {
            this->symbolTable.add(property->getName(), property->type->getName(),property->immutable);
        }
        else if (const auto method = dynamic_cast<const MethodMember*>(member.get())) {
            auto functionDeclErr = analyzeFunctionDeclaration(*method->function);
            if (functionDeclErr) return functionDeclErr;
        }
        else if (const auto init = dynamic_cast<const InitBlockMember*>(member.get())) {
            auto initBlockErr = analyzeBlockStatement(*init->block);
            if (initBlockErr) return initBlockErr;
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
    this->symbolTable.enterScope(Format("function {0}", decl.name));
    for (const auto& param : decl.parameters) {
        this->symbolTable.add(param.name, param.type->getName());
    }
    if (decl.body) {
        auto error = analyzeStatement(*decl.body);
        if (error) return error;
        if (this->currentFunctionReturnTypes.empty()) {
            if (decl.returnType->getName() != "void") {
                return Error(Format("Function '{0}' with return type '{1}' has no return statement",
                                    decl.name,
                                    decl.returnType->getName()),
                             decl.getLocation());
            }
        }
        else {
            auto [type, l] = this->currentFunctionReturnTypes.top();
            while (!this->currentFunctionReturnTypes.empty()) {
                auto [type, returnLocation] = this->currentFunctionReturnTypes.top();
                if (!decl.returnType) {}
                else {
                    if (!this->classTable.checkInherit(type.getName(),
                                                       decl.returnType->getName())) {
                        return Error(
                            Format(
                                "Return type mismatch in function '{0}': expected '{1}', got '{2}'",
                                decl.name,
                                decl.returnType->getName(),
                                type.getName()),
                            returnLocation);
                    }
                }
                this->currentFunctionReturnTypes.pop();
            }
        }
    }
    this->symbolTable.exitScope();
    return std::nullopt;
}