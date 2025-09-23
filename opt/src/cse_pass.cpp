#include "../include/cse_pass.h"
namespace std {
template<> struct hash<llvm::CSEExpression>
{
    size_t operator()(const llvm::CSEExpression& obj) const
    {
        return hash_combine(
            obj.opcode, obj.ty, hash_combine_range(obj.args.begin(), obj.args.end()));
    }
};
}   // namespace std
namespace llvm {

CSEExpression::CSEExpression(Instruction& inst)
{
    if (isa<BinaryOperator>(&inst))
        this->exprType = CSEExpression::ExpressionType::BINARY_OP;
    else if (isa<CmpInst>(&inst))
        this->exprType = CSEExpression::ExpressionType::CMP_INST;
    else if (isa<CastInst>(&inst))
        this->exprType = CSEExpression::ExpressionType::CAST_OP;
    else if (isa<GetElementPtrInst>(&inst))
        this->exprType = CSEExpression::ExpressionType::GETELEMENTPTR_INST;
    else if (isa<LoadInst>(&inst))
        this->exprType = CSEExpression::ExpressionType::LOAD_INST;

    this->opcode        = inst.getOpcode();
    this->ty            = inst.getType();
    this->isCommutative = inst.isCommutative();
    this->args          = {inst.value_op_begin(), inst.value_op_end()};
    this->result        = &inst;

    if (this->isCommutative && args[0] > args[1]) std::swap(args[0], args[1]);

    if (this->exprType == CSEExpression::ExpressionType::CMP_INST) {
        auto cmpInst = cast<CmpInst>(this->result);
        if (cmpInst->getPredicate() > cmpInst->getSwappedPredicate()) {
            this->opcode = cmpInst->getSwappedPredicate();
            std::swap(args[0], args[1]);
        }
        else {
            this->opcode = cmpInst->getPredicate();
        }
    }
}

bool CSEExpression::operator==(const CSEExpression& other) const
{
    if (exprType != other.exprType || ty != other.ty) return false;
    if (opcode == other.opcode && ty == other.ty && args == other.args) return true;
    return false;
}

PreservedAnalyses CSEPass::run(Function& F, FunctionAnalysisManager& AM)
{
    DominatorTree&                                  DT = AM.getResult<DominatorTreeAnalysis>(F);
    bool                                            changed = false;
    std::unordered_map<CSEExpression, Instruction*> map;
    for (auto& block : F) {
        std::vector<Instruction*> toRemove;
        for (auto& inst : block) {
            if (!canCSE(&inst)) continue;
            CSEExpression cseExpr{inst};

            auto iter = map.find(cseExpr);
            if (iter != map.end()) {
                auto existingInst = iter->second;
                if (DT.dominates(existingInst, &inst)) {
                    inst.replaceAllUsesWith(existingInst);
                    toRemove.push_back(&inst);
                    changed = true;
                    continue;
                }
            }

            map[cseExpr] = &inst;
        }
        for (Instruction* inst : toRemove) {
            inst->eraseFromParent();
        }
    }
    return changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
}   // namespace llvm