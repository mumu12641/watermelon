#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

#include <set>
#include <vector>


namespace llvm {

struct DeadCodeEliminationPass : PassInfoMixin<DeadCodeEliminationPass>
{
    PreservedAnalyses run(Function& F, FunctionAnalysisManager& AM);

private:
    bool isDeadAlloca(AllocaInst* AI);
    bool isDeadStore(StoreInst* SI);
    bool hasLiveLoad(AllocaInst* AI, StoreInst* SI);
    bool removeDeadInstructions(Function& F);
    static bool isRequired() { return true; }
};

}   // namespace llvm


extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK llvmGetPassPluginInfo()
{
    return {LLVM_PLUGIN_API_VERSION, "DeadCodeElimination", "v0.1", [](llvm::PassBuilder& PB) {
                PB.registerPipelineParsingCallback(
                    [](llvm::StringRef            Name,
                       llvm::FunctionPassManager& FPM,
                       llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) {
                        if (Name == "dce-pass") {
                            FPM.addPass(llvm::DeadCodeEliminationPass());
                            return true;
                        }
                        return false;
                    });
            }};
}