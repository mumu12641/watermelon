#include "../include/ir/ir.hpp"
#include "../include/utils/format.hpp"

void IRGen::generateDeclaration(const Declaration& decl)
{
    if (const auto* classDecl = dynamic_cast<const ClassDeclaration*>(&decl)) {
        generateClassDeclaration(*classDecl);
    }
    else if (const auto* enumDecl = dynamic_cast<const EnumDeclaration*>(&decl)) {
        generateEnumDeclaration(*enumDecl);
    }
    else if (const auto* funcDecl = dynamic_cast<const FunctionDeclaration*>(&decl)) {
        generateFunctionDeclaration(*funcDecl);
    }
}

void IRGen::generateClassDeclaration(const ClassDeclaration& decl)
{
    this->valueTable.enterScope(decl.name);
    this->currClass = &decl;
    int offset      = 1;
    for (const auto& param : this->classAllParams[decl.name]) {
        std::string paramName = this->getParamName(param);
        this->valueTable.add(paramName, IRValue(offset++));
    }
    this->generateClassMallocInit(decl);
    this->generateClassConstructor(decl);
    this->generateClassBuiltinInit(decl);
    for (const auto& member : decl.members) {
        if (const auto method = dynamic_cast<const MethodMember*>(member.get())) {
            generateFunctionDeclaration(*method->function);
        }
        else if (const auto init = dynamic_cast<const InitBlockMember*>(member.get())) {
            // TODO include all params, parents' property, baseClassConstructor
        }
    }
    this->currClass = nullptr;
    this->valueTable.exitScope();
}

void IRGen::generateClassBuiltinInit(const ClassDeclaration& decl)
{

    this->currFuncName = "builtin_init";
    this->valueTable.enterScope(Format("{0}_{1}", decl.name, this->currFuncName));
    llvm::Function* function = this->getCurrFunc();
    auto            entryBB  = llvm::BasicBlock::Create(*this->context, "entry", function);
    this->builder->SetInsertPoint(entryBB);
    int offset = 1;
    for (const auto& param : this->classAllParams[decl.name]) {
        const Expression* initExpr  = this->getParamInitExpr(param);
        std::string       paramName = this->getParamName(param);
        if (initExpr != nullptr) {
            auto initValue = generateExpression(*initExpr);
            auto ptr = this->builder->CreateStructGEP(this->generateType(this->currClass->name),
                                                      this->getCurrFunc()->getArg(0),
                                                      offset,
                                                      Format("{0}_ptr", paramName));
            this->builder->CreateStore(initValue, ptr);
        }
        offset++;
    }
    builder->CreateRetVoid();
    this->valueTable.exitScope();
}

void IRGen::generateClassConstructor(const ClassDeclaration& decl)
{
    this->currFuncName = "constructor";
    this->valueTable.enterScope(Format("{0}_{1}", decl.name, this->currFuncName));
    llvm::Function* function = this->getCurrFunc();
    auto            entryBB  = llvm::BasicBlock::Create(*this->context, "entry", function);
    this->builder->SetInsertPoint(entryBB);

    // TODO: 这里应该是分配空间，然后把指针返回，所以 constructor 的参数也不需要 self 了
    llvm::Type*  classType = this->generateType(decl.name);
    llvm::Value* self      = this->getCurrFunc()->getArg(0);
    // uint64_t    typeSize   = this->dataLayout->getTypeAllocSize(classType);
    // auto        sizeValue  = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), typeSize);
    // auto        mallocCall = this->builder->CreateCall(this->methodMap["malloc"], {sizeValue});
    // auto mallocResult = builder->CreateBitCast(mallocCall, llvm::PointerType::get(classType, 0));

    // this->builder->CreateCall(this->module->getFunction(Format("{0}_builtin_init", decl.name)),
    //                           {mallocResult});

    if (!decl.baseClass.empty()) {
        // TODO: 如果你根据上面那样做了，那你调用 baseclass 的 constructor 的时候又要 malloc 一次？
        auto baseClassName   = this->classTable.find(decl.baseClass)->name;
        auto castToBaseClass = this->builder->CreateBitCast(
            self, llvm::PointerType::get(this->generateType(baseClassName), 0));

        std::vector<llvm::Value*> constructorArgs = {castToBaseClass};
        for (const auto& param : decl.baseConstructorArgs) {
            auto value = generateExpression(*param);
            constructorArgs.push_back(value);
        }
        auto callBaseConstructor = this->builder->CreateCall(
            this->module->getFunction(Format("{0}_constructor", baseClassName)), constructorArgs);
        self =
            this->builder->CreateBitCast(callBaseConstructor, llvm::PointerType::get(classType, 0));
    }

    llvm::Value* undef = llvm::UndefValue::get(this->int32Ty);
    this->allocaInsertPoint =
        new llvm::BitCastInst(undef, undef->getType(), "alloca.point", entryBB);

    int paramOffset = 1;
    for (const auto& param : decl.constructorParameters) {
        llvm::Type*  paramType = this->generateType(*param.type);
        llvm::Value* paramVar  = allocateStackVariable(param.name, paramType);
        llvm::Value* argValue  = function->getArg(paramOffset);
        this->builder->CreateStore(argValue, paramVar, false);
        this->valueTable.add(param.name, IRValue(paramVar));

        auto ptr = this->builder->CreateStructGEP(
            classType, self, paramOffset, Format("{0}_ptr", param.name));
        this->builder->CreateStore(ptr, argValue);
        paramOffset++;
    }

    this->allocaInsertPoint->eraseFromParent();
    this->allocaInsertPoint = nullptr;

    auto selfDefinedInitName = Format("{0}_self_defined_init", decl.name);
    if (this->methodMap.count(selfDefinedInitName)) {
        this->builder->CreateCall(this->module->getFunction(selfDefinedInitName), {self});
    }


    this->builder->CreateRet(self);
    this->valueTable.exitScope();
}

void IRGen::generateClassMallocInit(const ClassDeclaration& decl)
{
    this->currFuncName = "malloc_init";
    this->valueTable.enterScope(Format("{0}_{1}", decl.name, this->currFuncName));
    llvm::Function* function = this->getCurrFunc();
    auto            entryBB  = llvm::BasicBlock::Create(*this->context, "entry", function);
    this->builder->SetInsertPoint(entryBB);

    llvm::Type* classType  = this->generateType(decl.name);
    uint64_t    typeSize   = this->dataLayout->getTypeAllocSize(classType);
    auto        sizeValue  = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), typeSize);
    auto        mallocCall = this->builder->CreateCall(this->methodMap["malloc"], {sizeValue});
    auto mallocResult = builder->CreateBitCast(mallocCall, llvm::PointerType::get(classType, 0));

    std::vector<llvm::Value*> constructorArgs;
    constructorArgs.push_back(mallocResult);

    for (unsigned i = 0; i < function->arg_size(); ++i) {
        constructorArgs.push_back(function->getArg(i));
    }

    auto constructorFunc = this->module->getFunction(Format("{0}_constructor", decl.name));
    auto self            = this->builder->CreateCall(constructorFunc, constructorArgs);

    this->builder->CreateRet(self);
    this->valueTable.exitScope();
}


void IRGen::generateEnumDeclaration(const EnumDeclaration& decl)
{
    throw "Not yet implemented generateEnumDeclaration";
}

void IRGen::generateFunctionDeclaration(const FunctionDeclaration& decl)
{
    this->currFuncName = decl.name;
    this->valueTable.enterScope(decl.name);

    llvm::Function* function = this->getCurrFunc();
    auto            entryBB  = llvm::BasicBlock::Create(*this->context, "entry", function);
    this->builder->SetInsertPoint(entryBB);

    llvm::Value* undef = llvm::UndefValue::get(this->int32Ty);
    this->allocaInsertPoint =
        new llvm::BitCastInst(undef, undef->getType(), "alloca.point", entryBB);

    bool isVoid = decl.returnType->isVoid();
    if (!isVoid) {
        llvm::Type* returnType = this->generateType(*decl.returnType);
        this->retVal           = this->allocateStackVariable("retval", returnType);
    }

    this->retBB = llvm::BasicBlock::Create(*this->context, "return");

    int paramOffset = this->currClass == nullptr ? 0 : 1;
    for (const auto& param : decl.parameters) {
        llvm::Type*  paramType = this->generateType(*param.type);
        llvm::Value* paramVar  = allocateStackVariable(param.name, paramType);
        llvm::Value* argValue  = function->getArg(paramOffset++);
        this->builder->CreateStore(argValue, paramVar, false);
        this->valueTable.add(param.name, IRValue(paramVar));
    }

    this->generateStatement(*decl.body);

    // if (this->retBB->hasNPredecessorsOrMore(1)) {
    //     this->builder->CreateBr(this->retBB);
    //     this->retBB->insertInto(function);
    //     this->builder->SetInsertPoint(this->retBB);
    // }

    this->retBB->insertInto(function);
    this->builder->SetInsertPoint(this->retBB);
    this->allocaInsertPoint->eraseFromParent();
    this->allocaInsertPoint = nullptr;

    if (isVoid) {
        builder->CreateRetVoid();
    }
    else {
        llvm::Type*  returnType  = this->generateType(*decl.returnType);
        llvm::Value* returnValue = builder->CreateLoad(returnType, retVal, "return_value");
        builder->CreateRet(returnValue);
    }

    this->valueTable.exitScope();
}
