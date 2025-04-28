grammar test;

program: (class | function)* EOF;

class:
	CLASS TYPEID '{' (fileds | method | init)* '}'
	| CLASS TYPEID INHERITS TYPEID '{' (fileds | method | init)* '}';

fileds: ID ':' TYPEID '=' primary_expr;

method:
	FUNCTION ID '(' (decl (',' decl)*)? ')' '{' expr* '}'
	| FUNCTION ID '(' (decl (',' decl)*)? ')' '->' TYPEID '{' expr* '}';
init: INIT '(' (decl (',' decl)*)? ')';
decl: ID ':' TYPEID | ID ':' TYPEID '=' primary_expr;

function: FUNCTION;

expr:
	IF '(' cond_expr ')' '{' expr* '}' ELSE '{' expr* '}'
	| WHILE '(' cond_expr ')' '{' expr* '}'
	| FOR '(' ')'
	| RETURN primary_expr
	| LET (decl (',' decl)*)?
	| ID '=' primary_expr;

basic_expr:
	ID
	| SELF
	| STRING
	| NUMBER
	| NEW TYPEID
	| NEW TYPEID '(' primary_expr* ')'
	| ID '(' primary_expr* ')'
	| basic_expr '.' ID '(' primary_expr* ')';
// math_mul_div: primary_expr math_mul_div_prime | primary_expr; math_mul_div_prime: (MINUS |
// DIVIDE) primary_expr math_mul_div_prime |;
math_mul_div:
	basic_expr (MUL | DIVIDE) math_mul_div
	| basic_expr;

primary_expr:
	basic_expr (PLUS | MINUS) primary_expr
	| math_mul_div
	| cond_expr;

cond_expr:
	basic_expr (EQUAL | GREATER | GREATERE | LESS | LESSE) primary_expr;

CLASS: 'class';
FUNCTION: 'fn';
RETURN: 'return';
IF: 'IF';
ELSE: 'else';
WHILE: 'while';
FOR: 'for';
INHERITS: 'inherits';
LET: 'let';
NEW: 'new';
SELF: 'self';
INIT: 'init';
ARROW: '->';

NOT: '!';
ASSIGN: '=';
PLUS: '+';
MINUS: '-';
DIVIDE: '/';
MUL: '*';
EQUAL: '==';
GREATER: '>';
GREATERE: '=>';
LESS: '<';
LESSE: '<=';

// othens
LBRACE: '{';
RBRACE: '}';
LPAREN: '(';
RPAREN: ')';
SIMICONLON: ';';
PERIOD: '.';
COMMA: ',';
COLON: ':';

TYPEID: '[A-Z][a-zA-Z0-9_]*' | 'int' | 'str';
ID: '[a-z][a-zA-Z0-9_]*';
NUMBER: [0-9]+;
STRING: '"' (~["\\] | '\\' .)* '"';

WS: [ \t\r\n]+ -> skip;