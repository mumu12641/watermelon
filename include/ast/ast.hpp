#ifndef AST_HPP
#define AST_HPP

#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

class Type;
class Expression;
class Statement;
class Declaration;

class Type
{
public:
    virtual ~Type()                      = default;
    virtual std::string toString() const = 0;
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
    {}

    std::string toString() const override
    {
        switch (kind) {
            case Kind::VOID: return "void";
            case Kind::INT: return "int";
            case Kind::FLOAT: return "float";
            case Kind::BOOL: return "bool";
            case Kind::STRING: return "String";
            default: return "unknown";
        }
    }
};

// class ArrayType : public Type
// {
// public:
//     std::unique_ptr<Type> elementType;

//     explicit ArrayType(std::unique_ptr<Type> elementType)
//         : elementType(std::move(elementType))
//     {}

//     std::string toString() const override { return "Array<" + elementType->toString() + ">"; }
// };

class NamedType : public Type
{
public:
    std::string name;

    explicit NamedType(std::string name)
        : name(std::move(name))
    {}

    std::string toString() const override { return name; }
};

class GenericType : public NamedType
{
public:
    std::vector<std::unique_ptr<Type>> typeArguments;

    GenericType(std::string name, std::vector<std::unique_ptr<Type>> typeArguments)
        : NamedType(name)
        , typeArguments(std::move(typeArguments))
    {}

    std::string toString() const override
    {
        std::string result = name + "<";
        for (size_t i = 0; i < typeArguments.size(); ++i) {
            if (i > 0) result += ", ";
            result += typeArguments[i]->toString();
        }
        result += ">";
        return result;
    }
};

class Expression
{
public:
    virtual ~Expression()                = default;
    virtual std::string toString() const = 0;
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
    {}

    std::string toString() const override
    {
        switch (kind) {
            case Kind::INT: return std::to_string(std::get<int>(value));
            case Kind::FLOAT: return std::to_string(std::get<float>(value));
            case Kind::BOOL: return std::get<bool>(value) ? "true" : "false";
            case Kind::STRING: return "\"" + std::get<std::string>(value) + "\"";
            default: return "unknown";
        }
    }
};

class IdentifierExpression : public Expression
{
public:
    std::string name;

    explicit IdentifierExpression(std::string name)
        : name(std::move(name))
    {}

    std::string toString() const override { return name; }
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
    {}

    std::string toString() const override
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
        }
        return "(" + left->toString() + " " + opStr + " " + right->toString() + ")";
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
    {}

    std::string toString() const override
    {
        std::string opStr = (op == Operator::NEG) ? "-" : "!";
        return opStr + operand->toString();
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
    {}

    std::string toString() const override
    {
        std::string result = callee->toString() + "(";
        for (size_t i = 0; i < arguments.size(); ++i) {
            if (i > 0) result += ", ";
            result += arguments[i]->toString();
        }
        result += ")";
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
    {}

    std::string toString() const override { return object->toString() + "." + property; }
};

class ArrayExpression : public Expression
{
public:
    std::vector<std::unique_ptr<Expression>> elements;

    explicit ArrayExpression(std::vector<std::unique_ptr<Expression>> elements)
        : elements(std::move(elements))
    {}

    std::string toString() const override
    {
        std::string result = "[";
        for (size_t i = 0; i < elements.size(); ++i) {
            if (i > 0) result += ", ";
            result += elements[i]->toString();
        }
        result += "]";
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
    {}

    std::string toString() const override
    {
        std::string result = "{";
        for (size_t i = 0; i < parameters.size(); ++i) {
            if (i > 0) result += ", ";
            result += parameters[i].name;
            if (parameters[i].type) {
                result += ":" + parameters[i].type->toString();
            }
        }
        result += " -> " + body->toString() + "}";
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
    {}

    std::string toString() const override
    {
        return expression->toString() + " is " + type->toString();
    }
};

class Statement
{
public:
    virtual ~Statement()                 = default;
    virtual std::string toString() const = 0;
};

class ExpressionStatement : public Statement
{
public:
    std::unique_ptr<Expression> expression;

    explicit ExpressionStatement(std::unique_ptr<Expression> expression)
        : expression(std::move(expression))
    {}

    std::string toString() const override { return expression->toString(); }
};

class BlockStatement : public Statement
{
public:
    std::vector<std::unique_ptr<Statement>> statements;

    explicit BlockStatement(std::vector<std::unique_ptr<Statement>> statements)
        : statements(std::move(statements))
    {}

    std::string toString() const override
    {
        std::string result = "{\n";
        for (const auto& stmt : statements) {
            result += "  " + stmt->toString() + "\n";
        }
        result += "}";
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
    {}

    std::string toString() const override
    {
        std::string result = "if (" + condition->toString() + ") " + thenBranch->toString();
        if (elseBranch) {
            result += " else " + elseBranch->toString();
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
    {}

    std::string toString() const override
    {
        std::string result = "when (" + subject->toString() + ") {\n";
        for (const auto& c : cases) {
            result += "  " + c.value->toString() + " -> " + c.body->toString() + "\n";
        }
        result += "}";
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
    {}

    std::string toString() const override
    {
        return "for(" + variable + " in " + iterable->toString() + ") " + body->toString();
    }
};

class ReturnStatement : public Statement
{
public:
    std::unique_ptr<Expression> value;

    explicit ReturnStatement(std::unique_ptr<Expression> value = nullptr)
        : value(std::move(value))
    {}

    std::string toString() const override
    {
        if (value) {
            return "return " + value->toString();
        }
        else {
            return "return";
        }
    }
};

class Declaration : public Statement
{
public:
    virtual ~Declaration() = default;
};

class VariableDeclaration : public Declaration
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

    VariableDeclaration(Kind kind, std::string name, std::unique_ptr<Type> type,
                        std::unique_ptr<Expression> initializer)
        : kind(kind)
        , name(std::move(name))
        , type(std::move(type))
        , initializer(std::move(initializer))
    {}

    std::string toString() const override
    {
        std::string kindStr;
        switch (kind) {
            case Kind::VAR: kindStr = "var"; break;
            case Kind::VAL: kindStr = "val"; break;
        }

        std::string result = kindStr + " " + name;
        if (type) {
            result += ":" + type->toString();
        }
        if (initializer) {
            result += " = " + initializer->toString();
        }
        return result;
    }
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
    {}

    std::string toString() const
    {
        std::string result = name;
        if (type) {
            result += ":" + type->toString();
        }
        if (defaultValue) {
            result += "=" + defaultValue->toString();
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
    {}

    std::string toString() const override
    {
        std::string result = (isOperator ? "operator " : "") + std::string("fn ") + name + "(";
        for (size_t i = 0; i < parameters.size(); ++i) {
            if (i > 0) result += ", ";
            result += parameters[i].toString();
        }
        result += ")";
        if (returnType) {
            result += " -> " + returnType->toString();
        }
        if (body) {
            result += " " + body->toString();
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
    {}

    std::string toString() const override
    {
        std::string result = "enum class " + name + "{\n";
        for (size_t i = 0; i < values.size(); ++i) {
            if (i > 0) result += ", ";
            result += "  " + values[i];
        }
        result += "\n}";
        return result;
    }
};

class ClassMember
{
public:
    virtual ~ClassMember()               = default;
    virtual std::string toString() const = 0;
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
    {}

    std::string toString() const override
    {
        std::string result = "var " + name + ":" + type->toString();
        if (initializer) {
            result += " = " + initializer->toString();
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
    {}

    std::string toString() const override { return function->toString(); }
};

class InitBlockMember : public ClassMember
{
public:
    std::unique_ptr<BlockStatement> block;

    explicit InitBlockMember(std::unique_ptr<BlockStatement> block)
        : block(std::move(block))
    {}

    std::string toString() const override { return "init" + block->toString(); }
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
    {}

    std::string toString() const override
    {
        std::string kindStr;
        switch (kind) {
            case Kind::NORMAL: kindStr = "class"; break;
            case Kind::DATA: kindStr = "data class"; break;
            case Kind::BASE: kindStr = "base class"; break;
        }

        std::string result = kindStr + " " + name;

        if (!typeParameters.empty()) {
            result += "<";
            for (size_t i = 0; i < typeParameters.size(); ++i) {
                if (i > 0) result += ", ";
                result += typeParameters[i];
            }
            result += ">";
        }

        result += "(";
        for (size_t i = 0; i < constructorParameters.size(); ++i) {
            if (i > 0) result += ", ";
            result += constructorParameters[i].toString();
        }
        result += ")";

        if (!baseClass.empty()) {
            result += " inherits " + baseClass;

            if (!baseTypeArguments.empty()) {
                result += "<";
                for (size_t i = 0; i < baseTypeArguments.size(); ++i) {
                    if (i > 0) result += ", ";
                    result += baseTypeArguments[i]->toString();
                }
                result += ">";
            }

            result += "(";
            for (size_t i = 0; i < baseConstructorArgs.size(); ++i) {
                if (i > 0) result += ", ";
                result += baseConstructorArgs[i]->toString();
            }
            result += ")";
        }

        result += " {\n";
        for (const auto& member : members) {
            result += "  " + member->toString() + "\n";
        }
        result += "}";

        return result;
    }
};

class Program
{
public:
    std::vector<std::unique_ptr<Declaration>> declarations;

    explicit Program(std::vector<std::unique_ptr<Declaration>> declarations)
        : declarations(std::move(declarations))
    {}

    std::string toString() const
    {
        std::string result;
        for (const auto& decl : declarations) {
            result += decl->toString() + "\n\n";
        }
        return result;
    }
};

#endif