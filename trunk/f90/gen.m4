define(NVES, dnl
`define(`NUMDIMS',0)dnl # not used, but permits sharing f90aux.m4 
define(`KINDVALUE',$1)dnl
define(`PUTORGET', $2)dnl
include(f90aux.m4)dnl
include(nves.m4)
')dnl

define(NVESPUT48, dnl
`define(`NUMDIMS',0)dnl # not used, but permits sharing f90aux.m4 
define(`KINDVALUE',$1)dnl
define(`PUTORGET', $2)dnl
include(f90aux.m4)dnl
include(nvesput48.m4)
')dnl

define(NVESGET48, dnl
`define(`NUMDIMS',0)dnl # not used, but permits sharing f90aux.m4 
define(`KINDVALUE',$1)dnl
define(`PUTORGET', $2)dnl
include(f90aux.m4)dnl
include(nvesget48.m4)
')dnl

define(NVEA, dnl
`define(`NUMDIMS',$1)dnl
define(`KINDVALUE',$2)dnl
define(`PUTORGET', $3)dnl
include(f90aux.m4)dnl
include(nvea.m4)
')dnl

define(NVEAPUT48, dnl
`define(`NUMDIMS',$1)dnl
define(`KINDVALUE',$2)dnl
define(`PUTORGET', $3)dnl
include(f90aux.m4)dnl
include(nveaput48.m4)
')dnl

define(NVEAGET48, dnl
`define(`NUMDIMS',$1)dnl
define(`KINDVALUE',$2)dnl
define(`PUTORGET', $3)dnl
include(f90aux.m4)dnl
include(nveaget48.m4)
')dnl

NVES(OneByteInt, put)
NVES(TwoByteInt, put)
NVESPUT48(FourByteInt, put)
NVESPUT48(EightByteInt, put)
NVES(FourByteReal, put)
NVES(EightByteReal, put)
NVES(OneByteInt, get)
NVES(TwoByteInt, get)
NVESGET48(FourByteInt, get)
NVESGET48(EightByteInt, get)
NVES(FourByteReal, get)
NVES(EightByteReal, get)

NVEA(1, OneByteInt, put)
NVEA(2, OneByteInt, put)
NVEA(3, OneByteInt, put)
NVEA(4, OneByteInt, put)
NVEA(5, OneByteInt, put)
NVEA(6, OneByteInt, put)
NVEA(7, OneByteInt, put)
NVEA(1, TwoByteInt, put)
NVEA(2, TwoByteInt, put)
NVEA(3, TwoByteInt, put)
NVEA(4, TwoByteInt, put)
NVEA(5, TwoByteInt, put)
NVEA(6, TwoByteInt, put)
NVEA(7, TwoByteInt, put)
NVEAPUT48(1, FourByteInt, put)
NVEAPUT48(2, FourByteInt, put)
NVEAPUT48(3, FourByteInt, put)
NVEAPUT48(4, FourByteInt, put)
NVEAPUT48(5, FourByteInt, put)
NVEAPUT48(6, FourByteInt, put)
NVEAPUT48(7, FourByteInt, put)
NVEAPUT48(1, EightByteInt, put)
NVEAPUT48(2, EightByteInt, put)
NVEAPUT48(3, EightByteInt, put)
NVEAPUT48(4, EightByteInt, put)
NVEAPUT48(5, EightByteInt, put)
NVEAPUT48(6, EightByteInt, put)
NVEAPUT48(7, EightByteInt, put)
NVEA(1, FourByteReal, put)
NVEA(2, FourByteReal, put)
NVEA(3, FourByteReal, put)
NVEA(4, FourByteReal, put)
NVEA(5, FourByteReal, put)
NVEA(6, FourByteReal, put)
NVEA(7, FourByteReal, put)
NVEA(1, EightByteReal, put)
NVEA(2, EightByteReal, put)
NVEA(3, EightByteReal, put)
NVEA(4, EightByteReal, put)
NVEA(5, EightByteReal, put)
NVEA(6, EightByteReal, put)
NVEA(7, EightByteReal, put)
NVEA(1, OneByteInt, get)
NVEA(2, OneByteInt, get)
NVEA(3, OneByteInt, get)
NVEA(4, OneByteInt, get)
NVEA(5, OneByteInt, get)
NVEA(6, OneByteInt, get)
NVEA(7, OneByteInt, get)
NVEA(1, TwoByteInt, get)
NVEA(2, TwoByteInt, get)
NVEA(3, TwoByteInt, get)
NVEA(4, TwoByteInt, get)
NVEA(5, TwoByteInt, get)
NVEA(6, TwoByteInt, get)
NVEA(7, TwoByteInt, get)
NVEAGET48(1, FourByteInt, get)
NVEAGET48(2, FourByteInt, get)
NVEAGET48(3, FourByteInt, get)
NVEAGET48(4, FourByteInt, get)
NVEAGET48(5, FourByteInt, get)
NVEAGET48(6, FourByteInt, get)
NVEAGET48(7, FourByteInt, get)
NVEAGET48(1, EightByteInt, get)
NVEAGET48(2, EightByteInt, get)
NVEAGET48(3, EightByteInt, get)
NVEAGET48(4, EightByteInt, get)
NVEAGET48(5, EightByteInt, get)
NVEAGET48(6, EightByteInt, get)
NVEAGET48(7, EightByteInt, get)
NVEA(1, FourByteReal, get)
NVEA(2, FourByteReal, get)
NVEA(3, FourByteReal, get)
NVEA(4, FourByteReal, get)
NVEA(5, FourByteReal, get)
NVEA(6, FourByteReal, get)
NVEA(7, FourByteReal, get)
NVEA(1, EightByteReal, get)
NVEA(2, EightByteReal, get)
NVEA(3, EightByteReal, get)
NVEA(4, EightByteReal, get)
NVEA(5, EightByteReal, get)
NVEA(6, EightByteReal, get)
NVEA(7, EightByteReal, get)
