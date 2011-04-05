/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

/*
 * Bison 2.4.2 grammar file for Google Protocol Buffers .proto files
 */

%error-verbose
%name-prefix "Protobuf"

%define public
%define package "unidata.protobuf.compiler"
%define extends "ProtobufActions"
%define throws "IOException"
%define lex_throws "IOException"

%code imports {
import java.io.*;
import unidata.protobuf.compiler.AST.Position;
}

%code {

//////////////////////////////////////////////////
// Constructors

    public ProtobufParser()
    {
	this((ASTFactory)null);
    }

    public ProtobufParser(ASTFactory factory)
    {
        super(factory);  
        if(factory == null) factory = new ASTFactoryDefault();
        this.yylexer = new ProtobufLexer(this);
        this.lexstate = (ProtobufLexer)this.yylexer;
    }
    
    public boolean parse(String filename, Reader stream) throws IOException
    {
	reset(filename,stream);
        return parse();
    }

    public Object parseWarning(String s)
    {
	lexstate.yywarning(s);
	return null;
    }

    public Object parseError(String s)
    {
	yyerror(s);
	return null;
    }
}

%token IMPORT PACKAGE OPTION MESSAGE EXTEND EXTENSIONS
%token ENUM SERVICE RPC RETURNS
%token TO MAX REQUIRED OPTIONAL REPEATED
%token DOUBLE FLOAT INT32 INT64 UINT32 UINT64
%token SINT32 SINT64 FIXED32 FIXED64 SFIXED32 SFIXED64
%token BOOL STRING BYTES
%token ENDFILE

%token INTCONST FLOATCONST STRINGCONST
%token TRUE FALSE POSNAN POSINF NEGNAN NEGINF
%token NAME

%start root

%%

root: protobuffile
	    {protobufroot($1);}

protobuffile:
	decllist ENDFILE
	    {$$=protobuffile($1);}
	;

packagedecl:
	PACKAGE packagename ';'
	    {$$=packagedecl($2);}
	;

importstmt:
	importprefix protobuffile
	    {$$=importstmt($1,$2);}
        ;
        
importprefix:
        IMPORT STRINGCONST ';'
	    {$$=importprefix($2); if(!filepush($$)) return YYABORT; }

/* pushfile: {if(!filepush()) {return YYABORT;};} */

decllist:
	  /*empty*/
	    {$$=decllist(null,null);}
	| decllist decl
	    {if($2 != null) $$=decllist($1,$2); else $$=$1;}
	;	

decl:
          message       {$$=$1;}
        | extend        {$$=$1;}
        | enumtype      {$$=$1;}
        | optionstmt    {$$=$1;}
        | service       {$$=$1;}
        | packagedecl   {$$=$1;}
        | importstmt  {$$=$1;}
        | /*empty*/ ';' {$$=null;}
        ;

optionstmt:
        OPTION option ';'
   	    {$$=$2;}
        ;

option:
	  name '=' constant
   	    {$$=option($1,$3);}
	| '(' name ')' '=' constant
   	    {$$=option($2,$5);}
        ;

message:
        MESSAGE name messagebody
	    {$$=message($2,$3);}
        ;

extend:
        EXTEND path '{' fieldlist '}'
	    {$$=extend($2,$4);}
        ;

/* A list of Fields or Groups */
fieldlist:
	  /*empty*/
	    {$$=fieldlist(null,null);}
	| fieldlist field
	    {$$=fieldlist($1,$2);}
	| fieldlist ';'
	    {$$=$1;}
	;

enumtype:
        ENUM name '{' enumlist '}'
	    {$$=enumtype($2,$4);}
        ;

enumlist:
	  /*empty*/
	    {$$=enumlist(null,null);}
	| enumlist optionstmt
	    {$$=enumlist($1,$2);}
	| enumlist enumvalue
	    {$$=enumlist($1,$2);}
	| enumlist ';' {$$=$1;}
	;

enumvalue:
          name '=' intconst
	    {if(($$=enumvalue($1,$3,null))==null) {return YYABORT;}}
        | name '=' intconst '[' enumoptionlist ']'
	    {if(($$=enumvalue($1,$3,$5))==null) {return YYABORT;}}
        ;

enumoptionlist:
	  option
	    {$$=enumoptionlist(null,$1);}
	| enumoptionlist ',' option
	    {$$=enumoptionlist($1,$3);}
	;


service:
        SERVICE name '{' servicecaselist '}'
	    {$$=service($2,$4);}
        ;

servicecaselist:
	  /*empty*/
	    {$$=servicecaselist(null,null);}
	| servicecaselist servicecase
	    {$$=servicecaselist($1,$2);}
	;

servicecase:
	  optionstmt {$$=$1;}
	| rpc    {$$=$1;}
	| ';'    {$$=null;}
	;

rpc:
          RPC name '(' usertype ')' RETURNS '(' usertype ')' ';'
	    {$$=rpc($2,$4,$8,null);}
        | RPC name '(' usertype ')' RETURNS '(' usertype ')'
          '{' optionstmtlist '}'
	    {$$=rpc($2,$4,$8,$11);}
        ;

optionstmtlist:
	  /*empty*/
	    {$$=optionstmtlist(null,null);}
	| optionstmtlist optionstmt
	    {$$=optionstmtlist($1,$2);}
	;	

messagebody:
        '{' messageelementlist '}'
	    {$$=$2;}
        ;

messageelementlist:
	  /*empty*/
	    {$$=messageelementlist(null,null);}
	| messageelementlist messageelement
	    {$$=messageelementlist($1,$2);}
	;

messageelement:
          field      {$$=$1;}
        | enumtype   {$$=$1;}
        | message    {$$=$1;}
        | extend     {$$=$1;}
        | extensions {$$=$1;}
        | optionstmt {$$=$1;}
        | ';'        {$$=null;}
        ;

// tag number must be 2^28-1 or lower
field:
	  cardinality type name '=' intconst  ';'
	    {$$=field($1,$2,$3,$5,null);}
	| cardinality type name '=' intconst '[' fieldoptionlist ']'  ';'
	    {$$=field($1,$2,$3,$5,$7);}
	| cardinality type name '=' intconst  messagebody 
	    {$$=group($1,$2,$3,$5,$6);}
	;

fieldoptionlist:
	  fieldoption
	    {$$=fieldoptionlist(null,$1);}
	| fieldoptionlist ',' fieldoption
	    {$$=fieldoptionlist($1,$3);}
	;

fieldoption:
          option
	    {$$=$1;}
        ;

extensions:
	EXTENSIONS extensionlist ';'
	    { $$=extensions($2);}
	;

extensionlist:
          extensionrange
	    {$$=extensionlist(null,$1);}
        | extensionlist ',' extensionrange
	    {$$=extensionlist($1,$3);}
        ;

extensionrange:
          INTCONST
	    {if(($$=extensionrange($1,null)) == null) return YYABORT;}
        | INTCONST TO INTCONST
	    {if(($$=extensionrange($1,$3)) == null) return YYABORT;}
        | INTCONST TO MAX
	    {if(($$=extensionrange($1,null)) == null) return YYABORT;}
        ;

cardinality:
	  REQUIRED {$$=$1;}
	| OPTIONAL {$$=$1;}
	| REPEATED {$$=$1;}
        ;

type:
	  DOUBLE   {$$=$1;}
	| FLOAT    {$$=$1;}
	| INT32    {$$=$1;}
	| INT64    {$$=$1;}
	| UINT32   {$$=$1;}
	| UINT64   {$$=$1;}
	| SINT32   {$$=$1;}
	| SINT64   {$$=$1;}
	| FIXED32  {$$=$1;}
	| FIXED64  {$$=$1;}
	| SFIXED32 {$$=$1;}
	| SFIXED64 {$$=$1;}
	| BOOL     {$$=$1;}
	| STRING   {$$=$1;}
	| BYTES    {$$=$1;}
	| usertype {$$=$1;}


usertype:
	path {$$=$1;}
	;

packagename:
	path {$$=$1;}
	;

constant:
          path  {$$=$1;}
	| compound {$$=$1;}
	| intconst {$$=$1;}
	| floatconst {$$=$1;}
	| STRINGCONST  {$$=new Constant(STRINGCONST,$1);}
	| TRUE {$$=new Constant(TRUE,$1);}
	| FALSE {$$=new Constant(FALSE,$1);}
	| POSNAN {$$=new Constant(POSNAN,$1);}
	| POSINF {$$=new Constant(POSINF,$1);}
	| '-' POSNAN {$$=new Constant(POSNAN, "-nan");}
	| '-' POSINF {$$=new Constant(POSINF, "-inf");}
        ;

intconst:
	  INTCONST {$$=new Constant(INTCONST,$1);}
	| '+' INTCONST {$$=new Constant(INTCONST,("+"+$2));}
	| '-' INTCONST {$$=new Constant(INTCONST,("+"+$2));}
	;

floatconst:
	  FLOATCONST {$$=new Constant(FLOATCONST,$1);}
	| '+' FLOATCONST {$$=new Constant(FLOATCONST,("+"+$1));}
	| '-' FLOATCONST {$$=new Constant(FLOATCONST,("-"+$1));}
	;

/* Relative or absolute */
path:
	  relpath {$$=path($1,false);}
	| '.' relpath {$$=path($2,true);}
	;

relpath:
	  name
	    {$$=relpath(null,$1);}
	| relpath '.' name
	    {$$=relpath($1,$3);}
	;

compound:
	'{' pairlist '}' {$$=compound($1);}
	;

pairlist:
          pair
	    {$$=pairlist(null,$1);}
        | pairlist pair
	    {$$=pairlist($1,$2);}
        ;

pair:
	name ':' constant
	    {$$=pair($1,$3);}

/* A Name can be a keyword */

name:
	text {$$=new Constant(NAME,$1);}
	;
text:
	  NAME		{$$=$1;}
	| IMPORT	{$$=$1;}
	| PACKAGE	{$$=$1;}
	| OPTION	{$$=$1;}
	| MESSAGE	{$$=$1;}
	| EXTEND	{$$=$1;}
	| EXTENSIONS	{$$=$1;}
	| ENUM		{$$=$1;}
	| SERVICE	{$$=$1;}
	| RPC		{$$=$1;}
	| RETURNS	{$$=$1;}
	| TO		{$$=$1;}
	| MAX		{$$=$1;}
	| REQUIRED	{$$=$1;}
	| OPTIONAL	{$$=$1;}
	| REPEATED	{$$=$1;}
	;
