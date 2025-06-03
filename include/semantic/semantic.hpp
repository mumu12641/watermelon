
#ifndef SEMANTIC_HPP
#define SEMANTIC_HPP

#include "../ast/ast.hpp"
#include "../lexer/token.hpp"
#include "../utils/error.hpp"

#include <string>
#include <unordered_map>
#include <vector>


class Scope
{
private:
    std::unordered_map<std::string, Type> map;

public:
    void add(const std::string& key, const Type& type);
};

class SymbolTable
{
private:
    std::vector<Scope> scopes;

public:
    void        enter_scope();
    void        exit_scope();
    const Type* find(const std::string& key);
    void        add(const std::string& key, const Type& type);
};

class SemanticAnalyzer
{
private:
    SymbolTable symbol_table;
    Program     program;

public:
    SemanticAnalyzer(Program p)
        : program(std::move(p))
        , symbol_table(SymbolTable())
    {
    }

    std::pair<std::unique_ptr<Program>, std::optional<Error>> analyze();
   
    void analyzeDeclaration(const Declaration& decl);
    void analyzeClassDeclaration(const ClassDeclaration& decl);
    void analyzeEnumDeclaration(const EnumDeclaration& decl);
    void analyzeFunctionDeclaration(const FunctionDeclaration& decl);
    
    void analyzeStatement(const Statement& stmt);
    void analyzeBlockStatement(const BlockStatement& stmt);
    void analyzeExpressionStatement(const ExpressionStatement& stmt);
    void analyzeForStatement(const ForStatement& stmt);
    void analyzeIfStatement(const IfStatement& stmt);
    void analyzeReturnStatement(const ReturnStatement& stmt);
    void analyzeVariableStatement(const VariableStatement& stmt);
    void analyzeWhenStatement(const WhenStatement& stmt);
    
    std::unique_ptr<Type> analyzeExpression(const Expression& expr);
    std::unique_ptr<Type> analyzeArrayExpression(const ArrayExpression& expr);
    std::unique_ptr<Type> analyzeBinaryExpression(const BinaryExpression& expr);
    std::unique_ptr<Type> analyzeCallExpression(const CallExpression& expr);
    std::unique_ptr<Type> analyzeIdentifierExpression(const IdentifierExpression& expr);
    std::unique_ptr<Type> analyzeLambdaExpression(const LambdaExpression& expr);
    std::unique_ptr<Type> analyzeLiteralExpression(const LiteralExpression& expr);
    std::unique_ptr<Type> analyzeMemberExpression(const MemberExpression& expr);
    std::unique_ptr<Type> analyzeTypeCheckExpression(const TypeCheckExpression& expr);
    std::unique_ptr<Type> analyzeUnaryExpression(const UnaryExpression& expr);
    
};

#endif