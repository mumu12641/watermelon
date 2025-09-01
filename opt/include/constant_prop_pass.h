#pragma once

#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"

namespace llvm {
class ConstantPropPass : public llvm::PassInfoMixin<ConstantPropPass>
{
public:
    llvm::PreservedAnalyses run(llvm::Function& F, llvm::FunctionAnalysisManager& AM);
};
}   // namespace llvm

extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK llvmGetPassPluginInfo()
{
    return {LLVM_PLUGIN_API_VERSION, "ConstantPropPass", "v0.1", [](llvm::PassBuilder& PB) {
                PB.registerPipelineParsingCallback(
                    [](llvm::StringRef            Name,
                       llvm::FunctionPassManager& FPM,
                       llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) {
                        if (Name == "constant-prop-pass") {
                            FPM.addPass(llvm::ConstantPropPass());
                            return true;
                        }
                        return false;
                    });
            }};
}