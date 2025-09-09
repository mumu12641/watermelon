#include "../include/ir/ir.hpp"

#include "../include/utils/format.hpp"

void IRValueScope::add(const std::string& key, IRValue value)
{
    map.insert_or_assign(key, value);
}
const IRValue* IRValueScope::find(const std::string& key)
{
    auto it = map.find(key);
    if (it != map.end()) return &it->second;

    return nullptr;
}

void IRValueTable::enterScope(const std::string& name)
{
    scopes.emplace_back(IRValueScope(name));
}

void IRValueTable::exitScope()
{
    if (!scopes.empty()) scopes.pop_back();
}

void IRValueTable::add(const std::string& key, IRValue value)
{
    if (!scopes.empty()) scopes.back().add(key, value);
}

const IRValue* IRValueTable::find(const std::string& key)
{
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        const auto* info = it->find(key);
        if (info) return info;
    }
    return nullptr;
}

void IRValueTable::debug() const
{
    std::cout << "IRValueTable Debug Information:\n";
    for (size_t i = 0; i < scopes.size(); ++i) {
        const auto& scope = scopes[i];
        std::cout << "Scope " << i << " " << scope.getName() << ":\n";
        for (const auto& [key, value] : scope.getMap()) {
            const bool             isProperty = (value.getKind() == IRValueKind::PROPERTY);
            const std::string_view kind       = isProperty ? "property" : "function param";
            std::cout << "  Key: " << key << ", Symbol kind: " << kind;
            if (isProperty) {
                std::cout << ", Symbol offset: " << value.getOffset();
            }
            std::cout << '\n';
        }
    }
    std::cout << '\n';
}


std::unique_ptr<llvm::Module> IRGen::generateIR()
{
    this->valueTable.enterScope("global");
    this->setupClasses();
    this->setupFunctions();
    for (const auto& decl : program->declarations) {
        this->generateDeclaration(*decl);
    }
    this->valueTable.exitScope();
    return std::move(this->module);
}

void IRGen::declareClasses()
{
    for (const auto& decl : program->declarations) {
        if (const ClassDeclaration* classDecl = dynamic_cast<const ClassDeclaration*>(decl.get())) {
            llvm::StructType* classType = llvm::StructType::create(*this->context, classDecl->name);
            this->typeMap[Type::classType(classDecl->name)] = classType;
        }
    }
}

void IRGen::buildVTables()
{
    for (const auto& decl : program->declarations) {
        const ClassDeclaration* classDecl = dynamic_cast<const ClassDeclaration*>(decl.get());
        if (!classDecl) continue;

        std::string                  className  = classDecl->name;
        std::string                  vTableName = Format("vTable_{0}", className);
        std::vector<llvm::Type*>     vTableMethods;
        std::vector<llvm::Constant*> vTableInitializers;

        this->addVTableMethod(vTableMethods,
                              vTableInitializers,
                              Format("{0}_builtin_init", className),
                              llvm::FunctionType::get(voidTy, {int8PtrTy}, false));

        std::vector<llvm::Type*> constructParamTypes = {int8PtrTy};
        for (const auto& constructParam : classDecl->constructorParameters) {
            constructParamTypes.emplace_back(this->generateType(*constructParam.type, false));
        }
        this->addVTableMethod(vTableMethods,
                              vTableInitializers,
                              Format("{0}_constructor", className),
                              llvm::FunctionType::get(
                                  this->generateType(className, true), constructParamTypes, false));
        constructParamTypes.erase(constructParamTypes.begin());
        this->addVTableMethod(vTableMethods,
                              vTableInitializers,
                              Format("{0}_malloc_init", className),
                              llvm::FunctionType::get(
                                  this->generateType(className, true), constructParamTypes, false));


        std::map<std::string, std::pair<std::string, llvm::FunctionType*>> inheritMethodMap;

        const auto* inheritanceChain = this->classTable.getInheritMap(classDecl->name);
        for (auto cls = inheritanceChain->rbegin(); cls != inheritanceChain->rend(); ++cls) {
            for (const auto& member : (*cls)->members) {
                if (const auto* method = dynamic_cast<const MethodMember*>(member.get())) {
                    std::string methodKey      = method->getName();
                    std::string fullMethodName = Format("{0}_{1}", (*cls)->name, method->getName());

                    std::vector<llvm::Type*> paramTypes = {int8PtrTy};

                    for (const auto& param : method->function->parameters) {
                        paramTypes.push_back(this->generateType(*param.type, false));
                    }

                    llvm::FunctionType* funcType = llvm::FunctionType::get(
                        this->generateType(*method->function->returnType, false),
                        paramTypes,
                        false);
                    inheritMethodMap[methodKey] = {fullMethodName, funcType};
                }
            }
        }
        for (const auto& member : classDecl->members) {
            if (const auto* method = dynamic_cast<const MethodMember*>(member.get())) {
                std::string methodKey      = method->getName();
                std::string fullMethodName = Format("{0}_{1}", className, method->getName());

                std::vector<llvm::Type*> paramTypes = {int8PtrTy};

                for (const auto& param : method->function->parameters) {
                    paramTypes.push_back(this->generateType(*param.type, false));
                }

                llvm::FunctionType* funcType = llvm::FunctionType::get(
                    this->generateType(*method->function->returnType, false), paramTypes, false);
                inheritMethodMap[methodKey] = {fullMethodName, funcType};
            }
            else if (const auto* init = dynamic_cast<const InitBlockMember*>(member.get())) {
                std::string         initMethodName = Format("{0}_self_defined_init", className);
                llvm::FunctionType* funcType = llvm::FunctionType::get(voidTy, {int8PtrTy}, false);
                this->addVTableMethod(vTableMethods, vTableInitializers, initMethodName, funcType);
            }
        }
        size_t vTableOffset = 3;
        for (const auto& [methodKey, methodInfo] : inheritMethodMap) {
            const auto& [fullMethodName, funcType]             = methodInfo;
            this->vTableOffsetMap[classDecl->name + methodKey] = vTableOffset;
            this->addVTableMethod(vTableMethods, vTableInitializers, fullMethodName, funcType);
            vTableOffset++;
        }

        auto vTableType     = llvm::StructType::create(*this->context, vTableMethods, vTableName);
        auto vTableConstant = llvm::ConstantStruct::get(vTableType, vTableInitializers);

        this->vTableVars[vTableName]  = new llvm::GlobalVariable(*this->module,
                                                                vTableType,
                                                                false,
                                                                llvm::GlobalValue::ExternalLinkage,
                                                                vTableConstant,
                                                                vTableName);
        this->vTableTypes[vTableName] = vTableType;
    }
}
void IRGen::addVTableMethod(std::vector<llvm::Type*>&     vTableMethods,
                            std::vector<llvm::Constant*>& vTableInitializers,
                            const std::string& methodName, llvm::FunctionType* funcType)
{
    llvm::Function* function;
    vTableMethods.push_back(llvm::PointerType::getUnqual(funcType));
    if (!this->methodMap.count(methodName)) {
        function = llvm::Function::Create(
            funcType, llvm::Function::ExternalLinkage, methodName, this->module.get());
        this->methodMap[methodName]     = function;
        this->methodTypeMap[methodName] = funcType;
    }
    else {
        function = this->methodMap[methodName];
    }
    vTableInitializers.push_back(function);
}

void IRGen::defineClasses()
{
    for (const auto& decl : program->declarations) {
        if (const ClassDeclaration* classDecl = dynamic_cast<const ClassDeclaration*>(decl.get())) {
            auto it = this->typeMap.find(Type::classType(classDecl->name));
            if (it != this->typeMap.end()) {
                const auto* inheritanceChain = this->classTable.getInheritMap(classDecl->name);
                std::string vTableName       = Format("vTable_{0}", classDecl->name);
                llvm::StructType*        structType = static_cast<llvm::StructType*>(it->second);
                std::vector<llvm::Type*> fieldTypes = {
                    llvm::PointerType::getUnqual(this->vTableTypes[vTableName])};
                std::vector<std::variant<const FunctionParameter*, const PropertyMember*>>
                    allParams;

                for (auto cls = inheritanceChain->rbegin(); cls != inheritanceChain->rend();
                     ++cls) {
                    for (const auto& param : (*cls)->constructorParameters) {
                        allParams.push_back(&param);
                    }
                    for (const auto& member : (*cls)->members) {
                        if (const auto property =
                                dynamic_cast<const PropertyMember*>(member.get())) {
                            allParams.push_back(property);
                        }
                    }
                }
                for (const auto& constructorParam : classDecl->constructorParameters) {
                    allParams.push_back(&constructorParam);
                }
                for (const auto& member : classDecl->members) {
                    if (const auto property = dynamic_cast<const PropertyMember*>(member.get())) {
                        allParams.push_back(property);
                    }
                }
                for (const auto& param : allParams) {
                    fieldTypes.emplace_back(this->getParamType(param));
                }
                if (!fieldTypes.empty() && structType->isOpaque()) {
                    structType->setBody(fieldTypes);
                }
                this->classAllParams[classDecl->name] = std::move(allParams);
            }
        }
    }
}

void IRGen::setupClasses()
{
    this->declareClasses();
    this->buildVTables();
    this->defineClasses();
    for (const auto& decl : program->declarations) {
        if (const auto* classDecl = dynamic_cast<const ClassDeclaration*>(decl.get())) {
            int offset = 1;
            for (const auto& param : this->classAllParams[classDecl->name]) {
                std::string paramName = this->getParamName(param);
                this->valueTable.add(Format("{0}_{1}", classDecl->name, paramName),
                                     IRValue(offset));
                offset++;
            }
        }
    }
}

void IRGen::setupFunctions()
{
    for (const auto& decl : program->declarations) {
        if (const auto funcDecl = dynamic_cast<const FunctionDeclaration*>(decl.get())) {
            auto funcName = funcDecl->name == "main" ? "builtin_main" : funcDecl->name;
            std::vector<llvm::Type*> paramTypes = {};
            for (const auto& param : funcDecl->parameters) {
                paramTypes.emplace_back(this->generateType(*param.type, true));
            }
            auto m = llvm::FunctionType::get(
                this->generateType(*funcDecl->returnType, true), paramTypes, false);
            this->methodMap[funcName] = llvm::Function::Create(
                m, llvm::Function::ExternalLinkage, funcName, this->module.get());
        }
    }
    auto m = llvm::FunctionType::get(int8PtrTy, {int64Ty}, false);
    this->methodMap["malloc"] =
        llvm::Function::Create(m, llvm::Function::ExternalLinkage, "gc_alloc", *this->module);
}

llvm::Type* IRGen::generateType(const Type& type, bool ptr)
{
    switch (type.kind) {
        case Type::Kind::INT: return builder->getInt32Ty();
        case Type::Kind::STR: return builder->getInt8PtrTy();
        case Type::Kind::BOOL: return builder->getInt1Ty();
        case Type::Kind::FLOAT: return builder->getFloatTy();
        case Type::Kind::VOID: return builder->getVoidTy();
        case Type::Kind::CLASS:
        {
            auto it = this->typeMap.find(type);
            if (it == this->typeMap.end()) {
                return nullptr;
            }
            return ptr ? llvm::PointerType::getUnqual(it->second) : it->second;
            // return llvm::PointerType::getUnqual(it->second);
        }
        default: break;
    }
    return nullptr;
}

llvm::Type* IRGen::generateType(const std::string& type, bool ptr)
{
    if (type == "int")
        return builder->getInt32Ty();
    else if (type == "String")
        return builder->getInt8PtrTy();
    else if (type == "bool")
        return builder->getInt1Ty();
    else if (type == "float")
        return builder->getDoubleTy();
    else if (type == "void")
        return builder->getVoidTy();
    else {
        Type classType = Type::classType(type);
        auto it        = this->typeMap.find(classType);
        if (it == this->typeMap.end()) {
            return nullptr;
        }
        return ptr ? llvm::PointerType::getUnqual(it->second) : it->second;
        // return llvm::PointerType::getUnqual(it->second);
    }
}

llvm::AllocaInst* IRGen::allocateStackVariable(const std::string_view identifier, llvm::Type* type)
{
    llvm::IRBuilder<> tmpBuilder(*context);
    tmpBuilder.SetInsertPoint(allocaInsertPoint);
    return tmpBuilder.CreateAlloca(type, nullptr, identifier);
}

llvm::Type* IRGen::getParamType(
    const std::variant<const FunctionParameter*, const PropertyMember*>& param)
{
    if (auto funcParamPtr = std::get_if<const FunctionParameter*>(&param)) {
        return this->generateType(*(*funcParamPtr)->type, true);
    }
    else if (auto propertyPtr = std::get_if<const PropertyMember*>(&param)) {
        return this->generateType((*propertyPtr)->getType(), true);
    }
    return nullptr;
}

std::string IRGen::getParamName(
    const std::variant<const FunctionParameter*, const PropertyMember*>& param)
{
    if (auto funcParamPtr = std::get_if<const FunctionParameter*>(&param)) {
        return (*funcParamPtr)->name;
    }
    else if (auto propertyPtr = std::get_if<const PropertyMember*>(&param)) {
        return (*propertyPtr)->getName();
    }
    return "";
}

const Expression* IRGen::getParamInitExpr(
    const std::variant<const FunctionParameter*, const PropertyMember*>& param)
{
    if (auto funcParamPtr = std::get_if<const FunctionParameter*>(&param)) {
        return (*funcParamPtr)->defaultValue.get();
    }
    else if (auto propertyPtr = std::get_if<const PropertyMember*>(&param)) {
        return (*propertyPtr)->initializer.get();
    }
    return nullptr;
}