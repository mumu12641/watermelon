#include "../../include/opt/ConstantFoldingPass.hpp"

#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"

#include <vector>

using namespace llvm;

PreservedAnalyses ConstantFoldingPass::run(Function& F, FunctionAnalysisManager& AM)
{
    if (F.isDeclaration()) {
        return PreservedAnalyses::all();
    }

    bool Changed      = false;
    bool LocalChanged = true;

    errs() << "=== Watermelon Constant Folding Pass ===\n";
    errs() << "Function: " << F.getName() << "\n";

    // 迭代直到没有更多常量可以折叠
    int iterations = 0;
    while (LocalChanged && iterations < 10) {   // 防止无限循环
        LocalChanged = false;
        iterations++;

        LocalChanged |= foldBinaryOperations(F);
        LocalChanged |= foldComparisonOperations(F);
        LocalChanged |= foldCastOperations(F);

        Changed |= LocalChanged;

        if (LocalChanged) {
            errs() << "  Iteration " << iterations << ": Found constants to fold\n";
        }
    }

    if (Changed) {
        errs() << "✅ Constants folded in function: " << F.getName()
               << " (iterations: " << iterations << ")\n";
    }
    else {
        errs() << "ℹ️  No constants to fold in function: " << F.getName() << "\n";
    }
    errs() << "========================================\n\n";

    return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

bool ConstantFoldingPass::foldBinaryOperations(Function& F)
{
    std::vector<Instruction*> ToReplace;
    bool                      Changed = false;

    for (BasicBlock& BB : F) {
        for (Instruction& I : BB) {
            if (BinaryOperator* BO = dyn_cast<BinaryOperator>(&I)) {
                Value* LHS = BO->getOperand(0);
                Value* RHS = BO->getOperand(1);

                // 检查两个操作数是否都是常量
                if (isa<Constant>(LHS) && isa<Constant>(RHS)) {
                    Constant* CLHS = cast<Constant>(LHS);
                    Constant* CRHS = cast<Constant>(RHS);

                    Constant* Result = nullptr;

                    switch (BO->getOpcode()) {
                        case Instruction::Add:
                            if (BO->getType()->isIntegerTy()) {
                                Result = ConstantExpr::getAdd(CLHS, CRHS);
                            }
                            else if (BO->getType()->isFloatingPointTy()) {
                                Result = ConstantExpr::getFAdd(CLHS, CRHS);
                            }
                            break;
                        case Instruction::Sub:
                            if (BO->getType()->isIntegerTy()) {
                                Result = ConstantExpr::getSub(CLHS, CRHS);
                            }
                            else if (BO->getType()->isFloatingPointTy()) {
                                Result = ConstantExpr::getFSub(CLHS, CRHS);
                            }
                            break;
                        case Instruction::Mul:
                            if (BO->getType()->isIntegerTy()) {
                                Result = ConstantExpr::getMul(CLHS, CRHS);
                            }
                            else if (BO->getType()->isFloatingPointTy()) {
                                Result = ConstantExpr::getFMul(CLHS, CRHS);
                            }
                            break;
                        case Instruction::UDiv: Result = ConstantExpr::getUDiv(CLHS, CRHS); break;
                        case Instruction::SDiv: Result = ConstantExpr::getSDiv(CLHS, CRHS); break;
                        case Instruction::FDiv: Result = ConstantExpr::getFDiv(CLHS, CRHS); break;
                        case Instruction::URem: Result = ConstantExpr::getURem(CLHS, CRHS); break;
                        case Instruction::SRem: Result = ConstantExpr::getSRem(CLHS, CRHS); break;
                        case Instruction::FRem: Result = ConstantExpr::getFRem(CLHS, CRHS); break;
                        case Instruction::And: Result = ConstantExpr::getAnd(CLHS, CRHS); break;
                        case Instruction::Or: Result = ConstantExpr::getOr(CLHS, CRHS); break;
                        case Instruction::Xor: Result = ConstantExpr::getXor(CLHS, CRHS); break;
                        case Instruction::Shl: Result = ConstantExpr::getShl(CLHS, CRHS); break;
                        case Instruction::LShr: Result = ConstantExpr::getLShr(CLHS, CRHS); break;
                        case Instruction::AShr: Result = ConstantExpr::getAShr(CLHS, CRHS); break;
                    }

                    if (Result) {
                        errs() << "  Folding binary op: " << *BO << " -> " << *Result << "\n";
                        BO->replaceAllUsesWith(Result);
                        ToReplace.push_back(BO);
                        Changed = true;
                    }
                }
            }
        }
    }

    // 删除被替换的指令
    for (Instruction* I : ToReplace) {
        I->eraseFromParent();
    }

    return Changed;
}

bool ConstantFoldingPass::foldComparisonOperations(Function& F)
{
    std::vector<Instruction*> ToReplace;
    bool                      Changed = false;

    for (BasicBlock& BB : F) {
        for (Instruction& I : BB) {
            if (ICmpInst* ICI = dyn_cast<ICmpInst>(&I)) {
                Value* LHS = ICI->getOperand(0);
                Value* RHS = ICI->getOperand(1);

                if (isa<Constant>(LHS) && isa<Constant>(RHS)) {
                    Constant* CLHS = cast<Constant>(LHS);
                    Constant* CRHS = cast<Constant>(RHS);

                    Constant* Result = ConstantExpr::getICmp(ICI->getPredicate(), CLHS, CRHS);

                    errs() << "  Folding icmp: " << *ICI << " -> " << *Result << "\n";
                    ICI->replaceAllUsesWith(Result);
                    ToReplace.push_back(ICI);
                    Changed = true;
                }
            }
            else if (FCmpInst* FCI = dyn_cast<FCmpInst>(&I)) {
                Value* LHS = FCI->getOperand(0);
                Value* RHS = FCI->getOperand(1);

                if (isa<Constant>(LHS) && isa<Constant>(RHS)) {
                    Constant* CLHS = cast<Constant>(LHS);
                    Constant* CRHS = cast<Constant>(RHS);

                    Constant* Result = ConstantExpr::getFCmp(FCI->getPredicate(), CLHS, CRHS);

                    errs() << "  Folding fcmp: " << *FCI << " -> " << *Result << "\n";
                    FCI->replaceAllUsesWith(Result);
                    ToReplace.push_back(FCI);
                    Changed = true;
                }
            }
        }
    }

    // 删除被替换的指令
    for (Instruction* I : ToReplace) {
        I->eraseFromParent();
    }

    return Changed;
}

bool ConstantFoldingPass::foldCastOperations(Function& F)
{
    std::vector<Instruction*> ToReplace;
    bool                      Changed = false;

    for (BasicBlock& BB : F) {
        for (Instruction& I : BB) {
            if (CastInst* CI = dyn_cast<CastInst>(&I)) {
                Value* Operand = CI->getOperand(0);

                if (isa<Constant>(Operand)) {
                    Constant* COperand = cast<Constant>(Operand);

                    Constant* Result =
                        ConstantExpr::getCast(CI->getOpcode(), COperand, CI->getDestTy());

                    errs() << "  Folding cast: " << *CI << " -> " << *Result << "\n";
                    CI->replaceAllUsesWith(Result);
                    ToReplace.push_back(CI);
                    Changed = true;
                }
            }
        }
    }

    // 删除被替换的指令
    for (Instruction* I : ToReplace) {
        I->eraseFromParent();
    }

    return Changed;
}

// Pass 注册
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo()
{
    return {LLVM_PLUGIN_API_VERSION, "WatermelonConstantFolding", "v1.0", [](PassBuilder& PB) {
                PB.registerPipelineParsingCallback([](StringRef            Name,
                                                      FunctionPassManager& FPM,
                                                      ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "watermelon-constant-folding") {
                        FPM.addPass(ConstantFoldingPass());
                        return true;
                    }
                    return false;
                });
            }};
}