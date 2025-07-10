#include "../include/ir/ir.hpp"

#include "../include/utils/format.hpp"

llvm::Module* IRGen::generateIR()
{
    // for(auto )
    for (const auto& decl : program->declarations) {
        generateDeclaration(*decl);
    }
}
llvm::Type* IRGen::generateType(Type* type)
{
    
    // switch (type.getName())
    // {
    // case constant expression:
    //     /* code */
    //     break;

    // default:
    //     break;
    // }
    if (auto* primitive = dynamic_cast<PrimitiveType*>(type)) {
        switch (primitive->kind) {
            case PrimitiveType::Kind::INT: return builder.getInt32Ty();
            case PrimitiveType::Kind::STRING: return builder.getInt8PtrTy();
            case PrimitiveType::Kind::BOOL: throw "Not yet implemented generateType BOOL";
            case PrimitiveType::Kind::FLOAT: return builder.getDoubleTy();
            case PrimitiveType::Kind::VOID: return builder.getVoidTy();
        }
    }
    else if (auto* custom = dynamic_cast<CustomType*>(type)) {
        // return module.getType
        // this->module.getIdentifiedStructTypes();
    }
}
