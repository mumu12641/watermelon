#include "../include/mem2reg_pass.h"

#include <map>
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
    errs() << "Mem2RegPass: " << F.getName() << "\n";

    DominatorTree&    DT = AM.getResult<DominatorTreeAnalysis>(F);
    DominanceFrontier DF;
    DF.analyze(DT);

    std::vector<AllocaInst*> allocas;
    for (auto& block : F) {
        for (auto& inst : block) {
            if (auto allocaInst = dyn_cast<AllocaInst>(&inst)) {
                allocas.push_back(allocaInst);
            }
        }
    }

    std::map<PHINode*, AllocaInst*> phiMap = insertPhiNode(F, DF);

    std::vector<Instruction*> removeInsts = removeMemInst(phiMap, DT);

    for (Instruction* I : removeInsts) {
        errs() << "     remove " << *I << "\n";
        changed = true;
        I->eraseFromParent();
    }
    return changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

std::map<PHINode*, AllocaInst*> Mem2RegPass::insertPhiNode(llvm::Function& F, DominanceFrontier& DF)
{
    std::vector<AllocaInst*> allocas;
    for (auto& block : F) {
        for (auto& inst : block) {
            if (auto allocaInst = dyn_cast<AllocaInst>(&inst)) {
                allocas.push_back(allocaInst);
            }
        }
    }
    std::map<AllocaInst*, std::set<std::pair<BasicBlock*, Value*>>> varDefMap;
    std::map<AllocaInst*, std::set<BasicBlock*>>                    varUseMap;
    std::map<std::pair<AllocaInst*, BasicBlock*>, std::set<std::pair<BasicBlock*, Value*>>>
                                    phiNumMap;
    std::map<PHINode*, AllocaInst*> phiMap;

    // collect def&use message
    for (auto& block : F) {
        for (auto& inst : block) {
            if (auto storeInst = dyn_cast<StoreInst>(&inst)) {
                if (auto allocaInst = dyn_cast<AllocaInst>(storeInst->getPointerOperand())) {
                    varDefMap[allocaInst].insert(
                        {storeInst->getParent(), storeInst->getValueOperand()});
                    varUseMap[allocaInst].insert(storeInst->getParent());
                }
            }
            else if (auto loadInst = dyn_cast<LoadInst>(&inst)) {
                if (auto allocaInst = dyn_cast<AllocaInst>(loadInst->getPointerOperand())) {
                    varUseMap[allocaInst].insert(loadInst->getParent());
                }
            }
        }
    }

    // if defBlock's frontier in useBlockSet, then the frontier need phi for defVal
    for (auto allocaInst : allocas) {
        auto& defBlockSet = varDefMap[allocaInst];
        auto& useBlockSet = varUseMap[allocaInst];
        if (useBlockSet.size() <= 1) continue;
        for (auto& [defBlock, defVal] : defBlockSet) {
            auto dfIter = DF.find(defBlock);
            if (dfIter != DF.end()) {
                for (auto frontier : dfIter->second) {
                    if (useBlockSet.count(frontier)) {
                        phiNumMap[{allocaInst, frontier}].insert({defBlock, defVal});
                        changed = true;
                    }
                }
            }
        }
    }

    for (auto& [pair, defBlockSet] : phiNumMap) {
        auto [allocaInst, frontier] = pair;
        if (defBlockSet.size() > 1) {
            PHINode* phi = PHINode::Create(
                allocaInst->getAllocatedType(), defBlockSet.size(), "", &frontier->front());
            phiMap[phi] = allocaInst;
            for (auto& [block, val] : defBlockSet) {
                phi->addIncoming(val, block);
            }
        }
    }
    return phiMap;
}

std::vector<Instruction*> Mem2RegPass::removeMemInst(std::map<PHINode*, AllocaInst*> phiMap,
                                                     DominatorTree&                  DT)
{
    std::map<AllocaInst*, std::stack<Value*>> valueStackMap;
    std::vector<Instruction*>                 removeInsts;
    DomTreeNode*                              root = DT.getRootNode();
    for (auto iter = df_begin(root), end = df_end(root); iter != end; ++iter) {
        auto                       node  = *iter;
        BasicBlock*                block = iter->getBlock();
        std::map<AllocaInst*, int> popMap;
        for (auto& inst : *block) {

            if (auto allocaInst = dyn_cast<AllocaInst>(&inst)) {
                valueStackMap[allocaInst] = std::stack<Value*>();
                removeInsts.push_back(allocaInst);
            }
            else if (auto storeInst = dyn_cast<StoreInst>(&inst)) {
                auto pointer = storeInst->getPointerOperand();
                if (auto allocaInst = dyn_cast<AllocaInst>(pointer)) {
                    if (valueStackMap.count(allocaInst)) {
                        popMap[allocaInst] += 1;
                        valueStackMap[allocaInst].push(storeInst->getValueOperand());
                        removeInsts.push_back(storeInst);
                    }
                }
            }
            else if (auto loadInst = dyn_cast<LoadInst>(&inst)) {
                auto pointer = loadInst->getPointerOperand();
                if (auto allocaInst = dyn_cast<AllocaInst>(pointer)) {
                    if (valueStackMap.count(allocaInst)) {
                        loadInst->replaceAllUsesWith(valueStackMap[allocaInst].top());
                        removeInsts.push_back(loadInst);
                    }
                }
            }
            else if (auto phiInst = dyn_cast<PHINode>(&inst)) {
                auto allocaInst = phiMap[phiInst];
                if (valueStackMap.count(allocaInst)) {
                    popMap[allocaInst] += 1;
                    valueStackMap[allocaInst].push(phiInst);
                }
            }
        }
        if (node->isLeaf()) {
            for (auto& [allocaInst, num] : popMap) {
                for (int i = 0; i < num; i++) {
                    valueStackMap[allocaInst].pop();
                }
            }
        }
    }
    return removeInsts;
}


}   // namespace llvm