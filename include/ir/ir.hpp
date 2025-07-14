#ifndef IR_HPP
#define IR_HPP

#include "../ast/ast.hpp"
#include "../lexer/token.hpp"
#include "../semantic/semantic.hpp"
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

class IRScope
{
private:
    std::string                                   name;
    std::unordered_map<std::string, llvm::Value*> map;

public:
    IRScope(const std::string& s)
        : name(s)
    {
    }
    void               add(const std::string& key, llvm::Value* value);
    const llvm::Value* find(const std::string& key);
};

class IRValueTable
{
private:
    std::vector<IRScope> scopes;

public:
    void               enterScope(const std::string& name = "");
    void               exitScope();
    void               add(const std::string& key, llvm::Value* value);
    const llvm::Value* find(const std::string& key);
};

class IRGen
{
private:
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::Module>      module;
    std::unique_ptr<llvm::IRBuilder<>> builder;

    IRValueTable               valueTable;
    const ClassDeclaration*    currClass;
    const FunctionDeclaration* currFunc;
    std::unique_ptr<Program>   program;
    ClassTable                 classTable;

    std::unordered_map<Type, llvm::Type*>                  typeMap;
    std::unordered_map<std::string, llvm::StructType*>     classTypes;
    std::unordered_map<std::string, llvm::StructType*>     vTableTypes;
    std::unordered_map<std::string, llvm::Function*>       methodMap;
    std::unordered_map<std::string, llvm::GlobalVariable*> vTableVars;

public:
    IRGen(std::unique_ptr<Program> p, ClassTable classTable)
        : context(std::make_unique<llvm::LLVMContext>())
        , module(std::make_unique<llvm::Module>("test_module", *context))
        , builder(std::make_unique<llvm::IRBuilder<>>(*context))
        , valueTable(IRValueTable())
        , currFunc(nullptr)
        , currClass(nullptr)
        , program(std::move(p))
        , classTable(classTable)
    {
        module->setTargetTriple(llvm::sys::getDefaultTargetTriple());
    }

    void        declareClasses();
    void        defineClasses();
    void        buildVTables();
    void        setupClasses();
    llvm::Type* generateType(const Type& type, bool ptr = false);

    llvm::Function* getCurrFunc()
    {
        return currClass == nullptr ? methodMap[currFunc->name]
                                    : methodMap[Format("{0}_{1}", currClass->name, currFunc->name)];
    }

    //    llvm::Module* generateIR();
    std::unique_ptr<llvm::Module> generateIR();
    void                          generateDeclaration(const Declaration& decl);
    void                          generateClassDeclaration(const ClassDeclaration& decl);
    void                          generateEnumDeclaration(const EnumDeclaration& decl);
    void                          generateFunctionDeclaration(const FunctionDeclaration& decl);

    void generateStatement(const Statement& stmt);
    void generateBlockStatement(const BlockStatement& stmt);
    void generateExpressionStatement(const ExpressionStatement& stmt);
    void generateForStatement(const ForStatement& stmt);
    void generateIfStatement(const IfStatement& stmt);
    void generateReturnStatement(const ReturnStatement& stmt);
    void generateVariableStatement(const VariableStatement& stmt);
    void generateWhenStatement(const WhenStatement& stmt);

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