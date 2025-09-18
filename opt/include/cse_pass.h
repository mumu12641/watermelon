#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/DominanceFrontier.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

#include <algorithm>
#include <functional>
#include <set>
#include <unordered_map>
#include <vector>

namespace llvm {
struct CSEExpression
{
    enum class ExpressionType
    {
        BINARY_OP,
        CMP_INST,
        CAST_OP,
        GETELEMENTPTR_INST
    };
    ExpressionType            exprType;
    unsigned                  opcode;
    llvm::Type*               ty;
    bool                      isCommutative;
    std::vector<llvm::Value*> args;
    llvm::Value*              result;

    CSEExpression(Instruction&);
    CSEExpression() = default;

    bool operator==(const CSEExpression&) const;
};

struct CSEPass : PassInfoMixin<CSEPass>
{
    PreservedAnalyses run(Function& F, FunctionAnalysisManager& AM);
    void              getAnalysisUsage(AnalysisUsage& AU)
    {
        AU.addRequired<DominatorTreeWrapperPass>();
        AU.addRequired<DominanceFrontierWrapperPass>();
    }

private:
    bool canCSE(Instruction* I)
    {
        if (I->mayHaveSideEffects() || I->mayReadFromMemory() || I->isTerminator() ||
            isa<PHINode>(I) || isa<CallInst>(I) || isa<InvokeInst>(I))
            return false;

        return isa<BinaryOperator>(I) || isa<CmpInst>(I) || isa<CastInst>(I) ||
               isa<GetElementPtrInst>(I);
    }
};
}   // namespace llvm
extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK llvmGetPassPluginInfo()
{
    return {LLVM_PLUGIN_API_VERSION,
            "CommonSubexpressionElimination",
            "v0.1",
            [](llvm::PassBuilder& PB) {
                PB.registerPipelineParsingCallback(
                    [](llvm::StringRef            Name,
                       llvm::FunctionPassManager& FPM,
                       llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) {
                        if (Name == "cse-pass") {
                            FPM.addPass(llvm::CSEPass());
                            return true;
                        }
                        return false;
                    });
            }};
}