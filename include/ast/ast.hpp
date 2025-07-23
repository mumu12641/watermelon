#ifndef AST_HPP
#define AST_HPP

#include "../utils/error.hpp"

#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

static std::string getTreePrefix(const std::string& prefix, bool isLast)
{
    return prefix + (isLast ? "'---" : "|---");
}

static std::string getChildPrefix(const std::string& prefix, bool isLast)
{
    return prefix + (isLast ? "    " : "|   ");
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

    std::string dump(const std::string& prefix = "", bool isLast = true) const
    {
        return prefix + (prefix.empty() ? "" : (isLast ? "'---" : "|---")) + "Type: " + name;
    }
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
}   // namespace std

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
    virtual ~Expression() = default;
    virtual std::string dump(const std::string& prefix = "", bool isLast = true) const = 0;
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

    std::string dump(const std::string& prefix = "", bool isLast = true) const override
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
        return getTreePrefix(prefix, isLast) + "LiteralExpression (" + kindStr + "): " + s;
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


    std::string dump(const std::string& prefix = "", bool isLast = true) const override
    {
        return getTreePrefix(prefix, isLast) + "IdentifierExpression: " + name +
               " type: " + type.getName();
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


    std::string dump(const std::string& prefix = "", bool isLast = true) const override
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

        std::string result = getTreePrefix(prefix, isLast) + "BinaryExpression: " + opStr +
                             " type: " + type.getName() + "\n";
        std::string childPrefix = getChildPrefix(prefix, isLast);
        result += left->dump(childPrefix, false) + "\n";
        result += right->dump(childPrefix, true);
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


    std::string dump(const std::string& prefix = "", bool isLast = true) const override
    {
        std::string opStr  = (op == Operator::NEG) ? "-" : "!";
        std::string result = getTreePrefix(prefix, isLast) + "UnaryExpression: " + opStr + "\n";
        std::string childPrefix = getChildPrefix(prefix, isLast);
        result += operand->dump(childPrefix, true);
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


    std::string dump(const std::string& prefix = "", bool isLast = true) const override
    {
        std::string result =
            getTreePrefix(prefix, isLast) + "CallExpression: type: " + type.getName() + "\n";
        std::string childPrefix = getChildPrefix(prefix, isLast);

        result += childPrefix + (arguments.empty() ? "'---" : "|---") + "Callee:\n";
        std::string calleePrefix = childPrefix + (arguments.empty() ? "    " : "|   ");
        result += callee->dump(calleePrefix, true) + "\n";

        if (!arguments.empty()) {
            result += childPrefix + "'---Arguments:\n";
            std::string argsPrefix = childPrefix + "    ";
            for (size_t i = 0; i < arguments.size(); ++i) {
                bool isLastArg = (i == arguments.size() - 1);
                result += arguments[i]->dump(argsPrefix, isLastArg);
                if (!isLastArg) result += "\n";
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


    std::string dump(const std::string& prefix = "", bool isLast = true) const override
    {
        std::string kindStr    = (kind == Kind::PROPERTY) ? "PROPERTY" : "METHOD";
        std::string memberName = (kind == Kind::PROPERTY) ? property : methodName;

        std::string result = getTreePrefix(prefix, isLast) + "MemberExpression (" + kindStr +
                             "): " + memberName + " type: " + type.getName() + "\n";
        std::string childPrefix = getChildPrefix(prefix, isLast);

        result += childPrefix + (arguments.empty() ? "'---" : "|---") + "Object:\n";
        std::string objectPrefix = childPrefix + (arguments.empty() ? "    " : "|   ");
        result += object->dump(objectPrefix, true) + "\n";

        if (kind == Kind::METHOD && !arguments.empty()) {
            result += childPrefix + "'---Arguments:\n";
            std::string argsPrefix = childPrefix + "    ";
            for (size_t i = 0; i < arguments.size(); ++i) {
                bool isLastArg = (i == arguments.size() - 1);
                result += arguments[i]->dump(argsPrefix, isLastArg);
                if (!isLastArg) result += "\n";
            }
        }
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

    std::string dump(const std::string& prefix = "", bool isLast = true) const override
    {
        std::string result = getTreePrefix(prefix, isLast) + "MethodCallExpression: " + methodName +
                             " type: " + type.getName() + "\n";
        std::string childPrefix = getChildPrefix(prefix, isLast);

        result += childPrefix + (arguments.empty() ? "'---" : "|---") + "Object:\n";
        std::string objectPrefix = childPrefix + (arguments.empty() ? "    " : "|   ");
        result += object->dump(objectPrefix, true) + "\n";

        if (!arguments.empty()) {
            result += childPrefix + "'---Arguments:\n";
            std::string argsPrefix = childPrefix + "    ";
            for (size_t i = 0; i < arguments.size(); ++i) {
                bool isLastArg = (i == arguments.size() - 1);
                result += arguments[i]->dump(argsPrefix, isLastArg);
                if (!isLastArg) result += "\n";
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

    std::string dump(const std::string& prefix = "", bool isLast = true) const override
    {
        throw "ArrayExpression::dump()";
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


    std::string dump(const std::string& prefix = "", bool isLast = true) const override
    {
        throw "LambdaExpression::dump()";
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

    std::string dump(const std::string& prefix = "", bool isLast = true) const override
    {
        throw "TypeCheckExpression::dump()";
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
    virtual ~Statement()                                                               = default;
    virtual std::string dump(const std::string& prefix = "", bool isLast = true) const = 0;
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


    std::string dump(const std::string& prefix = "", bool isLast = true) const override
    {
        std::string result      = getTreePrefix(prefix, isLast) + "ExpressionStatement:\n";
        std::string childPrefix = getChildPrefix(prefix, isLast);
        result += expression->dump(childPrefix, true);
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


    std::string dump(const std::string& prefix = "", bool isLast = true) const override
    {
        std::string result      = getTreePrefix(prefix, isLast) + "BlockStatement:\n";
        std::string childPrefix = getChildPrefix(prefix, isLast);

        for (size_t i = 0; i < statements.size(); ++i) {
            bool isLastStmt = (i == statements.size() - 1);
            result += statements[i]->dump(childPrefix, isLastStmt);
            if (!isLastStmt) result += "\n";
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


    std::string dump(const std::string& prefix = "", bool isLast = true) const override
    {
        std::string result      = getTreePrefix(prefix, isLast) + "IfStatement:\n";
        std::string childPrefix = getChildPrefix(prefix, isLast);

        result += childPrefix + (elseBranch ? "|---" : "|---") + "Condition:\n";
        std::string conditionPrefix = childPrefix + (elseBranch ? "|   " : "|   ");
        result += condition->dump(conditionPrefix, true) + "\n";

        result += childPrefix + (elseBranch ? "|---" : "'---") + "Then:\n";
        std::string thenPrefix = childPrefix + (elseBranch ? "|   " : "    ");
        result += thenBranch->dump(thenPrefix, true);

        if (elseBranch) {
            result += "\n" + childPrefix + "'---Else:\n";
            std::string elsePrefix = childPrefix + "    ";
            result += elseBranch->dump(elsePrefix, true);
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


    std::string dump(const std::string& prefix = "", bool isLast = true) const override
    {
        std::string result      = getTreePrefix(prefix, isLast) + "WhenStatement:\n";
        std::string childPrefix = getChildPrefix(prefix, isLast);

        result += childPrefix + "|---Subject:\n";
        std::string subjectPrefix = childPrefix + "|   ";
        result += subject->dump(subjectPrefix, true) + "\n";

        result += childPrefix + "'---Cases:\n";
        std::string casesPrefix = childPrefix + "    ";

        for (size_t i = 0; i < cases.size(); ++i) {
            bool        isLastCase = (i == cases.size() - 1);
            const auto& c          = cases[i];

            result += casesPrefix + (isLastCase ? "'---" : "|---") + "Case:\n";
            std::string casePrefix = casesPrefix + (isLastCase ? "    " : "|   ");

            result += casePrefix + "|---Value:\n";
            std::string valuePrefix = casePrefix + "|   ";
            result += c.value->dump(valuePrefix, true) + "\n";

            result += casePrefix + "'---Body:\n";
            std::string bodyPrefix = casePrefix + "    ";
            result += c.body->dump(bodyPrefix, true);

            if (!isLastCase) result += "\n";
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


    std::string dump(const std::string& prefix = "", bool isLast = true) const override
    {
        std::string result      = getTreePrefix(prefix, isLast) + "ForStatement:\n";
        std::string childPrefix = getChildPrefix(prefix, isLast);

        result += childPrefix + "|---Variable: " + variable + "\n";

        result += childPrefix + "|---Iterable:\n";
        std::string iterablePrefix = childPrefix + "|   ";
        result += iterable->dump(iterablePrefix, true) + "\n";

        result += childPrefix + "'---Body:\n";
        std::string bodyPrefix = childPrefix + "    ";
        result += body->dump(bodyPrefix, true);

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


    std::string dump(const std::string& prefix = "", bool isLast = true) const override
    {
        std::string result = getTreePrefix(prefix, isLast) + "ReturnStatement:";
        if (value) {
            result += "\n";
            std::string childPrefix = getChildPrefix(prefix, isLast);
            result += value->dump(childPrefix, true);
        }
        return result;
    }
};

class VariableStatement : public Statement
{
public:
    bool                        immutable;
    std::string                 name;
    std::unique_ptr<Type>       declType;
    std::unique_ptr<Type>       initType;
    std::unique_ptr<Expression> initializer;

    VariableStatement(Location location, bool immutable, std::string name,
                      std::unique_ptr<Type> declType, std::unique_ptr<Type> initType,
                      std::unique_ptr<Expression> initializer)
        : immutable(immutable)
        , name(std::move(name))
        , initType(std::move(initType))
        , declType(std::move(declType))
        , initializer(std::move(initializer))
        , Statement(location)
    {
    }


    std::string dump(const std::string& prefix = "", bool isLast = true) const override
    {
        std::string kindStr = immutable ? "val" : "var";
        std::string result =
            getTreePrefix(prefix, isLast) + "VariableStatement (" + kindStr + "): " + name;

        bool hasType = (declType != nullptr);
        bool hasInit = (initializer != nullptr);

        if (hasType || hasInit) {
            result += "\n";
            std::string childPrefix = getChildPrefix(prefix, isLast);

            if (hasType) {
                result += childPrefix + (hasInit ? "|---" : "'---") + "Type:\n";
                std::string typePrefix = childPrefix + (hasInit ? "|   " : "    ");
                result += declType->dump(typePrefix, true);
                if (hasInit) result += "\n";
            }

            if (hasInit) {
                result += childPrefix + "'---Initializer:\n";
                std::string initPrefix = childPrefix + "    ";
                result += initializer->dump(initPrefix, true);
            }
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


    std::string dump(const std::string& prefix = "", bool isLast = true) const
    {
        std::string result = getTreePrefix(prefix, isLast) + "Parameter: " + name;

        bool hasType    = (type != nullptr);
        bool hasDefault = (defaultValue != nullptr);

        if (hasType || hasDefault) {
            result += "\n";
            std::string childPrefix = getChildPrefix(prefix, isLast);

            if (hasType) {
                result += childPrefix + (hasDefault ? "|---" : "'---") + "Type:\n";
                std::string typePrefix = childPrefix + (hasDefault ? "|   " : "    ");
                result += type->dump(typePrefix, true);
                if (hasDefault) result += "\n";
            }

            if (hasDefault) {
                result += childPrefix + "'---DefaultValue:\n";
                std::string defaultPrefix = childPrefix + "    ";
                result += defaultValue->dump(defaultPrefix, true);
            }
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

    std::string dump(const std::string& prefix = "", bool isLast = true) const override
    {
        std::string result = getTreePrefix(prefix, isLast) + "FunctionDeclaration: " + name;
        if (isOperator) {
            result += " (operator)";
        }

        bool hasParams = !parameters.empty();
        bool hasReturn = (returnType != nullptr);
        bool hasBody   = (body != nullptr);

        if (hasParams || hasReturn || hasBody) {
            result += "\n";
            std::string childPrefix = getChildPrefix(prefix, isLast);

            if (hasParams) {
                result += childPrefix + (hasReturn || hasBody ? "|---" : "'---") + "Parameters:\n";
                std::string paramsPrefix = childPrefix + (hasReturn || hasBody ? "|   " : "    ");
                for (size_t i = 0; i < parameters.size(); ++i) {
                    bool isLastParam = (i == parameters.size() - 1);
                    result += parameters[i].dump(paramsPrefix, isLastParam);
                    if (!isLastParam) result += "\n";
                }
                if (hasReturn || hasBody) result += "\n";
            }

            if (hasReturn) {
                result += childPrefix + (hasBody ? "|---" : "'---") + "ReturnType:\n";
                std::string returnPrefix = childPrefix + (hasBody ? "|   " : "    ");
                result += returnType->dump(returnPrefix, true);
                if (hasBody) result += "\n";
            }

            if (hasBody) {
                result += childPrefix + "'---Body:\n";
                std::string bodyPrefix = childPrefix + "    ";
                result += body->dump(bodyPrefix, true);
            }
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


    std::string dump(const std::string& prefix = "", bool isLast = true) const override
    {
        std::string result      = getTreePrefix(prefix, isLast) + "EnumDeclaration: " + name + "\n";
        std::string childPrefix = getChildPrefix(prefix, isLast);

        result += childPrefix + "'---Values:\n";
        std::string valuesPrefix = childPrefix + "    ";

        for (size_t i = 0; i < values.size(); ++i) {
            bool isLastValue = (i == values.size() - 1);
            result += valuesPrefix + (isLastValue ? "'---" : "|---") + values[i];
            if (!isLastValue) result += "\n";
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

    virtual ~ClassMember()                                                             = default;
    virtual std::string dump(const std::string& prefix = "", bool isLast = true) const = 0;
    virtual std::string getName() const                                                = 0;
    virtual Type        getType() const                                                = 0;
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

    std::string dump(const std::string& prefix = "", bool isLast = true) const override
    {
        std::string result = getTreePrefix(prefix, isLast) + "PropertyMember: " + name;

        bool hasType = (type != nullptr);
        bool hasInit = (initializer != nullptr);

        if (hasType || hasInit) {
            result += "\n";
            std::string childPrefix = getChildPrefix(prefix, isLast);

            if (hasType) {
                result += childPrefix + (hasInit ? "|---" : "'---") + "Type:\n";
                std::string typePrefix = childPrefix + (hasInit ? "|   " : "    ");
                result += type->dump(typePrefix, true);
                if (hasInit) result += "\n";
            }

            if (hasInit) {
                result += childPrefix + "'---Initializer:\n";
                std::string initPrefix = childPrefix + "    ";
                result += initializer->dump(initPrefix, true);
            }
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

    std::string dump(const std::string& prefix = "", bool isLast = true) const override
    {
        std::string result      = getTreePrefix(prefix, isLast) + "MethodMember:\n";
        std::string childPrefix = getChildPrefix(prefix, isLast);
        result += function->dump(childPrefix, true);
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


    std::string dump(const std::string& prefix = "", bool isLast = true) const override
    {
        std::string result      = getTreePrefix(prefix, isLast) + "InitBlockMember:\n";
        std::string childPrefix = getChildPrefix(prefix, isLast);
        result += block->dump(childPrefix, true);
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
    bool containInitMember() const
    {
        for (const auto& m : this->members) {
            if (const auto _ = dynamic_cast<const InitBlockMember*>(m.get())) {
                return true;
            }
        }
        return false;
    }

    std::string dump(const std::string& prefix = "", bool isLast = true) const override
    {
        std::string kindStr;
        switch (kind) {
            case Kind::NORMAL: kindStr = "class"; break;
            case Kind::DATA: kindStr = "data class"; break;
            case Kind::BASE: kindStr = "base class"; break;
        }

        std::string result =
            getTreePrefix(prefix, isLast) + "ClassDeclaration (" + kindStr + "): " + name;

        bool hasParams   = !constructorParameters.empty();
        bool hasBase     = !baseClass.empty();
        bool hasBaseArgs = !baseConstructorArgs.empty();
        bool hasMembers  = !members.empty();

        if (hasParams || hasBase || hasMembers) {
            result += "\n";
            std::string childPrefix = getChildPrefix(prefix, isLast);

            if (hasParams) {
                result += childPrefix + (hasBase || hasMembers ? "|---" : "'---") +
                          "ConstructorParameters:\n";
                std::string paramsPrefix = childPrefix + (hasBase || hasMembers ? "|   " : "    ");
                for (size_t i = 0; i < constructorParameters.size(); ++i) {
                    bool isLastParam = (i == constructorParameters.size() - 1);
                    result += constructorParameters[i].dump(paramsPrefix, isLastParam);
                    if (!isLastParam) result += "\n";
                }
                if (hasBase || hasMembers) result += "\n";
            }

            if (hasBase) {
                result += childPrefix + (hasMembers ? "|---" : "'---") + "BaseClass: " + baseClass;
                if (hasBaseArgs) {
                    result += "\n";
                    std::string basePrefix = childPrefix + (hasMembers ? "|   " : "    ");
                    result += basePrefix + "'---BaseConstructorArgs:\n";
                    std::string argsPrefix = basePrefix + "    ";
                    for (size_t i = 0; i < baseConstructorArgs.size(); ++i) {
                        bool isLastArg = (i == baseConstructorArgs.size() - 1);
                        result += baseConstructorArgs[i]->dump(argsPrefix, isLastArg);
                        if (!isLastArg) result += "\n";
                    }
                }
                if (hasMembers) result += "\n";
            }

            if (hasMembers) {
                result += childPrefix + "'---Members:\n";
                std::string membersPrefix = childPrefix + "    ";
                for (size_t i = 0; i < members.size(); ++i) {
                    bool isLastMember = (i == members.size() - 1);
                    result += members[i]->dump(membersPrefix, isLastMember);
                    if (!isLastMember) result += "\n";
                }
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
        for (size_t i = 0; i < declarations.size(); ++i) {
            bool isLastDecl = (i == declarations.size() - 1);
            result += declarations[i]->dump("", isLastDecl);
            if (!isLastDecl) result += "\n";
        }
        return result;
    }
};

#endif