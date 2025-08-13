#ifndef CONSTANT_FOLDING_PASS_H
#define CONSTANT_FOLDING_PASS_H

#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"   
#include "llvm/Passes/PassPlugin.h"


class ConstantFoldingPass : public llvm::PassInfoMixin<ConstantFoldingPass>
{
public:
    llvm::PreservedAnalyses run(llvm::Function& F, llvm::FunctionAnalysisManager& AM);

private:
    bool foldBinaryOperations(llvm::Function& F);
    bool foldComparisonOperations(llvm::Function& F);
    bool foldCastOperations(llvm::Function& F);
};


#endif