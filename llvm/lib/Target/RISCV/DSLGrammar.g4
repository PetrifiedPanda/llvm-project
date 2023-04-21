
grammar DSLGrammar;
translationUnit: architectureDefinition*;
architectureDefinition: 'arch' ID (':' ID)? '{' ( (definition | overwrite) (',' (definition | overwrite))* ','?)?  '}';
definition: 'let' ID (':' type)? '=' expr;
overwrite: memberAccess '=' expr;
memberAccess: ID ('.' ID)*;
type: 'bool' | 'int' | listType | ID;
listType: 'list' '<' type '>';
expr: ID | BOOL | INT | list | obj;
list: '[' (expr (',' expr)* ','?)? ']';
obj: '{' (overwrite (',' overwrite)* ','?)?  '}';

BOOL: 'true' | 'false';
ID: [a-zA-Z_][a-zA-Z0-9_]*;
INT: [0-9]+;
COMMENT: '/*' .*? '*/' -> skip;
LINE_COMMENT: '//' ~('\n' | '\r')* -> skip;
SKIP_WHITESPACE: [ \r\t\n]+ -> skip;
