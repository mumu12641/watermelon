#include "../include/ir/ir.hpp"
#include "../include/utils/format.hpp"

void IRGen::generateStatement(const Statement& stmt)
{
    if (auto* blockStmt = dynamic_cast<const BlockStatement*>(&stmt)) {
        generateBlockStatement(*blockStmt);
    }
    else if (auto* exprStmt = dynamic_cast<const ExpressionStatement*>(&stmt)) {
        generateExpressionStatement(*exprStmt);
    }
    else if (auto* forStmt = dynamic_cast<const ForStatement*>(&stmt)) {
        generateForStatement(*forStmt);
    }
    else if (auto* ifStmt = dynamic_cast<const IfStatement*>(&stmt)) {
        generateIfStatement(*ifStmt);
    }
    else if (auto* returnStmt = dynamic_cast<const ReturnStatement*>(&stmt)) {
        generateReturnStatement(*returnStmt);
    }
    else if (auto* varStmt = dynamic_cast<const VariableStatement*>(&stmt)) {
        generateVariableStatement(*varStmt);
    }
    else if (auto* whenStmt = dynamic_cast<const WhenStatement*>(&stmt)) {
        generateWhenStatement(*whenStmt);
    }
}

void IRGen::generateBlockStatement(const BlockStatement& blockStmt)
{
    for (const auto& stmt : blockStmt.statements) {
        generateStatement(*stmt);
    }
}
void IRGen::generateExpressionStatement(const ExpressionStatement& exprStmt)
{
    generateExpression(*exprStmt.expression);
}

void IRGen::generateForStatement(const ForStatement& stmt) {}

void IRGen::generateIfStatement(const IfStatement& stmt) {}

void IRGen::generateReturnStatement(const ReturnStatement& stmt) {}

void IRGen::generateVariableStatement(const VariableStatement& stmt) {}

void IRGen::generateWhenStatement(const WhenStatement& stmt) {}
