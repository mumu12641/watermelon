
#ifndef SEMANTIC_HPP
#define SEMANTIC_HPP

#include "../ast/ast.hpp"
#include "../lexer/token.hpp"
#include "../utils/error.hpp"

#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

enum class SymbolKind
{
    VAR,
    VAL,
    FUNC,
    CLASS
};

class Scope
{
private:
    std::string                                                  name;
    std::unordered_map<std::string, std::pair<Type, SymbolKind>> map;

public:
    Scope(const std::string s)
        : name(s)
    {
    }
    void              add(const std::string& key, Type type, SymbolKind kind);
    const Type*       findType(const std::string& key);
    const SymbolKind* findKind(const std::string& key);
    const std::unordered_map<std::string, std::pair<Type, SymbolKind>>& getMap() const
    {
        return map;
    }
    const std::string& getName() const { return name; }
};

class SymbolTable
{
private:
    std::vector<Scope> scopes;

public:
    void              enterScope(const std::string& name);
    void              exitScope();
    const Type*       findType(const std::string& key);
    const SymbolKind* findKind(const std::string& key);
    void              add(const std::string& key, const Type& type, bool immutable);
    void add(const std::string& key, const Type& type, SymbolKind kind = SymbolKind::VAR);
    void debug() const;
};

class ClassTable
{
private:
    std::unordered_map<std::string, const ClassDeclaration*>              classes;
    std::unordered_map<std::string, std::vector<const ClassDeclaration*>> inheritMap;
    std::unordered_map<std::string, std::pair<bool, Type>>                classIterableMap;

public:
    void                    add(const std::string& className, const ClassDeclaration*);
    const ClassDeclaration* find(const std::string& className);

    const std::vector<const ClassDeclaration*>* getInheritMap(const std::string& className) const;
    const std::pair<bool, Type>*                isClassIterable(const std::string& className) const;

    void addInheritMap(const std::string& className, std::vector<const ClassDeclaration*> parents);
    bool checkInherit(const std::string& child, const std::string& parent) const;
    void setClassIterableMap(const std::string& className, bool iterable, const Type& type);
};

class FunctionTable
{
private:
    std::unordered_map<std::string, const FunctionDeclaration*> functions;

public:
    void                       add(const std::string& className, const FunctionDeclaration*);
    const FunctionDeclaration* find(const std::string& className);
};

class SemanticAnalyzer
{
private:
    SymbolTable                           symbolTable;
    ClassTable                            classTable;
    FunctionTable                         functionTable;
    std::unique_ptr<Program>              program;
    std::stack<std::pair<Type, Location>> currentFunctionReturnTypes;

public:
    SemanticAnalyzer(std::unique_ptr<Program> p)
        : program(std::move(p))
        , symbolTable(SymbolTable())
        , classTable(ClassTable())
        , functionTable(FunctionTable())
    {
    }
    ClassTable    getClassTable() { return classTable; }
    FunctionTable getFunctionTable() { return functionTable; }

    std::optional<Error> validateMethodOverride(const MethodMember*     method,
                                                const ClassMember*      parentMember,
                                                const ClassDeclaration* classDecl,
                                                const ClassDeclaration* parentClass);
    std::optional<Error> validatePropertyOverride(const PropertyMember*   property,
                                                  const ClassMember*      parentMember,
                                                  const ClassDeclaration* classDecl,
                                                  const ClassDeclaration* parentClass);
    std::optional<Error> checkPropertyConstructorConflict(const PropertyMember*   property,
                                                          const ClassDeclaration* classDecl);
    void                 checkClassOperator(const ClassDeclaration* classDecl);

    std::pair<std::unique_ptr<Program>, std::optional<Error>> analyze();
    std::optional<Error>                                      analyzeDeclaration(Declaration& decl);
    std::optional<Error> analyzeClassDeclaration(ClassDeclaration& decl);
    std::optional<Error> analyzeEnumDeclaration(EnumDeclaration& decl);
    std::optional<Error> analyzeFunctionDeclaration(FunctionDeclaration& decl);

    std::optional<Error> analyzeStatement(Statement& stmt);
    std::optional<Error> analyzeBlockStatement(BlockStatement& stmt);
    std::optional<Error> analyzeExpressionStatement(ExpressionStatement& stmt);
    std::optional<Error> analyzeForStatement(ForStatement& stmt);
    std::optional<Error> analyzeIfStatement(IfStatement& stmt);
    std::optional<Error> analyzeReturnStatement(ReturnStatement& stmt);
    std::optional<Error> analyzeVariableStatement(VariableStatement& stmt);
    std::optional<Error> analyzeWhenStatement(WhenStatement& stmt);

    template<typename ExprType>
    std::pair<std::unique_ptr<Type>, std::optional<Error>> handleExpression(
        Expression& expr,
        std::function<std::pair<std::unique_ptr<Type>, std::optional<Error>>(ExprType&)>
            analyzeFunc);
    std::pair<std::unique_ptr<Type>, std::optional<Error>> analyzeExpression(Expression& expr);
    std::pair<std::unique_ptr<Type>, std::optional<Error>> analyzeArrayExpression(
        ArrayExpression& expr);
    std::pair<std::unique_ptr<Type>, std::optional<Error>> analyzeBinaryExpression(
        BinaryExpression& expr);
    std::pair<std::unique_ptr<Type>, std::optional<Error>> analyzeCallExpression(
        CallExpression& expr);
    std::pair<std::unique_ptr<Type>, std::optional<Error>> analyzeIdentifierExpression(
        IdentifierExpression& expr);
    std::pair<std::unique_ptr<Type>, std::optional<Error>> analyzeLambdaExpression(
        LambdaExpression& expr);
    std::pair<std::unique_ptr<Type>, std::optional<Error>> analyzeLiteralExpression(
        LiteralExpression& expr);
    std::pair<std::unique_ptr<Type>, std::optional<Error>> analyzeMemberExpression(
        MemberExpression& expr);
    std::pair<std::unique_ptr<Type>, std::optional<Error>> analyzeTypeCheckExpression(
        TypeCheckExpression& expr);
    std::pair<std::unique_ptr<Type>, std::optional<Error>> analyzeUnaryExpression(
        UnaryExpression& expr);
};

#endif