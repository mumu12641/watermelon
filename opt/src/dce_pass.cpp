#include "../include/dce_pass.h"

#include <algorithm>
#include <set>
#include <stack>
#include <vector>

namespace llvm {

PreservedAnalyses DeadCodeEliminationPass::run(Function& F, FunctionAnalysisManager& AM)
{
    return removeDeadInstructions(F) ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

bool DeadCodeEliminationPass::removeDeadInstructions(Function& F)
{
    SmallPtrSet<Instruction*, 32>  alive;
    SmallVector<Instruction*, 128> worklist;
    errs() << "DeadCodeEliminationPass : " << F.getName() << "\n";

    for (Instruction& inst : instructions(F)) {
        if (inst.isDebugOrPseudoInst() || !inst.isSafeToRemove()) {
            alive.insert(&inst);
            worklist.push_back(&inst);
        }
        if (isa<AllocaInst>(inst) && !isDeadAlloca(dyn_cast<AllocaInst>(&inst))) {
            alive.insert(&inst);
            worklist.push_back(&inst);
        }
        if (isa<StoreInst>(inst) && !isDeadStore(dyn_cast<StoreInst>(&inst))) {
            alive.insert(&inst);
            worklist.push_back(&inst);
        }
    }

    while (!worklist.empty()) {
        Instruction* liveInst = worklist.pop_back_val();
        for (Use& op : liveInst->operands()) {
            if (Instruction* Inst = dyn_cast<Instruction>(op)) {
                if (alive.insert(Inst).second) {
                    worklist.push_back(Inst);
                }
            }
        }
    }

    for (Instruction& inst : instructions(F)) {
        if (!alive.count(&inst)) {
            errs() << "             remove : " << inst << "\n";
            worklist.push_back(&inst);
            inst.dropAllReferences();
        }
    }

    for (Instruction*& inst : worklist) {
        inst->eraseFromParent();
    }

    return !worklist.empty();
}

bool DeadCodeEliminationPass::isDeadAlloca(AllocaInst* allocaInst)
{
    if (allocaInst->use_empty()) return true;

    for (auto use : allocaInst->users()) {
        if (auto useInst = dyn_cast<Instruction>(use)) {
            if (auto storeInst = dyn_cast<StoreInst>(useInst)) {
                if (isDeadStore(storeInst)) return true;
            }
            else if (auto loadInst = dyn_cast<LoadInst>(useInst)) {
                if (loadInst->use_empty()) return true;
            }
            else {
                return false;
            }
        }
    }
    return false;
}

bool DeadCodeEliminationPass::isDeadStore(StoreInst* storeInst)
{
    auto ptr = storeInst->getPointerOperand();
    if (auto allocaInst = dyn_cast<AllocaInst>(ptr)) {
        return !hasLiveLoad(allocaInst, storeInst);
    }
    return false;
}

bool DeadCodeEliminationPass::hasLiveLoad(AllocaInst* allocaInst, StoreInst* storeInst)
{
    auto parentBlock = storeInst->getParent();
    bool find        = false;
    for (auto& inst : *parentBlock) {
        if (&inst == storeInst) {
            find = true;
            continue;
        }
        if (find) {
            if (auto loadInst = dyn_cast<LoadInst>(&inst)) {
                if (loadInst->getPointerOperand() == allocaInst && !loadInst->use_empty()) {
                    return true;
                }
            }
        }
    }

    std::set<BasicBlock*>   visited;
    std::queue<BasicBlock*> worklist;
    for (auto succ : successors(parentBlock)) {
        worklist.push(succ);
    }

    while (!worklist.empty()) {
        auto currBlock = worklist.front();
        worklist.pop();

        if (visited.count(currBlock)) continue;
        visited.insert(currBlock);

        for (auto& inst : *currBlock) {
            if (auto loadInst = dyn_cast<LoadInst>(&inst)) {
                if (loadInst->getPointerOperand() == allocaInst && !loadInst->use_empty()) {
                    return true;
                }
            }
            // if (StoreInst* otherStoreInst = dyn_cast<StoreInst>(&inst)) {
            //     if (otherStoreInst->getPointerOperand() == allocaInst) {}
            // }
        }
        for (auto succ : successors(currBlock)) {
            worklist.push(succ);
        }
    }

    return false;
}


}   // namespace llvm
