grammar EnumReflector;

options {
    language=CSharp3;
    output=AST;
}

tokens {
	LEFT_SHIFT = '<<' ;
	ENUM_ENTRY = 'EnumEntry' ;
	ENUM_BEGIN = 'EnumBegin' ;
	ENUM_END = 'EnumEnd' ;
	ENUM = 'enum' ;
	META = '//:' ;
	LPAREN = '(' ;
	RPAREN = ')' ;
	LBRACE = '{' ;
	RBRACE = '}' ;
	EQUALS = '=' ;
	COMMA = ',' ;
	SEMICOLON = ';' ;
	BITFIELD = 'BITFIELD' ;
	NAMESPACE = 'namespace' ;
	EXTENDS = 'extends' ;
	IDQUALIFIER = '::' ;
}

/*
	Parser rules
*/

qualifier_list
	: ( IDQUALIFIER ID )* ;
	
qualified_id
	: ID^ qualifier_list ;
	
enum_conversion_tag 
	: META! ENUM_ENTRY^ LPAREN! STRING RPAREN! ;

shift_expression
	: POSITIVE_INTEGER10 LEFT_SHIFT^ POSITIVE_INTEGER10 ;
		
integer_constant 
	: shift_expression | POSITIVE_INTEGER16 | POSITIVE_INTEGER10 | qualified_id;

value_assignment
	: EQUALS! integer_constant ;

last_enum_entry
	: ID^ value_assignment? enum_conversion_tag? ;

non_last_enum_entry
	: ID^ value_assignment? COMMA! enum_conversion_tag? ;
	
enum_entry_list
	: LBRACE^ non_last_enum_entry* last_enum_entry  RBRACE! ;

extends_clause
	: EXTENDS^ qualified_id ;
	
enum_properties
	: BITFIELD | extends_clause ;
	
enum_begin_meta
	: META! ENUM_BEGIN^ LPAREN! enum_properties? RPAREN! ;
	
enum_end_meta
	: META! ENUM_END^ ;

enum_definition
	: ENUM^ ID enum_entry_list SEMICOLON! ;
	
namespace_wrapper
	: NAMESPACE^ qualified_id LBRACE! enum_definition RBRACE! ;
	
enum_entry_point
	: namespace_wrapper | enum_definition ;
	 	
public parse
	: enum_begin_meta enum_entry_point enum_end_meta ;
									
/*
	Lexical rules
*/
ID  :	('a'..'z'|'A'..'Z'|'_') ('a'..'z'|'A'..'Z'|'0'..'9'|'_')*
    ;
	
WHITESPACE  
	:   ( ' ' | '\t' | '\r' | '\n' ) {$channel=Hidden;} ;

STRING
    	:  '"' ( ~('\\'|'"') )* '"' ;

POSITIVE_INTEGER16
	: '0x' HEX_DIGIT+ ;	
	
POSITIVE_INTEGER10 
	: ( '0' | ( POS_DIGIT DIGIT* ) ) ;
	
fragment POS_DIGIT
	: '1'..'9' ;
	
fragment DIGIT
	: '0'..'9' ;
	
fragment HEX_DIGIT 
	: ('0'..'9'|'a'..'f'|'A'..'F') ;


