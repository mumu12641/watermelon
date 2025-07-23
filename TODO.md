# Watermelon

## AST design

- PrimitiveType, NamedType(name), GenericType(name, typeArguments)

- LiteralExpression(kind, value), IdentifierExpression(name), BinaryExpression(op, left:Expression, right:Expression),UnaryExpression(op, oprand:Expression), CallExpression(callee:Expression, arguments:vector(Expression)), MemberExpression(object:Expression, property), ArrayExpression(elements: vector(Expression)), LambdaExpression(parameters, body), TypeCheckExpression(expression, type),

- ExpressionStatement(expression), BlockStatement(vector(Expression)), IfStatement(condition, thenBranch:Statement), WhenStatement(subject, cases), ForStatement(variable, iterable, body), ReturnStatement(value),VariableStatement(kind, name, type, initializer)

- FunctionParameter(name, type, defaultValue), FunctionDeclaration(name, parameters, returnType, body: vector(Statement), isOperator), EnumDeclaration(name, values), ClassDeclaration(kind, name, typePara, constructorPara, baseClass, baseTypeArgs, baseConstructorArgs, members)

- ClassMember: PropertyMember(name, type, init), MethodMember(functionDeclaration), InitBlockMember(block)

- Program

## IRGen
- [x] 每个类的字段应该包括所有父类的字段
- [x] 先把所有的function都decl出来
- [x] 每个类应该有自己的function table

- [x] 记得在处理 classdecl 的 init 的时候，要把 inherit 的部分全部弄好，包括父类的 init 和 property
- [x] 初始化类的时候调用 C_malloc_init,然后调用 constructor， constructor 先constructor 再 builtin_init 最后 self_defined_init
- [x] 空指针异常
- [ ] 究竟什么时候要load？？？？？？？？？？？？？？？
- [ ] 在 generateMemberExpression 的时候不能直接用 class_method 要考虑多态
- [ ] 在 generateCallExpression 要处理默认参数
- [ ] 在各个 identifierexpr 的时候要处理好 self 


## TODO
- [x] 子类和父类的初始化函数(done)，init 块(done)
- [x] new expression 能不能在 call 里面实现(done)
- [x] 多态
- [x] functionDecl 必须要有返回值的类型
- [x] 好像只有 .property 没有 .method(done)
- [x] for statement 的 可迭代性

- [x] 把所有的 Type 都给统一起来


- [ ] var & val 区别
- [ ] analyzeEnumDeclaration
- [ ] analyzeArrayExpression
- [ ] analyzeBinaryExpression
- [ ] analyzeLambdaExpression
- [ ] analyzeTypeCheckExpression
- [ ] 泛型
- [ ] data class

## Reference

https://github.com/kneorain/helix-lang
https://github.com/Lioncat2002/HelixLang

## logo
https://www.degraeve.com/img2txt.php
