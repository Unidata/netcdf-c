divert(-1)dnl
ifdef(`NUMDIMS',,
	`errprint(`****NUMDIMS should be defined as 0, 1, 2, 3, 4, 5, 6, 7, ...****')m4exit')dnl
ifdef(`KINDVALUE',,
	`errprint(`****KINDVALUE must be defined as  "text", "OneByteInt", "TwoBytInt", "FourByteInt", "EightByteInt", "FourByteReal", or "EightByteReal"****\n')m4exit')dnl
ifdef(`PUTORGET',,
	`errprint(`****PUTORGET must be defined as "put" or "get"****\n')m4exit')dnl
dnl# NCOLONS(1) = ":", NCOLONS(2) = ":, :", etc.
define(`NCOLONS',`ifelse($1, 1, `:', `:, '`NCOLONS(decr($1))')')
define(`COLONS',`NCOLONS(NUMDIMS)')
define(`NUMERIC_DECL',`$1 (kind = KINDVALUE)')
define(`TEXT_DEFINES',
	`define(`TYPE',`character (len = *)')define(`NCKIND',`text')')
define(`INT1_DEFINES',
	`define(`TYPE',`NUMERIC_DECL(integer)')define(`NCKIND',`int1')')
define(`INT2_DEFINES',
	`define(`TYPE',`NUMERIC_DECL(integer)')define(`NCKIND',`int2')')
define(`INT4_DEFINES',
	`define(`TYPE',`NUMERIC_DECL(integer)')define(`NCKIND',`int')')
define(`INT8_DEFINES',
	`define(`TYPE',`NUMERIC_DECL(integer)')define(`NCKIND',`int')')
define(`FLT4_DEFINES',
	`define(`TYPE',`NUMERIC_DECL(real)')define(`NCKIND',`real')')
define(`FLT8_DEFINES',
	`define(`TYPE',`NUMERIC_DECL(real)')define(`NCKIND',`double')')
ifelse(KINDVALUE,text,`TEXT_DEFINES',
       KINDVALUE,OneByteInt,`INT1_DEFINES',
       KINDVALUE,TwoByteInt,`INT2_DEFINES',
       KINDVALUE,FourByteInt,`INT4_DEFINES',
       KINDVALUE,EightByteInt,`INT8_DEFINES',
       KINDVALUE,FourByteReal,`FLT4_DEFINES',
       KINDVALUE,EightByteReal,`FLT8_DEFINES',
       
	`errprint(`****KINDVALUE must be "text", "OneByteInt", "TwoBytInt", "FourByteInt", "EightByteInt", "FourByteReal", or "EightByteReal"****\n')')
ifelse(PUTORGET,`put',`define(`IN_OR_OUT',` in')',
       PUTORGET,`get',`define(`IN_OR_OUT',`out')',
       
	`errprint(`****PUTORGET must be "put" or "get"****')')
define(`ND_KINDVALUE',NUMDIMS`'D_`'KINDVALUE)
define(`NF90_AFUN',`nf90_'PUTORGET`_var_'ND_KINDVALUE)
define(`NF90_1FUN',`nf90_'PUTORGET`_var_'KINDVALUE)
define(`NF_MFUN',`nf_'PUTORGET`_varm_'NCKIND)
define(`NF_SFUN',`nf_'PUTORGET`_vars_'NCKIND)
define(`NF_AFUN',`nf_'PUTORGET`_vara_'NCKIND)
define(`NF_1FUN',`nf_'PUTORGET`_var1_'NCKIND)
define(`m4_rename',`ifdef(`$1',`define(`m4'_`$1',defn(`$1'))undefine(`$1')')')
m4_rename(`index')
m4_rename(`len')
m4_rename(`shift')
divert`'dnl
