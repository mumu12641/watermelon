#include "../include/dce_pass.h"

#include <algorithm>
#include <set>
#include <stack>
#include <vector>

namespace llvm {

PreservedAnalyses DeadCodeEliminationPass::run(Function& F, FunctionAnalysisManager& AM)
{
    return eliminateDeadCode(F) ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

bool DeadCodeEliminationPass::eliminateDeadCode(Function& F)
{
    errs() << "DeadCodeEliminationPass : " << F.getName() << "\n";
    bool                             changed = false;
    SmallSetVector<Instruction*, 16> workList;
    for (Instruction& inst : llvm::make_early_inc_range(instructions(F))) {

        if (!workList.count(&inst)) {
            changed |= isDCEInstruction(&inst, workList);
        }
    }

    while (!workList.empty()) {
        auto inst = workList.pop_back_val();
        changed |= isDCEInstruction(inst, workList);
    }
    return changed;
}

bool DeadCodeEliminationPass::isDCEInstruction(Instruction*                      inst,
                                               SmallSetVector<Instruction*, 16>& workList)
{
    if (isInstructionTriviallyDead(inst)) {
        auto opsNum = inst->getNumOperands();
        for (auto i = 0; i < opsNum; i++) {
            auto op = inst->getOperand(i);
            inst->setOperand(i, nullptr);

            if (!op->use_empty()) continue;

            if (auto opInst = dyn_cast<Instruction>(op)) {
                if (isInstructionTriviallyDead(opInst)) {
                    workList.insert(opInst);
                }
            }
        }
        errs() << "     remove : " << *inst << "\n";
        inst->eraseFromParent();
        return true;
    }
    return false;
}
bool DeadCodeEliminationPass::isInstructionTriviallyDead(Instruction* inst)
{
    if (!inst->use_empty()) return false;
    if (inst->isTerminator()) return false;
    if (inst->isEHPad()) return false;
    if (!inst->mayHaveSideEffects()) return true;
    return false;
}


}   // namespace llvm
