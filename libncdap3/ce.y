/* Copyright 2009, UCAR/Unidata and OPeNDAP, Inc.
   See the COPYRIGHT file for more information. */

/*The lines down to DO NOT DELETE ... comment are specific to the C Parser.
  They will be commennted out when building a java parser.
*/

%pure-parser
%lex-param {CEparsestate* parsestate}
%parse-param {CEparsestate* parsestate}
%{#include "ceparselex.h"%}

/*DO NOT DELETE THIS LINE*/

%token  SCAN_WORD
%token  SCAN_STRINGCONST
%token  SCAN_NUMBERCONST

%start constraints

%%

constraints:
	  optquestionmark projections
	| optquestionmark selections
	| optquestionmark projections selections
	| /*empty*/
	;

optquestionmark: '?' | /*empty*/ ;

/* %type NClist<NCprojection*> */
projections:
	projectionlist {projections(parsestate,$1);}
	;

/* %type NClist<NCselection*> */
selections:
	clauselist {selections(parsestate,$1);}
	;

/* %type NClist<NCprojection*> */
projectionlist: //==expr.projection
	  projection
	    {$$=projectionlist(parsestate,(Object)null,$1);}
	| projectionlist ',' projection
	    {$$=projectionlist(parsestate,$1,$3);}
	;

/* %type NCprojection* */
projection: //==expr.proj_clause
	  segmentlist
	    {$$=projection(parsestate,$1);}
	| function
	    {$$=$1;}
	;

function:
	  ident '(' ')'
	    {$$=function(parsestate,$1,null);}
	| ident '(' arg_list ')'
	    {$$=function(parsestate,$1,$3);}
	;

/* %type NClist<OCsegment> */
segmentlist: //==expr.proj_variable
	  segment
	    {$$=segmentlist(parsestate,null,$1);}
	| segmentlist '.' segment
	    {$$=segmentlist(parsestate,$1,$3);}
	;

/* %type OCsegment */
segment: //==expr.component
	  word
	    {$$=segment(parsestate,$1,null);}
	| word rangelist
	    {$$=segment(parsestate,$1,$2);}
	;

/* %type NClist<NCslice*> */
rangelist: 
	  range
	    {$$=rangelist(parsestate,null,$1);}
        | rangelist range
	    {$$=rangelist(parsestate,$1,$2);}
	;

/* %type NCslice* */
range:
	  range1
	    {$$=range(parsestate,$1,null,null);}
	| '[' number ':' number ']'
	    {$$=range(parsestate,$2,null,$4);}
	| '[' number ':' number ':' number ']'
	    {$$=range(parsestate,$2,$4,$6);}
	;

range1: '[' number ']'
	    {$$ = range1(parsestate,$2);}
	;


/* %type NClist<NCselection*> */
clauselist: //==expr.selection
	  sel_clause
	    {$$=clauselist(parsestate,null,$1);}
	| clauselist sel_clause
	    {$$=clauselist(parsestate,$1,$2);}
	;

/* %type NCselection* */
sel_clause: //==expr.clause
	  '&' value rel_op '{' value_list '}'
	    {$$=sel_clause(parsestate,1,$2,$3,$5);} /*1,2 distinguish cases*/
	| '&' value rel_op value
	    {$$=sel_clause(parsestate,2,$2,$3,$4);}
	| '&' boolfunction
	    {$$=$1;}
        ;

value_list:
	  value
	    {$$=value_list(parsestate,null,$1);}
	| value_list ',' value
	    {$$=value_list(parsestate,$1,$3);}
	;

value:
	  var /* can be variable ref or a function */
	    {$$=value(parsestate,$1);}
	| function
	    {$$=value(parsestate,$1);}
	| constant
	    {$$=value(parsestate,$1);}
	;

constant:
	  number
	    {$$=constant(parsestate,$1,SCAN_NUMBERCONST);}
	| string
	    {$$=constant(parsestate,$1,SCAN_STRINGCONST);}
	;

var:
	indexpath
	    {$$=var(parsestate,$1);}
	;




/* %type NClist<NCselection*> */
indexpath:
	  index
	    {$$=indexpath(parsestate,null,$1);}
	| indexpath '.' index
	    {$$=indexpath(parsestate,$1,$3);}
	;

index:
	  word
	    {$$=indexer(parsestate,$1,null);}
	| word array_indices
	    {$$=indexer(parsestate,$1,$2);}
	;

/* %type NClist<NCslice*> */
array_indices:
	  range1
	    {$$=array_indices(parsestate,null,$1);}
        | array_indices range1
	    {$$=array_indices(parsestate,$1,$2);}
	;

boolfunction:
	  ident '(' ')'
	    {$$=function(parsestate,$1,null);}
	| ident '(' arg_list ')'
	    {$$=function(parsestate,$1,$3);}
	;

arg_list: //==expr.arg_list
	  value
	    {$$=arg_list(parsestate,null,$1);}
	| value_list ',' value
	    {$$=arg_list(parsestate,$1,$3);}
	;

/* %type NCsort */
rel_op:
	  '='     {$$=makeselectiontag(NS_EQ);}
	| '>'     {$$=makeselectiontag(NS_GT);}
	| '<'     {$$=makeselectiontag(NS_LT);}
	| '!' '=' {$$=makeselectiontag(NS_NEQ);}
	| '>' '=' {$$=makeselectiontag(NS_GE);}
	| '<' '=' {$$=makeselectiontag(NS_LE);}
	| '=' '~' {$$=makeselectiontag(NS_RE);}
	;

ident:  word
	    {$$ = $1;}
	;

word:  SCAN_WORD
	    {$$ = $1;}
	;

number:  SCAN_NUMBERCONST
	    {$$ = $1;}
	;

string: SCAN_STRINGCONST
	    {$$ = $1;}
	;

%%
