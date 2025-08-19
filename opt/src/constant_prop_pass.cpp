#include "../include/constant_prop_pass.h"

#include <map>

namespace llvm {
PreservedAnalyses ConstantPropPass::run(Function& F, FunctionAnalysisManager& AM)
{

    bool                        changed = false;
    std::map<Value*, Constant*> constantMap;
    std::vector<Instruction*>   removeInsts;

    errs() << "ConstantPropPass: " << F.getName() << "\n";

    for (auto& block : F) {
        for (auto& inst : block) {
            if (auto storeInst = dyn_cast<StoreInst>(&inst)) {
                Value* value   = storeInst->getValueOperand();
                Value* pointer = storeInst->getPointerOperand();
                if (Constant* constant = dyn_cast<Constant>(value)) {
                    constantMap[pointer] = constant;
                }
            }
        }for (auto& inst : block) {
            if (auto loadInst = dyn_cast<LoadInst>(&inst)) {
                auto pointer = loadInst->getPointerOperand();
                if (constantMap.count(pointer)) {
                    Constant* constant = constantMap[pointer];
                    loadInst->replaceAllUsesWith(constant);
                    removeInsts.push_back(loadInst);
                    changed = true;
                }
            }
            else if (auto binaryOpInst = dyn_cast<BinaryOperator>(&inst)) {
                Value*    left          = binaryOpInst->getOperand(0);
                Value*    right         = binaryOpInst->getOperand(1);
                Constant* leftConstant  = dyn_cast<Constant>(left);
                Constant* rightConstant = dyn_cast<Constant>(right);
                if (leftConstant && rightConstant) {
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
                            if (Constant* result = ConstantExpr::get(
                                    binaryOpInst->getOpcode(), leftConstant, rightConstant)) {
                                constantMap[binaryOpInst] = result;
                                binaryOpInst->replaceAllUsesWith(result);
                                removeInsts.push_back(binaryOpInst);
                                changed = true;
                            }
                            break;
                        }
                        case Instruction::ICmp:
                        {
                            ICmpInst* icmp = cast<ICmpInst>(binaryOpInst);
                            if (Constant* result = ConstantExpr::getICmp(
                                    icmp->getPredicate(), leftConstant, rightConstant)) {
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
        }
        constantMap.clear();
    }
    for (Instruction* I : removeInsts) {
        if (I->use_empty() && !I->mayHaveSideEffects() && !I->isTerminator()) {
            errs() << "     remove " << *I << "\n";

            I->eraseFromParent();
        }
    }
    return changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

}   // namespace llvm