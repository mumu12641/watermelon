#include "../include/semantic/semantic.hpp"
#include "../include/utils/format.hpp"

std::optional<Error> SemanticAnalyzer::analyzeDeclaration(Declaration& decl)
{
    if (auto* classDecl = dynamic_cast<ClassDeclaration*>(&decl)) {
        return analyzeClassDeclaration(*classDecl);
    }
    else if (auto* enumDecl = dynamic_cast<EnumDeclaration*>(&decl)) {
        return analyzeEnumDeclaration(*enumDecl);
    }
    else if (auto* funcDecl = dynamic_cast<FunctionDeclaration*>(&decl)) {
        return analyzeFunctionDeclaration(*funcDecl);
    }
    return std::nullopt;
}

std::optional<Error> SemanticAnalyzer::analyzeClassDeclaration(ClassDeclaration& classDecl)
{
    this->symbolTable.enterScope(Format("class {0}", classDecl.name));
    this->symbolTable.add("self", Type::classType(classDecl.name), SymbolKind::VAL);

    // parents' all construtor param
    if (!classDecl.baseClass.empty()) {
        auto parents = this->classTable.getInheritMap(classDecl.name);
        for (auto it = parents->rbegin(); it != parents->rend(); ++it) {
            for (const auto& constructorParam : (*it)->constructorParameters) {
                this->symbolTable.add(constructorParam.name, *constructorParam.type);
                this->symbolTable.add(Format("self_{1}", constructorParam.name),
                                      *constructorParam.type);
            }
        }
    }
    // self's construtor param override
    bool hasDefaultParam = false;
    for (const auto& constructorParam : classDecl.constructorParameters) {
        if (hasDefaultParam && constructorParam.defaultValue == nullptr) {
            return Error(Format("Parameter '{0}' without default value follows parameter with "
                                "default value in class '{1}' constructor",
                                constructorParam.name,
                                classDecl.name),
                         classDecl.getLocation());
        }
        if (constructorParam.defaultValue != nullptr) {
            hasDefaultParam = true;
        }
        this->symbolTable.add(constructorParam.name, *constructorParam.type);
        this->symbolTable.add(Format("self_{1}", constructorParam.name), *constructorParam.type);
    }
    if (!classDecl.baseClass.empty()) {
        auto parent             = this->classTable.find(classDecl.baseClass);
        int  requiredParamCount = 0;
        for (const auto& param : parent->constructorParameters) {
            if (param.defaultValue == nullptr) requiredParamCount++;
        }
        if (classDecl.baseConstructorArgs.size() < requiredParamCount) {
            return Error(Format("Base class '{0}' constructor requires at least {1} arguments, but "
                                "only {2} provided",
                                parent->name,
                                requiredParamCount,
                                classDecl.baseConstructorArgs.size()),
                         classDecl.getLocation());
        }

        if (classDecl.baseConstructorArgs.size() > parent->constructorParameters.size()) {
            return Error(
                Format(
                    "Base class '{0}' constructor accepts at most {1} arguments, but {2} provided",
                    parent->name,
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
                this->symbolTable.add(property->getName(), *property->type, property->immutable);
                this->symbolTable.add(
                    Format("self_{0}", property->getName()), *property->type, property->immutable);
            }
        }
    }
    for (const auto& member : classDecl.members) {
        if (const auto property = dynamic_cast<const PropertyMember*>(member.get())) {
            this->symbolTable.add(property->getName(), *property->type, property->immutable);
            this->symbolTable.add(
                Format("self_{0}", property->getName()), *property->type, property->immutable);
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

std::optional<Error> SemanticAnalyzer::analyzeEnumDeclaration(EnumDeclaration& decl)
{
    throw "Not yet implemented SemanticAnalyzer::analyzeEnumDeclaration";
}

std::optional<Error> SemanticAnalyzer::analyzeFunctionDeclaration(FunctionDeclaration& decl)
{
    this->symbolTable.enterScope(Format("function {0}", decl.name));
    bool hasDefaultParam = false;
    for (const auto& param : decl.parameters) {
        if (hasDefaultParam && param.defaultValue == nullptr) {
            return Error(Format("Parameter '{0}' without default value follows parameter with "
                                "default value in function '{1}'",
                                param.name,
                                decl.name),
                         decl.getLocation());
        }
        if (param.defaultValue != nullptr) {
            hasDefaultParam = true;
        }
        this->symbolTable.add(param.name, *param.type);
    }
    if (decl.body) {
        auto error = analyzeStatement(*decl.body);
        if (error) return error;
        if (this->currentFunctionReturnTypes.empty()) {
            if (!decl.returnType->isVoid()) {
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