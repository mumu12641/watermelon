#ifndef AST_HPP
#define AST_HPP

#include "../utils/error.hpp"

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
    enum class Kind
    {
        EMPTY,
        VOID,
        INT,
        FLOAT,
        BOOL,
        STRING,
        CLASS,
        FUNCTION
    };
    Kind        kind;
    std::string name;
    static Type builtinVoid() { return Type(Kind::VOID, "void"); }
    static Type builtinInt() { return Type(Kind::INT, "int"); }
    static Type builtinFloat() { return Type(Kind::FLOAT, "float"); }
    static Type builtinBool() { return Type(Kind::BOOL, "bool"); }
    static Type builtinString() { return Type(Kind::STRING, "String"); }
    static Type classType(const std::string& name) { return Type(Kind::CLASS, name); }
    static Type functionType(const std::string& name) { return Type(Kind::FUNCTION, name); }

    bool isBool() const { return kind == Kind::BOOL; }
    bool isVoid() const { return kind == Kind::VOID; }
    bool canMathOp() const { return kind == Kind::INT || kind == Kind::FLOAT; }
    bool canCompare() const { return kind == Kind::INT || kind == Kind::FLOAT; }

    bool operator==(const Type& other) const { return kind == other.kind && name == other.name; }
    bool operator!=(const Type& other) const { return !(*this == other); }

    std::string dump(int level = 0) const { return indent(level) + "Type: " + name; }
    std::string getName() const { return name; }

    Type(Kind kind, std::string name)
        : kind(kind)
        , name(std::move(name)) {};

    Type(std::string name)
        : name(std::move(name))
    {
        if (name == "void")
            kind = Kind::VOID;
        else if (name == "int")
            kind = Kind::INT;
        else if (name == "float")
            kind = Kind::FLOAT;
        else if (name == "bool")
            kind = Kind::BOOL;
        else if (name == "String")
            kind = Kind::STRING;
        else
            kind = Kind::CLASS;
    }
    Type()
        : kind(Kind::EMPTY)
        , name("none")
    {
    }
};

namespace std {
template<> struct hash<Type>
{
    std::size_t operator()(const Type& type) const noexcept
    {
        return std::hash<std::string>{}(type.getName());
    }
};
}  

class Expression
{
protected:
    Location location;
    Type     type;

public:
    Expression(Location l)
        : location(l)
    {
    }
    Expression(Location l, Type t)
        : location(l)
        , type(t)
    {
    }
    Location getLocation() const { return location; }
    Type     getType() const { return type; }
    void     setType(Type t) { type = t; }
    virtual ~Expression()                         = default;
    virtual std::string dump(int level = 0) const = 0;
};

class LiteralExpression : public Expression
{
public:
    std::variant<int, float, bool, std::string> value;

    explicit LiteralExpression(Location location, Type type,
                               std::variant<int, float, bool, std::string> value)
        : value(std::move(value))
        , Expression(location, type)
    {
    }

    std::string dump(int level = 0) const override
    {
        std::string kindStr, s;
        switch (type.kind) {
            case Type::Kind::INT:
                kindStr = "INT";
                s       = std::to_string(std::get<int>(value));
                break;
            case Type::Kind::FLOAT:
                kindStr = "FLOAT";
                s       = std::to_string(std::get<float>(value));
                break;
            case Type::Kind::BOOL:
                kindStr = "BOOL";
                s       = std::get<bool>(value) ? "true" : "false";
                break;
            case Type::Kind::STRING:
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

    explicit IdentifierExpression(Location location, std::string name)
        : name(std::move(name))
        , Expression(location)
    {
    }


    std::string dump(int level = 0) const override
    {
        return indent(level) + "IdentifierExpression: " + name + " type: " + type.getName();
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

    BinaryExpression(Location location, Operator op, std::unique_ptr<Expression> left,
                     std::unique_ptr<Expression> right)
        : op(op)
        , left(std::move(left))
        , right(std::move(right))
        , Expression(location)
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
        std::string result =
            indent(level) + "BinaryExpression: " + opStr + " type: " + type.getName() + "\n";
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

    UnaryExpression(Location location, Operator op, std::unique_ptr<Expression> operand)
        : op(op)
        , operand(std::move(operand))
        , Expression(location)
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
    // function call & class init
public:
    std::unique_ptr<Expression>              callee;
    std::vector<std::unique_ptr<Expression>> arguments;

    CallExpression(Location location, std::unique_ptr<Expression> callee,
                   std::vector<std::unique_ptr<Expression>> arguments)
        : callee(std::move(callee))
        , arguments(std::move(arguments))
        , Expression(location)
    {
    }


    std::string dump(int level = 0) const override
    {
        std::string result = indent(level) + "CallExpression:" + " type: " + type.getName() + "\n";
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
    enum class Kind
    {
        PROPERTY,
        METHOD,
    };

    std::unique_ptr<Expression>              object;
    std::string                              property;
    std::string                              methodName;
    std::vector<std::unique_ptr<Expression>> arguments;
    Kind                                     kind;

    MemberExpression(Location location, std::unique_ptr<Expression> object, std::string property)
        : object(std::move(object))
        , property(std::move(property))
        , Expression(location)
    {
        kind = Kind::PROPERTY;
    }
    MemberExpression(Location location, std::unique_ptr<Expression> object, std::string methodName,
                     std::vector<std::unique_ptr<Expression>> arguments)
        : object(std::move(object))
        , methodName(std::move(methodName))
        , arguments(std::move(arguments))
        , Expression(location)
    {
        kind = Kind::METHOD;
    }


    std::string dump(int level = 0) const override
    {
        std::string result =
            indent(level) + "MemberExpression: " +
            (kind == Kind::METHOD ? "method " + methodName : "property " + property) +
            " type: " + type.getName() + "\n";
        result += indent(level + 1) + "Object:\n";
        result += object->dump(level + 2);
        return result;
    }
};

class MethodCallExpression : public Expression
{
public:
    std::unique_ptr<Expression>              object;
    std::string                              methodName;
    std::vector<std::unique_ptr<Expression>> arguments;

    MethodCallExpression(Location location, std::unique_ptr<Expression> object,
                         std::string methodName, std::vector<std::unique_ptr<Expression>> arguments)
        : object(std::move(object))
        , methodName(std::move(methodName))
        , arguments(std::move(arguments))
        , Expression(location)
    {
    }

    std::string dump(int level = 0) const override
    {
        std::string result = indent(level) + "MethodCallExpression: " + methodName +
                             " type: " + type.getName() + "\n";
        result += indent(level + 1) + "Object:\n";
        result += object->dump(level + 2) + "\n";
        if (!arguments.empty()) {
            result += indent(level + 1) + "Arguments:\n";
            for (const auto& arg : arguments) {
                result += arg->dump(level + 2) + "\n";
            }
        }
        return result;
    }
};

class ArrayExpression : public Expression
{
public:
    std::vector<std::unique_ptr<Expression>> elements;

    explicit ArrayExpression(Location location, std::vector<std::unique_ptr<Expression>> elements)
        : elements(std::move(elements))
        , Expression(location)
    {
    }

    std::string dump(int level = 0) const override
    {
        std::string result = indent(level) + "ArrayExpression:" + " type: " + type.getName() + "\n";
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

    LambdaExpression(Location location, std::vector<Parameter> parameters,
                     std::unique_ptr<Expression> body)
        : parameters(std::move(parameters))
        , body(std::move(body))
        , Expression(location)
    {
    }


    std::string dump(int level = 0) const override
    {
        std::string result =
            indent(level) + "LambdaExpression:" + " type: " + type.getName() + "\n";
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

    TypeCheckExpression(Location location, std::unique_ptr<Expression> expression,
                        std::unique_ptr<Type> type)
        : expression(std::move(expression))
        , type(std::move(type))
        , Expression(location)
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

private:
    Location location;

public:
    Statement(Location l)
        : location(l)
    {
    }
    Location getLocation() const { return location; }
    virtual ~Statement()                          = default;
    virtual std::string dump(int level = 0) const = 0;
};

class ExpressionStatement : public Statement
{
public:
    std::unique_ptr<Expression> expression;

    explicit ExpressionStatement(Location location, std::unique_ptr<Expression> expression)
        : expression(std::move(expression))
        , Statement(location)
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

    explicit BlockStatement(Location location, std::vector<std::unique_ptr<Statement>> statements)
        : statements(std::move(statements))
        , Statement(location)
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

    IfStatement(Location location, std::unique_ptr<Expression> condition,
                std::unique_ptr<Statement> thenBranch,
                std::unique_ptr<Statement> elseBranch = nullptr)
        : condition(std::move(condition))
        , thenBranch(std::move(thenBranch))
        , elseBranch(std::move(elseBranch))
        , Statement(location)
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

    WhenStatement(Location location, std::unique_ptr<Expression> subject, std::vector<Case> cases)
        : subject(std::move(subject))
        , cases(std::move(cases))
        , Statement(location)
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

    ForStatement(Location location, std::string variable, std::unique_ptr<Expression> iterable,
                 std::unique_ptr<Statement> body)
        : variable(std::move(variable))
        , iterable(std::move(iterable))
        , body(std::move(body))
        , Statement(location)
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

    explicit ReturnStatement(Location location, std::unique_ptr<Expression> value = nullptr)
        : value(std::move(value))
        , Statement(location)
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
    bool                        immutable;
    std::string                 name;
    std::unique_ptr<Type>       type;
    std::unique_ptr<Expression> initializer;

    VariableStatement(Location location, bool immutable, std::string name,
                      std::unique_ptr<Type> type, std::unique_ptr<Expression> initializer)
        : immutable(immutable)
        , name(std::move(name))
        , type(std::move(type))
        , initializer(std::move(initializer))
        , Statement(location)
    {
    }


    std::string dump(int level = 0) const override
    {
        std::string kindStr;
        switch (immutable) {
            case false: kindStr = "var"; break;
            case true: kindStr = "val"; break;
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
    Declaration(Location l)
        : Statement(l)
    {
    }
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

    FunctionDeclaration(Location location, std::string name,
                        std::vector<FunctionParameter> parameters, std::unique_ptr<Type> returnType,
                        std::unique_ptr<Statement> body, bool isOperator = false)
        : Declaration(location)
        , name(std::move(name))
        , parameters(std::move(parameters))
        , returnType(std::move(returnType))
        , body(std::move(body))
        , isOperator(isOperator)

    {
    }

    bool checkParam(const FunctionDeclaration* func) const
    {
        if (parameters.size() != func->parameters.size()) {
            return false;
        }
        for (size_t i = 0; i < parameters.size(); ++i) {
            const FunctionParameter& param1 = parameters[i];
            const FunctionParameter& param2 = func->parameters[i];
            if (param1.name != param2.name) {
                return false;
            }
            if (!param1.type || !param2.type || !(*param1.type == *param2.type)) {
                return false;
            }
        }
        return true;
    }

    bool checkReturnType(const FunctionDeclaration* func) const
    {
        return *returnType == *func->returnType;
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

    EnumDeclaration(Location location, std::string name, std::vector<std::string> values)
        : name(std::move(name))
        , values(std::move(values))
        , Declaration(location)

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
private:
    Location location;

public:
    ClassMember(Location l)
        : location(l)
    {
    }
    Location getLocation() const { return location; }

    virtual ~ClassMember()                        = default;
    virtual std::string dump(int level = 0) const = 0;
    virtual std::string getName() const           = 0;
    virtual Type        getType() const           = 0;
};

class PropertyMember : public ClassMember
{
public:
    bool                        immutable;
    std::string                 name;
    std::unique_ptr<Type>       type;
    std::unique_ptr<Expression> initializer;

    PropertyMember(Location location, bool immutable, std::string name, std::unique_ptr<Type> type,
                   std::unique_ptr<Expression> initializer = nullptr)
        : immutable(immutable)
        , name(std::move(name))
        , type(std::move(type))
        , ClassMember(location)
        , initializer(std::move(initializer))
    {
    }

    std::string getName() const override { return name; }
    Type        getType() const override { return *type; }

    std::string dump(int level = 0) const override
    {
        std::string result = indent(level) + "PropertyMember: " + name + "\n";
        result += indent(level + 1) + "Type:\n";
        if (type) result += type->dump(level + 2);
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

    explicit MethodMember(Location location, std::unique_ptr<FunctionDeclaration> function)
        : function(std::move(function))
        , ClassMember(location)
    {
    }

    std::string getName() const override { return function.get()->name; }
    Type        getType() const override { return *function->returnType; }

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

    explicit InitBlockMember(Location location, std::unique_ptr<BlockStatement> block)
        : block(std::move(block))
        , ClassMember(location)
    {
    }

    std::string getName() const override { return ""; }
    Type        getType() const override { return Type(); }


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
    std::vector<FunctionParameter>            constructorParameters;
    std::string                               baseClass;
    std::vector<std::unique_ptr<Expression>>  baseConstructorArgs;
    std::vector<std::unique_ptr<ClassMember>> members;
    bool                                      iter;

    ClassDeclaration(Location location, Kind kind, std::string name,
                     std::vector<FunctionParameter> constructorParameters, std::string baseClass,
                     std::vector<std::unique_ptr<Expression>>  baseConstructorArgs,
                     std::vector<std::unique_ptr<ClassMember>> members)
        : kind(kind)
        , name(std::move(name))
        , constructorParameters(std::move(constructorParameters))
        , baseClass(std::move(baseClass))
        , baseConstructorArgs(std::move(baseConstructorArgs))
        , members(std::move(members))
        , Declaration(location)
    {
    }

    const ClassMember* containMember(const ClassMember* member) const
    {
        for (const auto& m : this->members) {
            if (m->getName() == member->getName()) {
                return m.get();
            }
        }
        return nullptr;
    }
    const ClassMember* containMember(const std::string member) const
    {
        for (const auto& m : this->members) {
            if (m->getName() == member) {
                return m.get();
            }
        }
        return nullptr;
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


        if (!constructorParameters.empty()) {
            result += indent(level + 1) + "ConstructorParameters:\n";
            for (const auto& param : constructorParameters) {
                result += param.dump(level + 2) + "\n";
            }
        }

        if (!baseClass.empty()) {
            result += indent(level + 1) + "BaseClass: " + baseClass + "\n";

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