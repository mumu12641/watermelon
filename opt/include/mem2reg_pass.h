#pragma once

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

namespace llvm {
class Mem2RegPass : public llvm::PassInfoMixin<Mem2RegPass>
{
public:
    llvm::PreservedAnalyses run(llvm::Function& F, llvm::FunctionAnalysisManager& AM);
    void                    getAnalysisUsage(AnalysisUsage& AU)
    {
        AU.addRequired<DominatorTreeWrapperPass>();
        AU.addRequired<DominanceFrontierWrapperPass>();
    }

private:
    std::map<PHINode*, AllocaInst*> insertPhiNode(std::vector<AllocaInst*>& allocas,

                                                  llvm::Function& F, DominanceFrontier& DF);
    std::vector<Instruction*>       removeMemInst(std::map<PHINode*, AllocaInst*> phiMap,

                                                  std::vector<AllocaInst*>& allocas,

                                                  llvm::Function& F, DominatorTree& DT);
bool isPromotable(AllocaInst* AI);

    bool                            changed = false;
};
}   // namespace llvm

extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK llvmGetPassPluginInfo()
{
    return {LLVM_PLUGIN_API_VERSION, "Mem2RegPass", "v0.1", [](llvm::PassBuilder& PB) {
                PB.registerPipelineParsingCallback(
                    [](llvm::StringRef            Name,
                       llvm::FunctionPassManager& FPM,
                       llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) {
                        if (Name == "mem2reg-pass") {
                            FPM.addPass(llvm::Mem2RegPass());
                            return true;
                        }
                        return false;
                    });
            }};
}