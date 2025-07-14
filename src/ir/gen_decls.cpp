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
    this->valueTable.enterScope();
    this->currClass = &decl;
    for (const auto& member : decl.members) {
        if (const auto method = dynamic_cast<const PropertyMember*>(member.get())) {

        }
    }
    for (const auto& member : decl.members) {
        if (const auto method = dynamic_cast<const MethodMember*>(member.get())) {
            this->currFunc = method->function.get();
            auto entryBB   = llvm::BasicBlock::Create(*this->context, "entry", this->getCurrFunc());

            this->builder->SetInsertPoint(entryBB);

            this->generateStatement(*method->function->body);
        }
        else if (const auto init = dynamic_cast<const InitBlockMember*>(member.get())) {
            // this->generateStatement(*init->block);
        }
    }
    this->currClass = nullptr;
    this->valueTable.exitScope();
}
void IRGen::generateEnumDeclaration(const EnumDeclaration& decl)
{
    throw "Not yet implemented generateEnumDeclaration";
}
void IRGen::generateFunctionDeclaration(const FunctionDeclaration& decl) {}
