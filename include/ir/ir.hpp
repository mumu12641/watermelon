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
    std::unique_ptr<llvm::LLVMContext> context;   // 先声明 context
    std::unique_ptr<llvm::Module>      module;    // 再声明 module
    std::unique_ptr<llvm::IRBuilder<>> builder;   // 最后声明 builder

    //    llvm::Module                          _module;
    std::unique_ptr<Program>              program;
    std::unordered_map<Type, llvm::Type*> typeMap;

public:
    IRGen(std::unique_ptr<Program> p)
        : context(std::make_unique<llvm::LLVMContext>())
        , module(std::make_unique<llvm::Module>("test_module", *context))
        , builder(std::make_unique<llvm::IRBuilder<>>(*context))
        , program(std::move(p))
    {
        module->setTargetTriple(llvm::sys::getDefaultTargetTriple());
    }

    void        installType();
    llvm::Type* generateType(const Type& type);

    //    llvm::Module* generateIR();
    std::unique_ptr<llvm::Module> generateIR();
    void                          generateDeclaration(const Declaration& decl);
    void                          generateClassDeclaration(const ClassDeclaration& decl);
    void                          generateEnumDeclaration(const EnumDeclaration& decl);
    void                          generateFunctionDeclaration(const FunctionDeclaration& decl);

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