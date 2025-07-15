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
        std::cout << "Scope " << i << " " << scopes[i].getName() << ":\n";
        for (const auto& pair : scopes[i].getMap()) {
            std::string kind =
                pair.second.getKind() == IRValueKind::PROPERTY ? "property" : "function param";
            std::cout << "  Key: " << pair.first << ", Symbol kind: " << kind
                      << ", Symbol offset: " << pair.second.getOffset() << "\n";
        }
    }
    std::cout << "\n";
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
        if (const ClassDeclaration* classDecl = dynamic_cast<const ClassDeclaration*>(decl.get())) {
            std::string                  vTableName         = Format("vTable_{0}", classDecl->name);
            std::vector<llvm::Type*>     vTableMethods      = {};
            std::vector<llvm::Constant*> vTableAInitializer = {};
            for (const auto& member : classDecl->members) {
                if (const auto* method = dynamic_cast<const MethodMember*>(member.get())) {
                    std::string classMethodName =
                        Format("{0}_{1}", classDecl->name, method->getName());
                    std::vector<llvm::Type*> paramTypes = {
                        this->generateType(Type::classType(classDecl->name), true)};
                    for (const auto& param : method->function->parameters) {
                        paramTypes.emplace_back(this->generateType(*param.type));
                    }
                    auto m = llvm::FunctionType::get(
                        this->generateType(*method->function->returnType), paramTypes, false);
                    vTableMethods.emplace_back(llvm::PointerType::getUnqual(m));
                    this->methodMap[classMethodName] = llvm::Function::Create(
                        m, llvm::Function::ExternalLinkage, classMethodName, this->module.get());
                    vTableAInitializer.emplace_back(this->methodMap[classMethodName]);
                }
            }
            auto vTableType = llvm::StructType::create(*this->context, vTableMethods, vTableName);
            auto vTableAConstant = llvm::ConstantStruct::get(vTableType, vTableAInitializer);
            this->vTableVars[vTableName] =
                new llvm::GlobalVariable(*this->module,
                                         vTableType,
                                         false,
                                         llvm::GlobalValue::ExternalLinkage,
                                         vTableAConstant,
                                         vTableName);
            this->vTableTypes[vTableName] = vTableType;
        }
    }
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
                std::vector<llvm::Type*> fieldTypes = {this->vTableTypes[vTableName]};
                std::vector<std::pair<std::string, llvm::Type*>> allParams;

                for (auto cls = inheritanceChain->rbegin(); cls != inheritanceChain->rend();
                     ++cls) {
                    for (const auto& param : (*cls)->constructorParameters) {
                        allParams.push_back({param.name, this->generateType(*param.type)});
                    }
                    for (const auto& member : (*cls)->members) {
                        if (const auto property =
                                dynamic_cast<const PropertyMember*>(member.get())) {
                            allParams.push_back(
                                {property->getName(), this->generateType(property->getType())});
                        }
                    }
                }
                for (const auto& constructorParam : classDecl->constructorParameters) {
                    allParams.push_back(
                        {constructorParam.name, this->generateType(*constructorParam.type)});
                }
                for (const auto& member : classDecl->members) {
                    if (const auto property = dynamic_cast<const PropertyMember*>(member.get())) {
                        allParams.push_back(
                            {property->getName(), this->generateType(property->getType())});
                    }
                }
                for (const auto& pair : allParams) {
                    fieldTypes.emplace_back(pair.second);
                }
                if (!fieldTypes.empty() && structType->isOpaque()) {
                    structType->setBody(fieldTypes);
                }
                this->classAllParams[classDecl->name] = std::move(allParams);

                llvm::GlobalVariable* globalVar =
                    new llvm::GlobalVariable(*this->module,
                                             structType,
                                             false,
                                             llvm::GlobalValue::ExternalLinkage,
                                             nullptr,
                                             "globalTest");
            }
        }
    }
}

void IRGen::setupClasses()
{
    this->declareClasses();
    this->buildVTables();
    this->defineClasses();
}
void IRGen::setupFunctions()
{
    for (const auto& decl : program->declarations) {
        if (const auto funcDecl = dynamic_cast<const FunctionDeclaration*>(decl.get())) {
            auto                     funcName   = funcDecl->name;
            std::vector<llvm::Type*> paramTypes = {};
            for (const auto& param : funcDecl->parameters) {
                paramTypes.emplace_back(this->generateType(*param.type));
            }
            auto m = llvm::FunctionType::get(
                this->generateType(*funcDecl->returnType), paramTypes, false);
            this->methodMap[funcName] = llvm::Function::Create(
                m, llvm::Function::ExternalLinkage, funcName, this->module.get());
        }
    }
}

llvm::Type* IRGen::generateType(const Type& type, bool ptr)
{
    switch (type.kind) {
        case Type::Kind::INT: return builder->getInt32Ty();
        case Type::Kind::STRING: return builder->getInt8PtrTy();
        case Type::Kind::BOOL: throw builder->getInt1Ty();
        case Type::Kind::FLOAT: return builder->getDoubleTy();
        case Type::Kind::VOID: return builder->getVoidTy();
        case Type::Kind::CLASS:
        {
            auto it = this->typeMap.find(type);
            if (it == this->typeMap.end()) {
                return nullptr;
            }
            return ptr ? llvm::PointerType::getUnqual(it->second) : it->second;
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
    }
}

llvm::AllocaInst* IRGen::allocateStackVariable(const std::string_view identifier, llvm::Type* type)
{
    llvm::IRBuilder<> tmpBuilder(*context);
    tmpBuilder.SetInsertPoint(allocaInsertPoint);
    return tmpBuilder.CreateAlloca(type, nullptr, identifier);
}