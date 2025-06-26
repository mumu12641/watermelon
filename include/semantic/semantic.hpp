
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

public:
    enum class SymbolKind
    {
        VAR,
        FUNC,
        CLASS
    };
    SymbolType()
        : name("")
        , kind(SymbolKind::VAR)
    {
    }
    SymbolType(const std::string& s)
        : name(s)
        , kind(SymbolKind::VAR)
    {
    }
    SymbolType(const std::string& s, SymbolKind kind)
        : name(s)
        , kind(kind)
    {
    }

    bool isBool() const { return kind == SymbolKind::VAR && (name == "bool" || name == "int"); }
    bool isVoid() const { return kind == SymbolKind::VAR && name == "void"; }
    bool canMathOp() const { return kind == SymbolKind::VAR && (name == "int" || name == "float"); }
    bool canCompare() const
    {
        return kind == SymbolKind::VAR && (name == "int" || name == "float");
    }

    const std::string& getName() const { return name; }
    const SymbolKind&  getKind() const { return kind; }


private:
    // only type name for simplity
    std::string name;
    SymbolKind  kind;
    // maybe some namespace or file name ...
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
    void              add(const std::string& key, const std::string& type,
                          SymbolType::SymbolKind kind = SymbolType::SymbolKind::VAR);
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
    bool checkInherit(const std::string& child, const std::string& parent) const;
    const std::vector<const ClassDeclaration*>* getInheritMap(const std::string& className) const;
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
    SymbolTable                                 symbolTable;
    ClassTable                                  classTable;
    FunctionTable                               functionTable;
    std::unique_ptr<Program>                    program;
    std::stack<std::pair<SymbolType, Location>> currentFunctionReturnTypes;

public:
    SemanticAnalyzer(std::unique_ptr<Program> p)
        : program(std::move(p))
        , symbolTable(SymbolTable())
        , classTable(ClassTable())
        , functionTable(FunctionTable())
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

    std::optional<Error> analyzeDeclaration(const Declaration& decl);
    std::optional<Error> analyzeClassDeclaration(const ClassDeclaration& decl);
    std::optional<Error> analyzeEnumDeclaration(const EnumDeclaration& decl);
    std::optional<Error> analyzeFunctionDeclaration(const FunctionDeclaration& decl);

    std::optional<Error> analyzeStatement(const Statement& stmt);
    std::optional<Error> analyzeBlockStatement(const BlockStatement& stmt);
    std::optional<Error> analyzeExpressionStatement(const ExpressionStatement& stmt);
    std::optional<Error> analyzeForStatement(const ForStatement& stmt);
    std::optional<Error> analyzeIfStatement(const IfStatement& stmt);
    std::optional<Error> analyzeReturnStatement(const ReturnStatement& stmt);
    std::optional<Error> analyzeVariableStatement(const VariableStatement& stmt);
    std::optional<Error> analyzeWhenStatement(const WhenStatement& stmt);

    std::pair<std::unique_ptr<SymbolType>, std::optional<Error>> analyzeExpression(
        const Expression& expr);
    std::pair<std::unique_ptr<SymbolType>, std::optional<Error>> analyzeArrayExpression(
        const ArrayExpression& expr);
    std::pair<std::unique_ptr<SymbolType>, std::optional<Error>> analyzeBinaryExpression(
        const BinaryExpression& expr);
    std::pair<std::unique_ptr<SymbolType>, std::optional<Error>> analyzeFunctionCallExpression(
        const CallExpression& expr);
    std::pair<std::unique_ptr<SymbolType>, std::optional<Error>> analyzeIdentifierExpression(
        const IdentifierExpression& expr);
    std::pair<std::unique_ptr<SymbolType>, std::optional<Error>> analyzeLambdaExpression(
        const LambdaExpression& expr);
    std::pair<std::unique_ptr<SymbolType>, std::optional<Error>> analyzeLiteralExpression(
        const LiteralExpression& expr);
    std::pair<std::unique_ptr<SymbolType>, std::optional<Error>> analyzeMemberExpression(
        const MemberExpression& expr);
    std::pair<std::unique_ptr<SymbolType>, std::optional<Error>> analyzeTypeCheckExpression(
        const TypeCheckExpression& expr);
    std::pair<std::unique_ptr<SymbolType>, std::optional<Error>> analyzeUnaryExpression(
        const UnaryExpression& expr);
};

#endif