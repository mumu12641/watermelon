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
        return generateCallExpression(*callExpr);
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

llvm::Value* IRGen::generateArrayExpression(const ArrayExpression& expr)
{
    throw "Not yet implemented IRGen::generateArrayExpression";
}

llvm::Value* IRGen::generateBinaryExpression(const BinaryExpression& expr)
{

    auto         leftResult  = generateExpression(*expr.left);
    auto         rightResult = generateExpression(*expr.right);
    llvm::Value* leftValue   = leftResult->getType()->isPointerTy()
                                   ? this->builder->CreateLoad(
                                       this->generateType(expr.left->getType()), leftResult, "left")
                                   : leftResult;
    llvm::Value* rightValue =
        rightResult->getType()->isPointerTy()
            ? this->builder->CreateLoad(
                  this->generateType(expr.right->getType()), rightResult, "left")
            : rightResult;

    bool isFloatOperation = expr.left->getType() == Type::builtinFloat() ||
                            expr.right->getType() == Type::builtinFloat();
    switch (expr.op) {
        case BinaryExpression::Operator::ADD:
            return isFloatOperation ? this->builder->CreateFAdd(leftValue, rightValue, "fadd")
                                    : this->builder->CreateAdd(leftValue, rightValue, "add");
        case BinaryExpression::Operator::SUB:
            return isFloatOperation ? this->builder->CreateFSub(leftValue, rightValue, "fsub")
                                    : this->builder->CreateSub(leftValue, rightValue, "sub");
        case BinaryExpression::Operator::MUL:
            return isFloatOperation ? this->builder->CreateFMul(leftValue, rightValue, "fmul")
                                    : this->builder->CreateMul(leftValue, rightValue, "mul");
        case BinaryExpression::Operator::DIV:
            return isFloatOperation ? this->builder->CreateFDiv(leftValue, rightValue, "fdiv")
                                    : this->builder->CreateSDiv(leftValue, rightValue, "sdiv");
        case BinaryExpression::Operator::MOD:
            return isFloatOperation ? this->builder->CreateFRem(leftValue, rightValue, "frem")
                                    : this->builder->CreateSRem(leftValue, rightValue, "srem");

        case BinaryExpression::Operator::EQ:
            return isFloatOperation ? this->builder->CreateFCmpOEQ(leftValue, rightValue, "fcmp.eq")
                                    : this->builder->CreateICmpEQ(leftValue, rightValue, "icmp.eq");
        case BinaryExpression::Operator::NEQ:
            return isFloatOperation ? this->builder->CreateFCmpONE(leftValue, rightValue, "fcmp.ne")
                                    : this->builder->CreateICmpNE(leftValue, rightValue, "icmp.ne");
        case BinaryExpression::Operator::LT:
            return isFloatOperation
                       ? this->builder->CreateFCmpOLT(leftValue, rightValue, "fcmp.lt")
                       : this->builder->CreateICmpSLT(leftValue, rightValue, "icmp.lt");
        case BinaryExpression::Operator::LE:
            return isFloatOperation
                       ? this->builder->CreateFCmpOLE(leftValue, rightValue, "fcmp.le")
                       : this->builder->CreateICmpSLE(leftValue, rightValue, "icmp.le");
        case BinaryExpression::Operator::GT:
            return isFloatOperation
                       ? this->builder->CreateFCmpOGT(leftValue, rightValue, "fcmp.gt")
                       : this->builder->CreateICmpSGT(leftValue, rightValue, "icmp.gt");
        case BinaryExpression::Operator::GE:
            return isFloatOperation
                       ? this->builder->CreateFCmpOGE(leftValue, rightValue, "fcmp.ge")
                       : this->builder->CreateICmpSGE(leftValue, rightValue, "icmp.ge");


        case BinaryExpression::Operator::AND:
            return this->builder->CreateAnd(leftValue, rightValue, "and");
        case BinaryExpression::Operator::OR:
            return this->builder->CreateOr(leftValue, rightValue, "or");
        case BinaryExpression::Operator::ASSIGN:
        {
            this->builder->CreateStore(rightValue, leftResult);
            return rightValue;   
        }
    }
}

llvm::Value* IRGen::generateCallExpression(const CallExpression& expr) {}

llvm::Value* IRGen::generateIdentifierExpression(const IdentifierExpression& expr)
{
    auto value = this->valueTable.find(expr.name);
    switch (value->getKind()) {
        case IRValueKind::PROPERTY:
        {
            return this->builder->CreateStructGEP(this->generateType(this->currClass->name),
                                                  this->getCurrFunc()->getArg(0),
                                                  value->getOffset(),
                                                  Format("{0}_ptr", expr.name));
        }
        case IRValueKind::FUNCTIONPARAM:
        {
            return this->valueTable.find(expr.name)->getValue();
        }
    }
}

llvm::Value* IRGen::generateLambdaExpression(const LambdaExpression& expr)
{
    throw "Not yet implemented IRGen::generateLambdaExpression";
}

llvm::Value* IRGen::generateLiteralExpression(const LiteralExpression& expr)
{
    switch (expr.getType().kind) {
        case Type::Kind::INT: return this->builder->getInt32(std::get<int>(expr.value));
        case Type::Kind::FLOAT:
            return llvm::ConstantFP::get(llvm::Type::getFloatTy(*this->context),
                                         std::get<float>(expr.value));
        case Type::Kind::BOOL: return this->builder->getInt1(std::get<bool>(expr.value) ? 1 : 0);
        case Type::Kind::STRING:
            return builder->CreateGlobalStringPtr(std::get<std::string>(expr.value));
    }
}

llvm::Value* IRGen::generateMemberExpression(const MemberExpression& expr) {}

llvm::Value* IRGen::generateTypeCheckExpression(const TypeCheckExpression& expr)
{
    throw "Not yet implemented IRGen::generateTypeCheckExpression";
}

llvm::Value* IRGen::generateUnaryExpression(const UnaryExpression& expr) {}