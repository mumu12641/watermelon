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
    this->valueTable.enterScope("block");
    for (const auto& stmt : blockStmt.statements) {
        generateStatement(*stmt);
    }
    this->valueTable.exitScope();
}

void IRGen::generateExpressionStatement(const ExpressionStatement& exprStmt)
{
    generateExpression(*exprStmt.expression);
}

void IRGen::generateForStatement(const ForStatement& stmt)
{
    auto iterable = generateExpression(*stmt.iterable);
    auto iterType = stmt.iterable->getType();

    auto iterableI8Ptr = this->builder->CreateBitCast(iterable, this->int8PtrTy, "iterableI8Ptr");
    this->builder->CreateCall(this->module->getFunction(Format("{0}__first", iterType.getName())),
                              {iterableI8Ptr});

    auto iterablePair = this->classTable.isClassIterable(iterType.getName());
    auto variable     = this->allocateStackVariable(
        stmt.variable, this->generateType(iterablePair->second.getName(), false));

    auto              currFunc = this->getCurrFunc();
    llvm::BasicBlock* condBB   = llvm::BasicBlock::Create(*context, "while.cond", currFunc);
    llvm::BasicBlock* bodyBB   = llvm::BasicBlock::Create(*context, "while.body", currFunc);
    llvm::BasicBlock* endBB    = llvm::BasicBlock::Create(*context, "while.end", currFunc);

    this->builder->CreateBr(condBB);
    this->builder->SetInsertPoint(condBB);
    auto cond = this->builder->CreateCall(
        this->module->getFunction(Format("{0}__end", iterType.getName())), {iterableI8Ptr});
    this->builder->CreateCondBr(cond, endBB, bodyBB);

    this->builder->SetInsertPoint(bodyBB);
    auto current = this->builder->CreateCall(
        this->module->getFunction(Format("{0}__current", iterType.getName())), {iterableI8Ptr});
    this->builder->CreateStore(current, variable);
    this->valueTable.add(stmt.variable, IRValue(variable));

    generateStatement(*stmt.body);

    this->builder->CreateCall(this->module->getFunction(Format("{0}__next", iterType.getName())),
                              {iterableI8Ptr});
    this->builder->CreateBr(condBB);

    this->builder->SetInsertPoint(endBB);
}

void IRGen::generateIfStatement(const IfStatement& stmt)
{
    auto currFunc = this->getCurrFunc();
    auto trueBB   = llvm::BasicBlock::Create(*context, "if.true");
    auto exitBB   = llvm::BasicBlock::Create(*context, "if.exit");
    auto elseBB   = stmt.elseBranch ? llvm::BasicBlock::Create(*context, "if.false") : exitBB;

    auto cond = generateExpression(*stmt.condition);
    this->builder->CreateCondBr(cond, trueBB, elseBB);

    trueBB->insertInto(currFunc);
    this->builder->SetInsertPoint(trueBB);
    generateStatement(*stmt.thenBranch);
    this->builder->CreateBr(exitBB);

    if (stmt.elseBranch) {
        elseBB->insertInto(currFunc);
        this->builder->SetInsertPoint(elseBB);
        generateStatement(*stmt.elseBranch);
        this->builder->CreateBr(exitBB);
    }
    exitBB->insertInto(currFunc);
    this->builder->SetInsertPoint(exitBB);
}

void IRGen::generateReturnStatement(const ReturnStatement& stmt)
{
    if (stmt.value) {
        builder->CreateStore(generateExpression(*stmt.value), retVal);
    }
    builder->CreateBr(retBB);
}

void IRGen::generateVariableStatement(const VariableStatement& stmt)
{
    auto         declType = this->generateType(*stmt.declType, true);
    llvm::Value* value    = this->allocateStackVariable(stmt.name, declType);
    this->valueTable.add(stmt.name, IRValue(value));
    if (stmt.initializer == nullptr) return;
    llvm::Value* init = generateExpression(*stmt.initializer);
    if (stmt.declType && stmt.initType && *stmt.declType != *stmt.initType) {
        init = this->builder->CreateBitCast(init, declType, "bit_cast");
    }
    this->builder->CreateStore(init, value);
}

void IRGen::generateWhenStatement(const WhenStatement& stmt) {}
