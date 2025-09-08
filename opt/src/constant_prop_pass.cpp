#include "../include/constant_prop_pass.h"

#include <map>

namespace llvm {
PreservedAnalyses ConstantPropPass::run(Function& F, FunctionAnalysisManager& AM)
{

    bool                        changed = false;
    std::map<Value*, Constant*> constantMap;
    std::vector<Instruction*>   removeInsts;
    std::set<BasicBlock*>       unreachableBlocks;
    // errs() << "LocalConstantPropPass: " << F.getName() << "\n";
    for (auto& block : F) {
        for (auto& inst : block) {
            if (auto storeInst = dyn_cast<StoreInst>(&inst)) {
                Value* value   = storeInst->getValueOperand();
                Value* pointer = storeInst->getPointerOperand();
                if (Constant* constant = dyn_cast<Constant>(value)) {
                    constantMap[pointer] = constant;
                }
            }
        }
        for (auto& inst : block) {
            if (auto* loadInst = dyn_cast<LoadInst>(&inst)) {
                auto pointer = loadInst->getPointerOperand();
                if (constantMap.count(pointer)) {
                    Constant* constant = constantMap[pointer];
                    loadInst->replaceAllUsesWith(constant);
                    removeInsts.push_back(loadInst);
                    changed = true;
                }
            }
            else if (auto* binaryOpInst = dyn_cast<BinaryOperator>(&inst)) {
                Constant* left  = dyn_cast<Constant>(binaryOpInst->getOperand(0));
                Constant* right = dyn_cast<Constant>(binaryOpInst->getOperand(1));
                if (left && right) {
                    switch (binaryOpInst->getOpcode()) {
                        case Instruction::Add:
                        case Instruction::Sub:
                        case Instruction::Mul:
                        case Instruction::UDiv:
                        case Instruction::SDiv:
                        case Instruction::URem:
                        case Instruction::SRem:
                        case Instruction::And:
                        case Instruction::Or:
                        case Instruction::Xor:
                        case Instruction::Shl:
                        case Instruction::LShr:
                        case Instruction::AShr:
                        {
                            if (Constant* result =
                                    ConstantExpr::get(binaryOpInst->getOpcode(), left, right)) {
                                constantMap[binaryOpInst] = result;
                                binaryOpInst->replaceAllUsesWith(result);
                                removeInsts.push_back(binaryOpInst);
                                changed = true;
                            }
                            break;
                        }
                    }
                }
            }
            else if (auto* icmpInst = dyn_cast<ICmpInst>(&inst)) {
                auto* left  = dyn_cast<Constant>(icmpInst->getOperand(0));
                auto* right = dyn_cast<Constant>(icmpInst->getOperand(1));
                if (left && right) {
                    if (Constant* result =
                            ConstantExpr::getICmp(icmpInst->getPredicate(), left, right)) {
                        icmpInst->replaceAllUsesWith(result);
                        removeInsts.push_back(icmpInst);
                        changed = true;
                    }
                }
            }
        }
        constantMap.clear();
    }


    for (auto& block : F) {
        for (auto instIt = block.begin(); instIt != block.end();) {
            Instruction* inst = &*instIt;
            ++instIt;
            if (auto* branchInst = dyn_cast<BranchInst>(inst)) {
                if (branchInst->isConditional()) {
                    Constant* cond = dyn_cast<Constant>(branchInst->getCondition());
                    if (cond) {
                        BasicBlock* currentBlock = branchInst->getParent();
                        BasicBlock* trueSucc     = branchInst->getSuccessor(0);
                        BasicBlock* falseSucc    = branchInst->getSuccessor(1);

                        bool        takeTrue = cond->isOneValue();
                        BasicBlock* liveSucc = takeTrue ? trueSucc : falseSucc;
                        BasicBlock* deadSucc = takeTrue ? falseSucc : trueSucc;

                        BranchInst* newBranch = BranchInst::Create(liveSucc, branchInst);

                        branchInst->eraseFromParent();
                        deadSucc->removePredecessor(currentBlock);
                        changed = true;
                    }
                }
            }
        }
    }

    for (BasicBlock& block : F) {
        if (pred_empty(&block) && &block != &F.getEntryBlock()) {
            unreachableBlocks.insert(&block);
            for (auto succIt = succ_begin(&block); succIt != succ_end(&block); ++succIt) {
                BasicBlock* successor = *succIt;
                if (std::distance(pred_begin(successor), pred_end(successor)) == 1) {
                    unreachableBlocks.insert(successor);
                }
            }
        }
    }

    for (auto& block : F) {
        for (auto& inst : block) {
            if (auto* phi = dyn_cast<PHINode>(&inst)) {
                std::vector<std::pair<Value*, BasicBlock*>> reachableValues;
                for (int i = 0; i < phi->getNumIncomingValues(); i++) {
                    auto incomingBlock = phi->getIncomingBlock(i);
                    if (unreachableBlocks.find(incomingBlock) == unreachableBlocks.end()) {
                        reachableValues.push_back({phi->getIncomingValue(i), incomingBlock});
                    }
                }
                if (reachableValues.size() == 1) {
                    phi->replaceAllUsesWith(reachableValues[0].first);
                    removeInsts.push_back(phi);
                    changed = true;
                }
                else if (reachableValues.size() < phi->getNumIncomingValues()) {
                    PHINode* newPhi = PHINode::Create(phi->getType(),
                                                      reachableValues.size(),
                                                      phi->getName() + ".simplified",
                                                      phi);

                    for (auto& Val : reachableValues) {
                        newPhi->addIncoming(Val.first, Val.second);
                    }
                    phi->replaceAllUsesWith(newPhi);
                    phi->setOperand(0, UndefValue::get(phi->getType()));
                    removeInsts.push_back(phi);
                    changed = true;
                }
            }
        }
    }

    for (Instruction* I : removeInsts) {
        if (I->use_empty() && !I->mayHaveSideEffects() && !I->isTerminator()) {
            // errs() << "     remove " << *I << "\n";
            I->eraseFromParent();
        }
    }
    
    if (!unreachableBlocks.empty()) {
        for (BasicBlock* BB : unreachableBlocks) {
            while (BB->size() > 1) {
                Instruction& I = BB->front();
                if (&I == BB->getTerminator()) break;
                I.replaceAllUsesWith(UndefValue::get(I.getType()));
                I.eraseFromParent();
            }
            if (BB->getTerminator()) {
                BB->getTerminator()->eraseFromParent();
            }
        }

        for (BasicBlock* BB : unreachableBlocks) {
            // errs() << "Removing unreachable block: " << BB->getName() << "\n";
            BB->eraseFromParent();
            changed = true;
        }
    }
    return changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

}   // namespace llvm