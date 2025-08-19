#include "../include/dce_pass.h"

#include <set>
#include <vector>


namespace llvm {


PreservedAnalyses DeadCodeEliminationPass::run(Function& F, FunctionAnalysisManager& AM)
{
    return removeDeadInstructions(F) ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

bool DeadCodeEliminationPass::removeDeadInstructions(Function& F)
{
    SmallPtrSet<Instruction*, 32>  Alive;
    SmallVector<Instruction*, 128> Worklist;
    errs() << "removeDeadInstructions : " << F.getName() << "\n";

    for (Instruction& I : instructions(F)) {
        if (I.isDebugOrPseudoInst() || !I.isSafeToRemove()) {
            Alive.insert(&I);
            Worklist.push_back(&I);
        }
        if (isa<AllocaInst>(I) && !isDeadAlloca(dyn_cast<AllocaInst>(&I))) {
            Alive.insert(&I);
            Worklist.push_back(&I);
        }
        if (isa<StoreInst>(I) && !isDeadStore(dyn_cast<StoreInst>(&I))) {
            Alive.insert(&I);
            Worklist.push_back(&I);
        }
    }

    while (!Worklist.empty()) {
        Instruction* LiveInst = Worklist.pop_back_val();
        for (Use& OI : LiveInst->operands()) {
            if (Instruction* Inst = dyn_cast<Instruction>(OI)) {
                if (Alive.insert(Inst).second) {
                    Worklist.push_back(Inst);
                }
            }
        }
    }

    for (Instruction& I : instructions(F)) {
        if (!Alive.count(&I)) {
            errs() << "             remove : " << I << "\n";
            Worklist.push_back(&I);
            I.dropAllReferences();
        }
    }

    for (Instruction*& I : Worklist) {
        I->eraseFromParent();
    }

    return !Worklist.empty();
}

bool DeadCodeEliminationPass::isDeadAlloca(AllocaInst* AI)
{
    if (AI->use_empty()) {
        return true;
    }

    for (User* U : AI->users()) {
        if (Instruction* UserInst = dyn_cast<Instruction>(U)) {
            if (StoreInst* SI = dyn_cast<StoreInst>(UserInst)) {
                if (!isDeadStore(SI)) {
                    return false;
                }
            }

            else if (LoadInst* LI = dyn_cast<LoadInst>(UserInst)) {
                if (!LI->use_empty()) {
                    return false;
                }
            }
            else {
                return false;
            }
        }
        else {
            return false;
        }
    }
    return true;
}

bool DeadCodeEliminationPass::isDeadStore(StoreInst* SI)
{
    Value* Ptr = SI->getPointerOperand();
    if (AllocaInst* AI = dyn_cast<AllocaInst>(Ptr)) {
        return !hasLiveLoad(AI, SI);
    }
    return false;
}

bool DeadCodeEliminationPass::hasLiveLoad(AllocaInst* AI, StoreInst* SI)
{
    BasicBlock* StoreBB = SI->getParent();

    bool FoundStore = false;
    for (Instruction& I : *StoreBB) {
        if (&I == SI) {
            FoundStore = true;
            continue;
        }
        if (FoundStore) {
            if (LoadInst* LI = dyn_cast<LoadInst>(&I)) {
                if (LI->getPointerOperand() == AI && !LI->use_empty()) {
                    return true;
                }
            }
            if (StoreInst* OtherSI = dyn_cast<StoreInst>(&I)) {
                if (OtherSI->getPointerOperand() == AI) {
                    break;
                }
            }
        }
    }

    std::set<BasicBlock*>   Visited;
    std::queue<BasicBlock*> Worklist;

    for (BasicBlock* Succ : successors(StoreBB)) {
        Worklist.push(Succ);
    }

    while (!Worklist.empty()) {
        BasicBlock* BB = Worklist.front();
        Worklist.pop();

        if (Visited.count(BB)) continue;
        Visited.insert(BB);

        for (Instruction& I : *BB) {
            if (LoadInst* LI = dyn_cast<LoadInst>(&I)) {
                if (LI->getPointerOperand() == AI && !LI->use_empty()) {
                    return true;
                }
            }
            if (StoreInst* OtherSI = dyn_cast<StoreInst>(&I)) {
                if (OtherSI->getPointerOperand() == AI) {
                    goto next_block;
                }
            }
        }

        if (Visited.size() < 10) {
            for (BasicBlock* Succ : successors(BB)) {
                if (!Visited.count(Succ)) {
                    Worklist.push(Succ);
                }
            }
        }

    next_block:;
    }

    return false;
}


}   // namespace llvm
