#pragma once

#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
namespace llvm
{
    class MyPass : public llvm::PassInfoMixin<MyPass>
    {
    public:
        llvm::PreservedAnalyses run(llvm::Function &F, llvm::FunctionAnalysisManager &AM);

    private:
        bool foldBinaryOperations(llvm::Function &F);
        bool foldComparisonOperations(llvm::Function &F);
        bool foldCastOperations(llvm::Function &F);
    };
}

extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK
llvmGetPassPluginInfo()
{
    return {
        LLVM_PLUGIN_API_VERSION, "MyPass", "v0.1",
        [](llvm::PassBuilder &PB)
        {
            PB.registerPipelineParsingCallback(
                [](llvm::StringRef Name, llvm::FunctionPassManager &FPM,
                   llvm::ArrayRef<llvm::PassBuilder::PipelineElement>)
                {
                    if (Name == "mypass")
                    {
                        FPM.addPass(llvm::MyPass());
                        return true;
                    }
                    return false;
                });
        }};
}