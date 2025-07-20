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
    int offset = 1;
    for (const auto& param : this->classAllParams[decl.name]) {
        const Expression* initExpr  = this->getParamInitExpr(param);
        std::string       paramName = this->getParamName(param);
        if (initExpr != nullptr) {
            auto initValue = generateExpression(*initExpr);
            auto ptr =
                this->builder->CreateStructGEP(this->generateType(this->currClass->name, false),
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

    llvm::Type*  classType = this->generateType(decl.name, false);
    llvm::Value* self      = this->getCurrFunc()->getArg(0);

    if (!decl.baseClass.empty()) {
        auto baseClass       = this->classTable.find(decl.baseClass);
        auto baseClassName   = baseClass->name;
        auto castToBaseClass = this->builder->CreateBitCast(
            self, llvm::PointerType::get(this->generateType(baseClassName, false), 0));

        std::vector<llvm::Value*> constructorArgs = {castToBaseClass};
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
    constructorArgs.push_back(mallocResult);

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

    llvm::Type*  classType = this->generateType(className, false);
    llvm::Value* self      = this->getCurrFunc()->getArg(0);

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
        llvm::Type* returnType = this->generateType(*decl.returnType, false);
        this->retVal           = this->allocateStackVariable("retval", returnType);
    }

    this->retBB = llvm::BasicBlock::Create(*this->context, "return");

    int paramOffset = this->currClass == nullptr ? 0 : 1;
    for (const auto& param : decl.parameters) {
        llvm::Type*  paramType = this->generateType(*param.type, false);
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
        llvm::Type*  returnType  = this->generateType(*decl.returnType, false);
        llvm::Value* returnValue = builder->CreateLoad(returnType, retVal, "return_value");
        builder->CreateRet(returnValue);
    }

    this->valueTable.exitScope();
}
