
#ifndef SEMANTIC_HPP
#define SEMANTIC_HPP

#include "../ast/ast.hpp"
#include "../lexer/token.hpp"
#include "../utils/error.hpp"

#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

class SymbolType
{
private:
    std::string type;
    // maybe some namespace or file name ...
public:
    SymbolType()
        : type("")
    {
    }
    SymbolType(const std::string& s)
        : type(s)
    {
    }
    const std::string& getType() const { return type; }
};

class Scope
{
private:
    std::string                                 name;
    std::unordered_map<std::string, SymbolType> map;

public:
    Scope(const std::string s)
        : name(s)
    {
    }
    void                                               add(const std::string& key, SymbolType type);
    const SymbolType*                                  find(const std::string& key);
    const std::unordered_map<std::string, SymbolType>& getMap() const { return map; }
    const std::string&                                 getName() const { return name; }
};

class SymbolTable
{
private:
    std::vector<Scope> scopes;

public:
    void              enterScope(const std::string& name);
    void              exitScope();
    const SymbolType* find(const std::string& key);
    void              add(const std::string& key, const std::string& type);
    void              debug() const;
};

class ClassTable
{
private:
    std::unordered_map<std::string, const ClassDeclaration*> classes;
    // std::vector<std::pair<std::string, const ClassDeclaration*>> classes;
    std::unordered_map<std::string, std::vector<const ClassDeclaration*>> inheritMap;

public:
    void                    add(const std::string& className, const ClassDeclaration*);
    const ClassDeclaration* find(const std::string& className);
    void addInheritMap(const std::string& className, std::vector<const ClassDeclaration*> parents);
    const std::vector<const ClassDeclaration*>* getInheritMap(const std::string& className);
    const bool checkInherit(const std::string& child, const std::string& parent);
};

class SemanticAnalyzer
{
private:
    SymbolTable              symbolTable;
    ClassTable               classTable;
    std::unique_ptr<Program> program;

    std::stack<std::pair<Type*, Location>> currentFunctionReturnTypes;


public:
    SemanticAnalyzer(std::unique_ptr<Program> p)
        : program(std::move(p))
        , symbolTable(SymbolTable())
        , classTable(ClassTable())
    {
    }
    std::optional<Error> validateMethodOverride(const MethodMember*     method,
                                                const ClassMember*      parentMember,
                                                const ClassDeclaration* classDecl,
                                                const ClassDeclaration* parentClass);
    std::optional<Error> validatePropertyOverride(const PropertyMember*   property,
                                                  const ClassMember*      parentMember,
                                                  const ClassDeclaration* classDecl,
                                                  const ClassDeclaration* parentClass);
    std::pair<std::unique_ptr<Program>, std::optional<Error>> analyze();

    // void analyzeDeclaration(const Declaration& decl);
    // void analyzeClassDeclaration(const ClassDeclaration& decl);
    // void analyzeEnumDeclaration(const EnumDeclaration& decl);
    // void analyzeFunctionDeclaration(const FunctionDeclaration& decl);

    std::optional<Error> analyzeStatement(const Statement& stmt);
    std::optional<Error> analyzeBlockStatement(const BlockStatement& stmt);
    std::optional<Error> analyzeExpressionStatement(const ExpressionStatement& stmt);
    std::optional<Error> analyzeForStatement(const ForStatement& stmt);
    std::optional<Error> analyzeIfStatement(const IfStatement& stmt);
    std::optional<Error> analyzeReturnStatement(const ReturnStatement& stmt);
    std::optional<Error> analyzeVariableStatement(const VariableStatement& stmt);
    std::optional<Error> analyzeWhenStatement(const WhenStatement& stmt);

    std::pair<std::unique_ptr<Type>, std::optional<Error>> analyzeExpression(
        const Expression& expr);
    std::pair<std::unique_ptr<Type>, std::optional<Error>> analyzeArrayExpression(
        const ArrayExpression& expr);
    std::pair<std::unique_ptr<Type>, std::optional<Error>> analyzeBinaryExpression(
        const BinaryExpression& expr);
    std::pair<std::unique_ptr<Type>, std::optional<Error>> analyzeCallExpression(
        const CallExpression& expr);
    std::pair<std::unique_ptr<Type>, std::optional<Error>> analyzeIdentifierExpression(
        const IdentifierExpression& expr);
    std::pair<std::unique_ptr<Type>, std::optional<Error>> analyzeLambdaExpression(
        const LambdaExpression& expr);
    std::pair<std::unique_ptr<Type>, std::optional<Error>> analyzeLiteralExpression(
        const LiteralExpression& expr);
    std::pair<std::unique_ptr<Type>, std::optional<Error>> analyzeMemberExpression(
        const MemberExpression& expr);
    std::pair<std::unique_ptr<Type>, std::optional<Error>> analyzeTypeCheckExpression(
        const TypeCheckExpression& expr);
    std::pair<std::unique_ptr<Type>, std::optional<Error>> analyzeUnaryExpression(
        const UnaryExpression& expr);
};

#endif