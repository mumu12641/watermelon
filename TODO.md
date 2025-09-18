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
- [x] 初始化类的时候调用 C_malloc_init,然后调用 constructor， constructor 先 builtin_init 再 constructor 最后 self_defined_init
- [x] 空指针异常
- [x] 究竟什么时候要load？？？？？？？？？？？？？？？回答：只有赋值的时候的左边需要ptr，且只会是memberexpr 或者idexpr
- [x] 在 generateCallExpression 要处理默认参数
- [x] 在各个 identifierexpr 的时候要处理好 self 
- [x] 在 generateMemberExpression 的时候不能直接用 class_method 要考虑多态
- [x] print函数, 感觉还是得用std，然后内联llvm ir比较好

- [x] std str type，一些oprator
- [ ] 但是还是要一个标准的类，String
- [x] 运行时候的空指针 null，检查是不是null，isnull
- [ ] object.clone()，好像还是得有泛型
- [ ] 还是得有泛型
- [ ] 做一个可以自己扩充 cap 的 vector

- [x] 死代码消除：删除未使用的变量和不可达代码
- [x] 常量折叠：编译时计算常量表达式
- [ ] 公共子表达式消除：避免重复计算
- [ ] 循环优化：循环不变量外提、循环展开

clang -emit-llvm -S -O0 test.c -o test.ll 

clang++ -std=c++17 -fPIC -shared -fno-rtti \
  $(llvm-config --cxxflags) \
  $(llvm-config --ldflags) \
  $(llvm-config --libs core support) \
  $(llvm-config --system-libs) \
  -o pass.so \
  pass.cpp   

opt -load-pass-plugin=./pass.so \
  -passes="simple-counter" \
  -S test.ll -o output_optimized.ll

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
