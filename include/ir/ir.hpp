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
#include <variant>
#include <vector>

enum class IRValueKind
{
    PROPERTY,
    FUNCTIONPARAM,
};

class IRValue
{
private:
    IRValueKind                     kind;
    std::variant<int, llvm::Value*> data;

public:
    IRValue(int offset)
        : kind(IRValueKind::PROPERTY)
        , data(offset)
    {
    }

    IRValue(llvm::Value* value)
        : kind(IRValueKind::FUNCTIONPARAM)
        , data(value)
    {
    }

    IRValueKind getKind() const { return kind; }

    int getOffset() const
    {
        assert(kind == IRValueKind::PROPERTY);
        return std::get<int>(data);
    }

    llvm::Value* getValue() const
    {
        assert(kind == IRValueKind::FUNCTIONPARAM);
        return std::get<llvm::Value*>(data);
    }
};

class IRValueScope
{
private:
    std::string                              name;
    std::unordered_map<std::string, IRValue> map;

public:
    IRValueScope(const std::string& s)
        : name(s)
    {
    }
    void                                            add(const std::string& key, IRValue value);
    const IRValue*                                  find(const std::string& key);
    const std::unordered_map<std::string, IRValue>& getMap() const { return map; }
    const std::string&                              getName() const { return name; }
};

class IRValueTable
{
private:
    std::vector<IRValueScope> scopes;

public:
    void           enterScope(const std::string& name = "");
    void           exitScope();
    void           add(const std::string& key, IRValue value);
    const IRValue* find(const std::string& key);
    void           debug() const;
};

class IRGen
{
private:
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::Module>      module;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    std::unique_ptr<llvm::DataLayout>  dataLayout;

    IRValueTable             valueTable;
    std::unique_ptr<Program> program;
    ClassTable               classTable;
    FunctionTable            functionTable;

    const ClassDeclaration* currClass;
    std::string             currFuncName;
    llvm::Instruction*      allocaInsertPoint = nullptr;
    llvm::Value*            retVal            = nullptr;
    llvm::BasicBlock*       retBB             = nullptr;

    std::unordered_map<Type, llvm::Type*> typeMap;
    std::unordered_map<std::string,
                       std::vector<std::variant<const FunctionParameter*, const PropertyMember*>>>
                                                           classAllParams;
    std::unordered_map<std::string, llvm::StructType*>     classTypes;
    std::unordered_map<std::string, llvm::StructType*>     vTableTypes;
    std::unordered_map<std::string, llvm::Function*>       methodMap;
    std::unordered_map<std::string, llvm::GlobalVariable*> vTableVars;

    llvm::Type* int32Ty;
    llvm::Type* int64Ty;
    llvm::Type* voidTy;
    llvm::Type* int8PtrTy;
    llvm::Type* boolTy;
    llvm::Type* floatTy;

public:
    IRGen(std::unique_ptr<Program> p, ClassTable&& classTable, FunctionTable&& functionTable)
        : context(std::make_unique<llvm::LLVMContext>())
        , module(std::make_unique<llvm::Module>("test_module", *context))
        , builder(std::make_unique<llvm::IRBuilder<>>(*context))
        , dataLayout(std::make_unique<llvm::DataLayout>(module.get()))
        , valueTable(IRValueTable())
        , currFuncName("")
        , program(std::move(p))
        , classTable(classTable)
        , functionTable(functionTable)
    {
        module->setTargetTriple(llvm::sys::getDefaultTargetTriple());
        int32Ty   = llvm::Type::getInt32Ty(*context);
        int64Ty   = llvm::Type::getInt64Ty(*context);
        voidTy    = llvm::Type::getVoidTy(*context);
        int8PtrTy = llvm::Type::getInt8PtrTy(*context);
        boolTy    = llvm::Type::getInt1Ty(*context);
        floatTy   = llvm::Type::getDoubleTy(*context);
    }

    /* setup methods */
    void declareClasses();
    void defineClasses();
    void buildVTables();
    void addVTableMethod(std::vector<llvm::Type*>&     vTableMethods,
                         std::vector<llvm::Constant*>& vTableInitializers,
                         const std::string& methodName, llvm::FunctionType* funcType);
    void setupClasses();
    void setupFunctions();

    /* utils methods */
    llvm::Type* generateType(const Type& type, bool ptr);
    llvm::Type* generateType(const std::string& type, bool ptr);

    llvm::Function* getCurrFunc()
    {
        return currClass == nullptr
                   ? this->module->getFunction(currFuncName)
                   : this->module->getFunction(Format("{0}_{1}", currClass->name, currFuncName));
    }
    llvm::Type* getParamType(
        const std::variant<const FunctionParameter*, const PropertyMember*>& param);
    std::string getParamName(
        const std::variant<const FunctionParameter*, const PropertyMember*>& param);
    const Expression* getParamInitExpr(
        const std::variant<const FunctionParameter*, const PropertyMember*>& param);

    llvm::AllocaInst* allocateStackVariable(const std::string_view identifier, llvm::Type* type);

    /* generate methods */
    std::unique_ptr<llvm::Module> generateIR();
    void                          generateDeclaration(const Declaration& decl);

    void generateClassDeclaration(const ClassDeclaration& decl);

    /* generate class init methods  */
    void generateClassBuiltinInit(const ClassDeclaration& decl);
    void generateClassConstructor(const ClassDeclaration& decl);
    void generateClassMallocInit(const ClassDeclaration& decl);
    void generateClassSelfDefinedInit(const InitBlockMember& init, const std::string& className);

    void generateEnumDeclaration(const EnumDeclaration& decl);
    void generateFunctionDeclaration(const FunctionDeclaration& decl);

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
    llvm::Value* generateCallExpression(const CallExpression& expr);
    llvm::Value* generateIdentifierExpression(const IdentifierExpression& expr);
    llvm::Value* generateLambdaExpression(const LambdaExpression& expr);
    llvm::Value* generateLiteralExpression(const LiteralExpression& expr);
    llvm::Value* generateMemberExpression(const MemberExpression& expr);
    llvm::Value* generateTypeCheckExpression(const TypeCheckExpression& expr);
    llvm::Value* generateUnaryExpression(const UnaryExpression& expr);
};
#endif