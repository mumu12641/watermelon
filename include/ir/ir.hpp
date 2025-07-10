#ifndef IR_HPP
#define IR_HPP

#include "../ast/ast.hpp"
#include "../lexer/token.hpp"
#include "../utils/error.hpp"

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/Host.h>
#include <map>
#include <memory>
#include <string_view>
#include <utility>
#include <vector>

class IRGen
{
private:
    llvm::LLVMContext        context;
    llvm::IRBuilder<>        builder;
    llvm::Module             module;
    std::unique_ptr<Program> program;

public:
    IRGen(std::unique_ptr<Program> p)
        : program(std::move(p))
        , builder(context)
        , module("test_module", context)
    {
        module.setTargetTriple(llvm::sys::getDefaultTargetTriple());
    }

    llvm::Type* generateType(Type* type);

    llvm::Module* generateIR();
    void          generateDeclaration(const Declaration& decl);
    void          generateClassDeclaration(const ClassDeclaration& decl);
    void          generateEnumDeclaration(const EnumDeclaration& decl);
    void          generateFunctionDeclaration(const FunctionDeclaration& decl);

    llvm::Value* generateStatement(const Statement& stmt);
    llvm::Value* generateBlockStatement(const BlockStatement& stmt);
    llvm::Value* generateExpressionStatement(const ExpressionStatement& stmt);
    llvm::Value* generateForStatement(const ForStatement& stmt);
    llvm::Value* generateIfStatement(const IfStatement& stmt);
    llvm::Value* generateReturnStatement(const ReturnStatement& stmt);
    llvm::Value* generateVariableStatement(const VariableStatement& stmt);
    llvm::Value* generateWhenStatement(const WhenStatement& stmt);

    llvm::Value* generateExpression(const Expression& expr);
    llvm::Value* generateArrayExpression(const ArrayExpression& expr);
    llvm::Value* generateBinaryExpression(const BinaryExpression& expr);
    llvm::Value* generateFunctionCallExpression(const CallExpression& expr);
    llvm::Value* generateIdentifierExpression(const IdentifierExpression& expr);
    llvm::Value* generateLambdaExpression(const LambdaExpression& expr);
    llvm::Value* generateLiteralExpression(const LiteralExpression& expr);
    llvm::Value* generateMemberExpression(const MemberExpression& expr);
    llvm::Value* generateTypeCheckExpression(const TypeCheckExpression& expr);
    llvm::Value* generateUnaryExpression(const UnaryExpression& expr);
};
#endif