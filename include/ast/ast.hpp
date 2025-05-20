#ifndef AST_HPP
#define AST_HPP

#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

static std::string indent(int level)
{
    return std::string(level * 2, ' ');
}

class Type;
class Expression;
class Statement;
class Declaration;

class Type
{
public:
    virtual ~Type()                               = default;
    virtual std::string dump(int level = 0) const = 0;
};

class PrimitiveType : public Type
{
public:
    enum class Kind
    {
        VOID,
        INT,
        FLOAT,
        BOOL,
        STRING
    };

    Kind kind;

    explicit PrimitiveType(Kind kind)
        : kind(kind)
    {
    }


    std::string dump(int level = 0) const override
    {
        std::string s;
        switch (kind) {
            case Kind::VOID: s = "void"; break;
            case Kind::INT: s = "int"; break;
            case Kind::FLOAT: s = "float"; break;
            case Kind::BOOL: s = "bool"; break;
            case Kind::STRING: s = "String"; break;
            default: s = "unknown"; break;
        }
        return indent(level) + "PrimitiveType: " + s;
    }
};

class NamedType : public Type
{
public:
    std::string name;

    explicit NamedType(std::string name)
        : name(std::move(name))
    {
    }


    std::string dump(int level = 0) const override { return indent(level) + "NamedType: " + name; }
};

class GenericType : public NamedType
{
public:
    std::vector<std::unique_ptr<Type>> typeArguments;

    GenericType(std::string name, std::vector<std::unique_ptr<Type>> typeArguments)
        : NamedType(name)
        , typeArguments(std::move(typeArguments))
    {
    }

    std::string dump(int level = 0) const override
    {
        std::string result = indent(level) + "GenericType: " + name + "\n";
        for (const auto& arg : typeArguments) {
            result += arg->dump(level + 1) + "\n";
        }
        return result;
    }
};

class Expression
{
public:
    virtual ~Expression()                         = default;
    virtual std::string dump(int level = 0) const = 0;
};

class LiteralExpression : public Expression
{
public:
    enum class Kind
    {
        INT,
        FLOAT,
        BOOL,
        STRING
    };

    Kind                                        kind;
    std::variant<int, float, bool, std::string> value;

    LiteralExpression(Kind kind, std::variant<int, float, bool, std::string> value)
        : kind(kind)
        , value(std::move(value))
    {
    }

    std::string dump(int level = 0) const override
    {
        std::string kindStr, s;
        switch (kind) {
            case Kind::INT:
                kindStr = "INT";
                s       = std::to_string(std::get<int>(value));
                break;
            case Kind::FLOAT:
                kindStr = "FLOAT";
                s       = std::to_string(std::get<float>(value));
                break;
            case Kind::BOOL:
                kindStr = "BOOL";
                s       = std::get<bool>(value) ? "true" : "false";
                break;
            case Kind::STRING:
                kindStr = "STRING";
                s       = "\"" + std::get<std::string>(value) + "\"";
                break;
            default:
                kindStr = "UNKNOWN";
                s       = "UNKNOWN";
                break;
        }
        return indent(level) + "LiteralExpression (" + kindStr + "): " + s;
    }
};

class IdentifierExpression : public Expression
{
public:
    std::string name;

    explicit IdentifierExpression(std::string name)
        : name(std::move(name))
    {
    }


    std::string dump(int level = 0) const override
    {
        return indent(level) + "IdentifierExpression: " + name;
    }
};

class BinaryExpression : public Expression
{
public:
    enum class Operator
    {
        ADD,
        SUB,
        MUL,
        DIV,
        MOD,
        EQ,
        NEQ,
        LT,
        LE,
        GT,
        GE,
        AND,
        OR,
        ASSIGN
    };

    Operator                    op;
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;

    BinaryExpression(Operator op, std::unique_ptr<Expression> left,
                     std::unique_ptr<Expression> right)
        : op(op)
        , left(std::move(left))
        , right(std::move(right))
    {
    }


    std::string dump(int level = 0) const override
    {
        std::string opStr;
        switch (op) {
            case Operator::ADD: opStr = "+"; break;
            case Operator::SUB: opStr = "-"; break;
            case Operator::MUL: opStr = "*"; break;
            case Operator::DIV: opStr = "/"; break;
            case Operator::MOD: opStr = "%"; break;
            case Operator::EQ: opStr = "=="; break;
            case Operator::NEQ: opStr = "!="; break;
            case Operator::LT: opStr = "<"; break;
            case Operator::LE: opStr = "<="; break;
            case Operator::GT: opStr = ">"; break;
            case Operator::GE: opStr = ">="; break;
            case Operator::AND: opStr = "&&"; break;
            case Operator::OR: opStr = "||"; break;
            case Operator::ASSIGN: opStr = "="; break;
        }
        std::string result = indent(level) + "BinaryExpression: " + opStr + "\n";
        result += left->dump(level + 1) + "\n";
        result += right->dump(level + 1);
        return result;
    }
};

class UnaryExpression : public Expression
{
public:
    enum class Operator
    {
        NEG,
        NOT
    };

    Operator                    op;
    std::unique_ptr<Expression> operand;

    UnaryExpression(Operator op, std::unique_ptr<Expression> operand)
        : op(op)
        , operand(std::move(operand))
    {
    }


    std::string dump(int level = 0) const override
    {
        std::string opStr  = (op == Operator::NEG) ? "-" : "!";
        std::string result = indent(level) + "UnaryExpression: " + opStr + "\n";
        result += operand->dump(level + 1);
        return result;
    }
};

class CallExpression : public Expression
{
public:
    std::unique_ptr<Expression>              callee;
    std::vector<std::unique_ptr<Expression>> arguments;

    CallExpression(std::unique_ptr<Expression>              callee,
                   std::vector<std::unique_ptr<Expression>> arguments)
        : callee(std::move(callee))
        , arguments(std::move(arguments))
    {
    }


    std::string dump(int level = 0) const override
    {
        std::string result = indent(level) + "CallExpression:\n";
        result += indent(level + 1) + "Callee:\n";
        result += callee->dump(level + 2) + "\n";
        if (!arguments.empty()) {
            result += indent(level + 1) + "Arguments:\n";
            for (const auto& arg : arguments) {
                result += arg->dump(level + 2) + "\n";
            }
        }
        return result;
    }
};

class MemberExpression : public Expression
{
public:
    std::unique_ptr<Expression> object;
    std::string                 property;

    MemberExpression(std::unique_ptr<Expression> object, std::string property)
        : object(std::move(object))
        , property(std::move(property))
    {
    }


    std::string dump(int level = 0) const override
    {
        std::string result = indent(level) + "MemberExpression: " + property + "\n";
        result += indent(level + 1) + "Object:\n";
        result += object->dump(level + 2);
        return result;
    }
};

class ArrayExpression : public Expression
{
public:
    std::vector<std::unique_ptr<Expression>> elements;

    explicit ArrayExpression(std::vector<std::unique_ptr<Expression>> elements)
        : elements(std::move(elements))
    {
    }

    std::string dump(int level = 0) const override
    {
        std::string result = indent(level) + "ArrayExpression:\n";
        for (const auto& elem : elements) {
            result += elem->dump(level + 1) + "\n";
        }
        return result;
    }
};

class LambdaExpression : public Expression
{
public:
    struct Parameter
    {
        std::string           name;
        std::unique_ptr<Type> type;
    };

    std::vector<Parameter>      parameters;
    std::unique_ptr<Expression> body;

    LambdaExpression(std::vector<Parameter> parameters, std::unique_ptr<Expression> body)
        : parameters(std::move(parameters))
        , body(std::move(body))
    {
    }


    std::string dump(int level = 0) const override
    {
        std::string result = indent(level) + "LambdaExpression:\n";
        if (!parameters.empty()) {
            result += indent(level + 1) + "Parameters:\n";
            for (const auto& param : parameters) {
                result += indent(level + 2) + param.name;
                if (param.type) {
                    result += ":\n" + param.type->dump(level + 3);
                }
                result += "\n";
            }
        }
        result += indent(level + 1) + "Body:\n";
        result += body->dump(level + 2);
        return result;
    }
};

class TypeCheckExpression : public Expression
{
public:
    std::unique_ptr<Expression> expression;
    std::unique_ptr<Type>       type;

    TypeCheckExpression(std::unique_ptr<Expression> expression, std::unique_ptr<Type> type)
        : expression(std::move(expression))
        , type(std::move(type))
    {
    }

    std::string dump(int level = 0) const override
    {
        std::string result = indent(level) + "TypeCheckExpression:\n";
        result += indent(level + 1) + "Expression:\n";
        result += expression->dump(level + 2) + "\n";
        result += indent(level + 1) + "Type:\n";
        result += type->dump(level + 2);
        return result;
    }
};

class Statement
{
public:
    virtual ~Statement()                          = default;
    virtual std::string dump(int level = 0) const = 0;
};

class ExpressionStatement : public Statement
{
public:
    std::unique_ptr<Expression> expression;

    explicit ExpressionStatement(std::unique_ptr<Expression> expression)
        : expression(std::move(expression))
    {
    }


    std::string dump(int level = 0) const override
    {
        std::string result = indent(level) + "ExpressionStatement:\n";
        result += expression->dump(level + 1);
        return result;
    }
};

class BlockStatement : public Statement
{
public:
    std::vector<std::unique_ptr<Statement>> statements;

    explicit BlockStatement(std::vector<std::unique_ptr<Statement>> statements)
        : statements(std::move(statements))
    {
    }


    std::string dump(int level = 0) const override
    {
        std::string result = indent(level) + "BlockStatement:\n";
        for (const auto& stmt : statements) {
            result += stmt->dump(level + 1) + "\n";
        }
        return result;
    }
};

class IfStatement : public Statement
{
public:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement>  thenBranch;
    std::unique_ptr<Statement>  elseBranch;

    IfStatement(std::unique_ptr<Expression> condition, std::unique_ptr<Statement> thenBranch,
                std::unique_ptr<Statement> elseBranch = nullptr)
        : condition(std::move(condition))
        , thenBranch(std::move(thenBranch))
        , elseBranch(std::move(elseBranch))
    {
    }


    std::string dump(int level = 0) const override
    {
        std::string result = indent(level) + "IfStatement:\n";
        result += indent(level + 1) + "Condition:\n";
        result += condition->dump(level + 2) + "\n";
        result += indent(level + 1) + "Then:\n";
        result += thenBranch->dump(level + 2);
        if (elseBranch) {
            result += "\n" + indent(level + 1) + "Else:\n";
            result += elseBranch->dump(level + 2);
        }
        return result;
    }
};

class WhenStatement : public Statement
{
public:
    struct Case
    {
        std::unique_ptr<Expression> value;
        std::unique_ptr<Statement>  body;
    };

    std::unique_ptr<Expression> subject;
    std::vector<Case>           cases;

    WhenStatement(std::unique_ptr<Expression> subject, std::vector<Case> cases)
        : subject(std::move(subject))
        , cases(std::move(cases))
    {
    }


    std::string dump(int level = 0) const override
    {
        std::string result = indent(level) + "WhenStatement:\n";
        result += indent(level + 1) + "Subject:\n";
        result += subject->dump(level + 2) + "\n";
        result += indent(level + 1) + "Cases:\n";
        for (const auto& c : cases) {
            result += indent(level + 2) + "Case:\n";
            result += indent(level + 3) + "Value:\n";
            result += c.value->dump(level + 4) + "\n";
            result += indent(level + 3) + "Body:\n";
            result += c.body->dump(level + 4) + "\n";
        }
        return result;
    }
};

class ForStatement : public Statement
{
public:
    std::string                 variable;
    std::unique_ptr<Expression> iterable;
    std::unique_ptr<Statement>  body;

    ForStatement(std::string variable, std::unique_ptr<Expression> iterable,
                 std::unique_ptr<Statement> body)
        : variable(std::move(variable))
        , iterable(std::move(iterable))
        , body(std::move(body))
    {
    }


    std::string dump(int level = 0) const override
    {
        std::string result = indent(level) + "ForStatement:\n";
        result += indent(level + 1) + "Variable: " + variable + "\n";
        result += indent(level + 1) + "Iterable:\n";
        result += iterable->dump(level + 2) + "\n";
        result += indent(level + 1) + "Body:\n";
        result += body->dump(level + 2);
        return result;
    }
};

class ReturnStatement : public Statement
{
public:
    std::unique_ptr<Expression> value;

    explicit ReturnStatement(std::unique_ptr<Expression> value = nullptr)
        : value(std::move(value))
    {
    }


    std::string dump(int level = 0) const override
    {
        std::string result = indent(level) + "ReturnStatement:";
        if (value) {
            result += "\n" + value->dump(level + 1);
        }
        return result;
    }
};

class VariableStatement : public Statement
{
public:
    enum class Kind
    {
        VAR,
        VAL,
    };

    Kind                        kind;
    std::string                 name;
    std::unique_ptr<Type>       type;
    std::unique_ptr<Expression> initializer;

    VariableStatement(Kind kind, std::string name, std::unique_ptr<Type> type,
                      std::unique_ptr<Expression> initializer)
        : kind(kind)
        , name(std::move(name))
        , type(std::move(type))
        , initializer(std::move(initializer))
    {
    }


    std::string dump(int level = 0) const override
    {
        std::string kindStr;
        switch (kind) {
            case Kind::VAR: kindStr = "var"; break;
            case Kind::VAL: kindStr = "val"; break;
        }

        std::string result = indent(level) + "VariableStatement (" + kindStr + "): " + name;
        if (type) {
            result += "\n" + indent(level + 1) + "Type:\n";
            result += type->dump(level + 2);
        }
        if (initializer) {
            result += "\n" + indent(level + 1) + "Initializer:\n";
            result += initializer->dump(level + 2);
        }
        return result;
    }
};

class Declaration : public Statement
{
public:
    virtual ~Declaration() = default;
};

class FunctionParameter
{
public:
    std::string                 name;
    std::unique_ptr<Type>       type;
    std::unique_ptr<Expression> defaultValue;

    FunctionParameter(std::string name, std::unique_ptr<Type> type,
                      std::unique_ptr<Expression> defaultValue = nullptr)
        : name(std::move(name))
        , type(std::move(type))
        , defaultValue(std::move(defaultValue))
    {
    }


    std::string dump(int level = 0) const
    {
        std::string result = indent(level) + "Parameter: " + name;
        if (type) {
            result += "\n" + indent(level + 1) + "Type:\n";
            result += type->dump(level + 2);
        }
        if (defaultValue) {
            result += "\n" + indent(level + 1) + "DefaultValue:\n";
            result += defaultValue->dump(level + 2);
        }
        return result;
    }
};

class FunctionDeclaration : public Declaration
{
public:
    std::string                    name;
    std::vector<FunctionParameter> parameters;
    std::unique_ptr<Type>          returnType;
    std::unique_ptr<Statement>     body;
    bool                           isOperator;

    FunctionDeclaration(std::string name, std::vector<FunctionParameter> parameters,
                        std::unique_ptr<Type> returnType, std::unique_ptr<Statement> body,
                        bool isOperator = false)
        : name(std::move(name))
        , parameters(std::move(parameters))
        , returnType(std::move(returnType))
        , body(std::move(body))
        , isOperator(isOperator)
    {
    }


    std::string dump(int level = 0) const override
    {
        std::string result = indent(level) + "FunctionDeclaration: " + name;
        if (isOperator) {
            result += " (operator)";
        }
        result += "\n";

        if (!parameters.empty()) {
            result += indent(level + 1) + "Parameters:\n";
            for (const auto& param : parameters) {
                result += param.dump(level + 2) + "\n";
            }
        }

        if (returnType) {
            result += indent(level + 1) + "ReturnType:\n";
            result += returnType->dump(level + 2) + "\n";
        }

        if (body) {
            result += indent(level + 1) + "Body:\n";
            result += body->dump(level + 2);
        }

        return result;
    }
};

class EnumDeclaration : public Declaration
{
public:
    std::string              name;
    std::vector<std::string> values;

    EnumDeclaration(std::string name, std::vector<std::string> values)
        : name(std::move(name))
        , values(std::move(values))
    {
    }


    std::string dump(int level = 0) const override
    {
        std::string result = indent(level) + "EnumDeclaration: " + name + "\n";
        result += indent(level + 1) + "Values:\n";
        for (const auto& value : values) {
            result += indent(level + 2) + value + "\n";
        }
        return result;
    }
};

class ClassMember
{
public:
    virtual ~ClassMember()                        = default;
    virtual std::string dump(int level = 0) const = 0;
};

class PropertyMember : public ClassMember
{
public:
    std::string                 name;
    std::unique_ptr<Type>       type;
    std::unique_ptr<Expression> initializer;

    PropertyMember(std::string name, std::unique_ptr<Type> type,
                   std::unique_ptr<Expression> initializer = nullptr)
        : name(std::move(name))
        , type(std::move(type))
        , initializer(std::move(initializer))
    {
    }


    std::string dump(int level = 0) const override
    {
        std::string result = indent(level) + "PropertyMember: " + name + "\n";
        result += indent(level + 1) + "Type:\n";
        result += type->dump(level + 2);
        if (initializer) {
            result += "\n" + indent(level + 1) + "Initializer:\n";
            result += initializer->dump(level + 2);
        }
        return result;
    }
};

class MethodMember : public ClassMember
{
public:
    std::unique_ptr<FunctionDeclaration> function;

    explicit MethodMember(std::unique_ptr<FunctionDeclaration> function)
        : function(std::move(function))
    {
    }


    std::string dump(int level = 0) const override
    {
        std::string result = indent(level) + "MethodMember:\n";
        result += function->dump(level + 1);
        return result;
    }
};

class InitBlockMember : public ClassMember
{
public:
    std::unique_ptr<BlockStatement> block;

    explicit InitBlockMember(std::unique_ptr<BlockStatement> block)
        : block(std::move(block))
    {
    }


    std::string dump(int level = 0) const override
    {
        std::string result = indent(level) + "InitBlockMember:\n";
        result += block->dump(level + 1);
        return result;
    }
};

class ClassDeclaration : public Declaration
{
public:
    enum class Kind
    {
        NORMAL,
        DATA,
        BASE
    };

    Kind                                      kind;
    std::string                               name;
    std::vector<std::string>                  typeParameters;
    std::vector<FunctionParameter>            constructorParameters;
    std::string                               baseClass;
    std::vector<std::unique_ptr<Type>>        baseTypeArguments;
    std::vector<std::unique_ptr<Expression>>  baseConstructorArgs;
    std::vector<std::unique_ptr<ClassMember>> members;

    ClassDeclaration(Kind kind, std::string name, std::vector<std::string> typeParameters,
                     std::vector<FunctionParameter> constructorParameters, std::string baseClass,
                     std::vector<std::unique_ptr<Type>>        baseTypeArguments,
                     std::vector<std::unique_ptr<Expression>>  baseConstructorArgs,
                     std::vector<std::unique_ptr<ClassMember>> members)
        : kind(kind)
        , name(std::move(name))
        , typeParameters(std::move(typeParameters))
        , constructorParameters(std::move(constructorParameters))
        , baseClass(std::move(baseClass))
        , baseTypeArguments(std::move(baseTypeArguments))
        , baseConstructorArgs(std::move(baseConstructorArgs))
        , members(std::move(members))
    {
    }



    std::string dump(int level = 0) const override
    {
        std::string kindStr;
        switch (kind) {
            case Kind::NORMAL: kindStr = "class"; break;
            case Kind::DATA: kindStr = "data class"; break;
            case Kind::BASE: kindStr = "base class"; break;
        }

        std::string result = indent(level) + "ClassDeclaration (" + kindStr + "): " + name + "\n";

        if (!typeParameters.empty()) {
            result += indent(level + 1) + "TypeParameters:\n";
            for (const auto& param : typeParameters) {
                result += indent(level + 2) + param + "\n";
            }
        }

        if (!constructorParameters.empty()) {
            result += indent(level + 1) + "ConstructorParameters:\n";
            for (const auto& param : constructorParameters) {
                result += param.dump(level + 2) + "\n";
            }
        }

        if (!baseClass.empty()) {
            result += indent(level + 1) + "BaseClass: " + baseClass + "\n";

            if (!baseTypeArguments.empty()) {
                result += indent(level + 1) + "BaseTypeArguments:\n";
                for (const auto& arg : baseTypeArguments) {
                    result += arg->dump(level + 2) + "\n";
                }
            }

            if (!baseConstructorArgs.empty()) {
                result += indent(level + 1) + "BaseConstructorArgs:\n";
                for (const auto& arg : baseConstructorArgs) {
                    result += arg->dump(level + 2) + "\n";
                }
            }
        }

        if (!members.empty()) {
            result += indent(level + 1) + "Members:\n";
            for (const auto& member : members) {
                result += member->dump(level + 2) + "\n";
            }
        }

        return result;
    }
};

class Program
{
public:
    std::vector<std::unique_ptr<Declaration>> declarations;

    explicit Program(std::vector<std::unique_ptr<Declaration>> declarations)
        : declarations(std::move(declarations))
    {
    }

    std::string dump() const
    {
        std::string result = "Program:\n";
        for (const auto& decl : declarations) {
            result += decl->dump(1) + "\n";
        }
        return result;
    }
};

#endif