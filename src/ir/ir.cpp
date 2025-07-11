#include "../include/ir/ir.hpp"

#include "../include/utils/format.hpp"

// llvm::Module* IRGen::generateIR()
std::unique_ptr<llvm::Module> IRGen::generateIR()

{
    // for(auto )
    installType();
    for (const auto& decl : program->declarations) {
        generateDeclaration(*decl);
    }
    return std::move(module);
    // return &_module;
}


void IRGen::installType()
{
    for (const auto& decl : program->declarations) {
        if (const ClassDeclaration* classDecl = dynamic_cast<const ClassDeclaration*>(decl.get())) {
            llvm::StructType* classType = llvm::StructType::create(*context, classDecl->name);
            typeMap[Type::classType(classDecl->name)] = classType;
        }
    }

    for (const auto& decl : program->declarations) {
        if (const ClassDeclaration* classDecl = dynamic_cast<const ClassDeclaration*>(decl.get())) {
            auto it = typeMap.find(Type::classType(classDecl->name));
            if (it != typeMap.end()) {
                llvm::StructType*        structType = static_cast<llvm::StructType*>(it->second);
                std::vector<llvm::Type*> fieldTypes;
                for (const auto& baseArgExpr : classDecl->baseConstructorArgs) {
                    fieldTypes.emplace_back(generateType(baseArgExpr->getType()));
                }
                for (const auto& constructorParam : classDecl->constructorParameters) {
                    fieldTypes.emplace_back(generateType(*constructorParam.type));
                }
                for (const auto& member : classDecl->members) {
                    if (const auto property = dynamic_cast<const PropertyMember*>(member.get())) {
                        fieldTypes.emplace_back(generateType(property->getType()));
                    }
                }
                if (!fieldTypes.empty() && !structType->isOpaque()) {
                    structType->setBody(fieldTypes);
                }
            }
        }
    }
}

llvm::Type* IRGen::generateType(const Type& type)
{
    switch (type.kind) {
        case Type::Kind::INT: return builder->getInt32Ty();
        case Type::Kind::STRING: return builder->getInt8PtrTy();
        case Type::Kind::BOOL: throw "Not yet implemented generateType BOOL";
        case Type::Kind::FLOAT: return builder->getDoubleTy();
        case Type::Kind::VOID: return builder->getVoidTy();
        case Type::Kind::CLASS:
        {
            auto it = typeMap.find(type);
            if (it == typeMap.end()) {
                return nullptr;
            }
            return it->second;
        }
        default: break;
    }
    return nullptr;
}
