#include "../include/ir/ir.hpp"
#include "../include/utils/format.hpp"

void IRGen::generateDeclaration(const Declaration& decl)
{
    if (const auto* classDecl = dynamic_cast<const ClassDeclaration*>(&decl)) {
        generateClassDeclaration(*classDecl);
    }
    else if (const auto* enumDecl = dynamic_cast<const EnumDeclaration*>(&decl)) {
        generateEnumDeclaration(*enumDecl);
    }
    else if (const auto* funcDecl = dynamic_cast<const FunctionDeclaration*>(&decl)) {
        generateFunctionDeclaration(*funcDecl);
    }
}

void IRGen::generateClassDeclaration(const ClassDeclaration& decl)
{
    // for()
    // decl.baseConstructorArgs
    // decl.constructorParameters
    for (const auto& baseArgs : decl.baseConstructorArgs) {
        // baseArgs->
    }
}
void IRGen::generateEnumDeclaration(const EnumDeclaration& decl)
{
    throw "Not yet implemented generateEnumDeclaration";
}
void IRGen::generateFunctionDeclaration(const FunctionDeclaration& decl) {}
