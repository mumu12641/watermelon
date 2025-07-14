#include "../include/ir/ir.hpp"
#include "../include/utils/format.hpp"

llvm::Value* IRGen::generateExpression(const Expression& expr)
{
    if (const auto* arrayExpr = dynamic_cast<const ArrayExpression*>(&expr)) {
        return generateArrayExpression(*arrayExpr);
    }
    else if (const auto* binaryExpr = dynamic_cast<const BinaryExpression*>(&expr)) {
        return generateBinaryExpression(*binaryExpr);
    }
    else if (const auto* callExpr = dynamic_cast<const CallExpression*>(&expr)) {
        return generateFunctionCallExpression(*callExpr);
    }
    else if (const auto* idExpr = dynamic_cast<const IdentifierExpression*>(&expr)) {
        return generateIdentifierExpression(*idExpr);
    }
    else if (const auto* lambdaExpr = dynamic_cast<const LambdaExpression*>(&expr)) {
        return generateLambdaExpression(*lambdaExpr);
    }
    else if (const auto* literalExpr = dynamic_cast<const LiteralExpression*>(&expr)) {
        return generateLiteralExpression(*literalExpr);
    }
    else if (const auto* memberExpr = dynamic_cast<const MemberExpression*>(&expr)) {
        return generateMemberExpression(*memberExpr);
    }
    else if (const auto* typeCheckExpr = dynamic_cast<const TypeCheckExpression*>(&expr)) {
        return generateTypeCheckExpression(*typeCheckExpr);
    }
    else if (const auto* unaryExpr = dynamic_cast<const UnaryExpression*>(&expr)) {
        return generateUnaryExpression(*unaryExpr);
    }
    return nullptr;
}

llvm::Value* IRGen::generateArrayExpression(const ArrayExpression& expr) {}

llvm::Value* IRGen::generateBinaryExpression(const BinaryExpression& expr) {
    
}

llvm::Value* IRGen::generateFunctionCallExpression(const CallExpression& expr) {}

llvm::Value* IRGen::generateIdentifierExpression(const IdentifierExpression& expr) {}

llvm::Value* IRGen::generateLambdaExpression(const LambdaExpression& expr) {}

llvm::Value* IRGen::generateLiteralExpression(const LiteralExpression& expr) {}

llvm::Value* IRGen::generateMemberExpression(const MemberExpression& expr) {}

llvm::Value* IRGen::generateTypeCheckExpression(const TypeCheckExpression& expr) {}

llvm::Value* IRGen::generateUnaryExpression(const UnaryExpression& expr) {}