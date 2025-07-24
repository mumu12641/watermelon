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
        this->valueTable.add(paramName, IRValue(offset));
        offset++;
    }

    this->generateClassMallocInit(decl);
    this->generateClassBuiltinInit(decl);
    this->generateClassConstructor(decl);
    for (const auto& member : decl.members) {
        if (const auto method = dynamic_cast<const MethodMember*>(member.get())) {
            generateFunctionDeclaration(*method->function);
        }
        else if (const auto init = dynamic_cast<const InitBlockMember*>(member.get())) {
            this->generateClassSelfDefinedInit(*init, decl.name);
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

    auto self = this->builder->CreateBitCast(
        this->getCurrFunc()->getArg(0), this->generateType(decl.name, true), "self");

    int offset = 1;
    for (const auto& param : this->classAllParams[decl.name]) {
        const Expression* initExpr  = this->getParamInitExpr(param);
        std::string       paramName = this->getParamName(param);
        if (initExpr != nullptr) {
            auto initValue = generateExpression(*initExpr);
            auto ptr =
                this->builder->CreateStructGEP(this->generateType(this->currClass->name, false),
                                               self,
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

    llvm::Type*  classType = this->generateType(decl.name, false);
    llvm::Value* selfI8    = this->getCurrFunc()->getArg(0);
    llvm::Value* self =
        this->builder->CreateBitCast(selfI8, this->generateType(decl.name, true), "self");

    if (!decl.baseClass.empty()) {
        auto baseClass       = this->classTable.find(decl.baseClass);
        auto baseClassName   = baseClass->name;
        auto castToBaseClass = this->builder->CreateBitCast(
            self, llvm::PointerType::get(this->generateType(baseClassName, false), 0));

        std::vector<llvm::Value*> constructorArgs = {selfI8};
        for (const auto& param : decl.baseConstructorArgs) {
            auto value = generateExpression(*param);
            constructorArgs.push_back(value);
        }

        auto diff = baseClass->constructorParameters.size() - (constructorArgs.size() - 1);
        if (diff > 0) {
            for (int i = 0; i < diff; i++) {
                auto value = generateExpression(
                    *baseClass->constructorParameters[i + (constructorArgs.size() - 1)]
                         .defaultValue);
                constructorArgs.push_back(value);
            }
        }
        auto callBaseConstructor = this->builder->CreateCall(
            this->module->getFunction(Format("{0}_constructor", baseClassName)), constructorArgs);
        self =
            this->builder->CreateBitCast(callBaseConstructor, llvm::PointerType::get(classType, 0));
    }

    int paramOffset = 1;
    for (const auto& param : decl.constructorParameters) {
        auto         ptr      = this->builder->CreateStructGEP(this->generateType(decl.name, false),
                                                  self,
                                                  this->valueTable.find(param.name)->getOffset(),
                                                  Format("{0}_ptr", param.name));
        llvm::Value* argValue = function->getArg(paramOffset);
        this->builder->CreateStore(argValue, ptr);
        paramOffset++;
    }
    auto builtinInitName = Format("{0}_builtin_init", decl.name);
    if (this->methodMap.count(builtinInitName)) {
        this->builder->CreateCall(this->module->getFunction(builtinInitName), {selfI8});
    }
    auto selfDefinedInitName = Format("{0}_self_defined_init", decl.name);
    if (this->methodMap.count(selfDefinedInitName)) {
        this->builder->CreateCall(this->module->getFunction(selfDefinedInitName), {selfI8});
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

    llvm::Type* classType  = this->generateType(decl.name, false);
    uint64_t    typeSize   = this->dataLayout->getTypeAllocSize(classType);
    auto        sizeValue  = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*context), typeSize);
    auto        mallocCall = this->builder->CreateCall(this->methodMap["malloc"], {sizeValue});
    auto mallocResult = builder->CreateBitCast(mallocCall, llvm::PointerType::get(classType, 0));

    std::string vTableName = Format("vTable_{0}", decl.name);
    auto        ptr    = this->builder->CreateStructGEP(classType, mallocResult, 0, "vtable_ptr");
    auto        vTable = this->builder->CreateLoad(
        this->vTableTypes[vTableName], this->vTableVars[vTableName], "vtable_load");
    this->builder->CreateStore(vTable, ptr);

    std::vector<llvm::Value*> constructorArgs;
    constructorArgs.push_back(mallocCall);

    for (unsigned i = 0; i < function->arg_size(); ++i) {
        constructorArgs.push_back(function->getArg(i));
    }

    auto constructorFunc = this->module->getFunction(Format("{0}_constructor", decl.name));
    auto self            = this->builder->CreateCall(constructorFunc, constructorArgs);

    this->builder->CreateRet(self);
    this->valueTable.exitScope();
}

void IRGen::generateClassSelfDefinedInit(const InitBlockMember& init, const std::string& className)
{
    this->currFuncName = "self_defined_init";
    this->valueTable.enterScope(Format("{0}_{1}", className, this->currFuncName));
    llvm::Function* function = this->getCurrFunc();
    auto            entryBB  = llvm::BasicBlock::Create(*this->context, "entry", function);
    this->builder->SetInsertPoint(entryBB);

    auto selfVal = this->builder->CreateBitCast(
        function->getArg(0), this->generateType(className, true), "self");
    this->valueTable.add("self", IRValue(selfVal));

    generateBlockStatement(*init.block);

    this->builder->CreateRetVoid();
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
        llvm::Type* returnType = this->generateType(*decl.returnType, true);
        this->retVal           = this->allocateStackVariable("retval", returnType);
    }

    this->retBB = llvm::BasicBlock::Create(*this->context, "return");

    int paramOffset = 0;
    if (this->currClass) {
        llvm::Type*  selfType = this->generateType(this->currClass->name, true);
        llvm::Value* selfVar  = allocateStackVariable("self", selfType);
        llvm::Value* selfArg =
            this->builder->CreateBitCast(function->getArg(paramOffset++), selfType, "self");
        this->builder->CreateStore(selfArg, selfVar, false);
        this->valueTable.add("self", IRValue(selfVar));
    }
    for (const auto& param : decl.parameters) {
        llvm::Type*  paramType = this->generateType(*param.type, true);
        llvm::Value* paramVar  = allocateStackVariable(param.name, paramType);
        llvm::Value* argValue  = function->getArg(paramOffset++);
        this->builder->CreateStore(argValue, paramVar, false);
        this->valueTable.add(param.name, IRValue(paramVar));
    }

    this->generateStatement(*decl.body);

    this->retBB->insertInto(function);
    this->builder->SetInsertPoint(this->retBB);
    this->allocaInsertPoint->eraseFromParent();
    this->allocaInsertPoint = nullptr;

    if (isVoid) {
        builder->CreateRetVoid();
    }
    else {
        llvm::Type*  returnType  = this->generateType(*decl.returnType, true);
        llvm::Value* returnValue = builder->CreateLoad(returnType, retVal, "return_value");
        builder->CreateRet(returnValue);
    }

    this->valueTable.exitScope();
}
