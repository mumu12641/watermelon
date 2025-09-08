#include "../include/mem2reg_pass.h"

#include <map>
#include <queue>
#include <set>
#include <stack>
#include <unordered_map>
#include <utility>
#include <vector>


namespace llvm {

/***
 * 1、插入 phi：确定要 phi 的变量（跨多个基本快的内存变量，在被定义变量
 * store指令所在的支配边界插入） 2、变量重命名：基于栈的到达分析（对 alloca
 * 指令的变量构建栈），并回填 phi ，按照 DT 进行 DFS 3、然后死代码消除
 */
PreservedAnalyses Mem2RegPass::run(Function& F, FunctionAnalysisManager& AM)
{
    // errs() << "Mem2RegPass: " << F.getName() << "\n";

    DominatorTree&    DT = AM.getResult<DominatorTreeAnalysis>(F);
    DominanceFrontier DF;
    DF.analyze(DT);

    std::vector<AllocaInst*> allocas;
    for (auto& inst : F.getEntryBlock()) {
        if (auto allocaInst = dyn_cast<AllocaInst>(&inst)) {
            if (isPromotable(allocaInst)) {
                allocas.push_back(allocaInst);
            }
        }
    }
    if (allocas.empty()) return PreservedAnalyses::all();

    std::map<PHINode*, AllocaInst*> phiMap = insertPhiNode(allocas, F, DF);

    std::vector<Instruction*> removeInsts = removeMemInst(phiMap, allocas, F);

    for (Instruction* I : removeInsts) {
        // errs() << "     remove " << *I << "\n";
        I->eraseFromParent();
    }
    return PreservedAnalyses::none();
}

bool Mem2RegPass::isPromotable(AllocaInst* allocaInst)
{
    if (allocaInst->getParent() != &allocaInst->getFunction()->getEntryBlock()) {
        return false;
    }
    for (User* use : allocaInst->users()) {
        if (auto* storeInst = dyn_cast<StoreInst>(use)) {
            if (storeInst->getValueOperand() == allocaInst) {
                return false;
            }
            if (storeInst->getPointerOperand() != allocaInst) {
                return false;
            }
        }
        else if (auto* loadInst = dyn_cast<LoadInst>(use)) {
            if (loadInst->getPointerOperand() != allocaInst) {
                return false;
            }
        }
        else {
            return false;
        }
    }
    return true;
}

std::map<PHINode*, AllocaInst*> Mem2RegPass::insertPhiNode(std::vector<AllocaInst*>& allocas,

                                                           llvm::Function& F, DominanceFrontier& DF)
{
    std::map<AllocaInst*, std::set<BasicBlock*>> allocaDefs;
    std::map<PHINode*, AllocaInst*>              phiMap;

    // get alloca defs (store sth to alloca)
    for (AllocaInst* allocaInst : allocas) {
        for (User* use : allocaInst->users()) {
            if (auto* storeInst = dyn_cast<StoreInst>(use)) {
                if (storeInst->getPointerOperand() == allocaInst) {
                    allocaDefs[allocaInst].insert(storeInst->getParent());
                }
            }
        }
    }

    for (AllocaInst* alloca : allocas) {
        std::set<BasicBlock*>   visited;
        std::queue<BasicBlock*> worklist;

        auto it = allocaDefs.find(alloca);
        if (it != allocaDefs.end()) {
            for (BasicBlock* bb : allocaDefs[alloca]) {
                worklist.push(bb);
            }
        }

        while (!worklist.empty()) {
            BasicBlock* bb = worklist.front();
            worklist.pop();

            auto dfIt = DF.find(bb);
            if (dfIt != DF.end()) {
                for (BasicBlock* df : dfIt->second) {
                    if (visited.find(df) != visited.end()) {
                        continue;
                    }

                    PHINode* phi = PHINode::Create(alloca->getAllocatedType(),
                                                   pred_size(df),
                                                   alloca->getName() + ".phi",
                                                   &df->front());

                    phiMap[phi] = alloca;
                    visited.insert(df);
                    worklist.push(df);
                }
            }
        }
    }
    return phiMap;
}

std::vector<Instruction*> Mem2RegPass::removeMemInst(std::map<PHINode*, AllocaInst*> phiMap,
                                                     std::vector<AllocaInst*>&       allocas,
                                                     llvm::Function&                 F)
{
    std::vector<Instruction*> removeInsts;
    std::set<BasicBlock*>     visited;

    // {block, map(alloca, val)}
    std::queue<std::pair<BasicBlock*, std::map<AllocaInst*, Value*>>> worklist;

    std::map<AllocaInst*, Value*> incomingVals;
    for (AllocaInst* alloca : allocas) {
        incomingVals[alloca] = UndefValue::get(alloca->getAllocatedType());
    }

    worklist.push({&F.getEntryBlock(), incomingVals});

    while (!worklist.empty()) {
        auto [block, currentVals] = worklist.front();
        worklist.pop();

        if (visited.find(block) != visited.end()) {
            continue;
        }
        visited.insert(block);

        for (Instruction& inst : *block) {
            if (auto* allocaInst = dyn_cast<AllocaInst>(&inst)) {
                if (std::find(allocas.begin(), allocas.end(), allocaInst) != allocas.end()) {
                    removeInsts.push_back(&inst);
                }
            }
            else if (auto* loadInst = dyn_cast<LoadInst>(&inst)) {
                if (auto* allocaInst = dyn_cast<AllocaInst>(loadInst->getPointerOperand())) {
                    if (std::find(allocas.begin(), allocas.end(), allocaInst) != allocas.end()) {
                        loadInst->replaceAllUsesWith(currentVals[allocaInst]);
                        removeInsts.push_back(&inst);
                    }
                }
            }
            else if (auto* storeInst = dyn_cast<StoreInst>(&inst)) {
                if (auto* allocaInst = dyn_cast<AllocaInst>(storeInst->getPointerOperand())) {
                    if (std::find(allocas.begin(), allocas.end(), allocaInst) != allocas.end()) {
                        currentVals[allocaInst] = storeInst->getValueOperand();
                        removeInsts.push_back(&inst);
                    }
                }
            }
            else if (auto* phi = dyn_cast<PHINode>(&inst)) {
                auto phiIt = phiMap.find(phi);
                if (phiIt != phiMap.end()) {
                    AllocaInst* alloca  = phiIt->second;
                    currentVals[alloca] = phi;
                }
            }
        }

        for (BasicBlock* succ : successors(block)) {
            for (Instruction& inst : *succ) {
                if (auto* phi = dyn_cast<PHINode>(&inst)) {
                    auto phiIt = phiMap.find(phi);
                    if (phiIt != phiMap.end()) {
                        AllocaInst* alloca = phiIt->second;
                        phi->addIncoming(currentVals[alloca], block);
                    }
                }
                else {
                    break;
                }
            }

            worklist.push({succ, currentVals});
        }
    }
    return removeInsts;
}


}   // namespace llvm