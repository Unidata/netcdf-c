#ifndef ENUMTESTS_H
#define ENUMTESTS_H



typedef enum Testenum {
    ECON1=1,
    ECON2=2,
    ECON3=3,
    ECON4=4,
    ECON5=5,
    ECON6=6
} Testenum;

/* Forward definitions */
typedef struct Enumtest Enumtest;

struct Enumtest {
    Testenum renum;
    struct {int defined; Testenum value;} oenum;
    struct {size_t count; Testenum* values;} penum;
    struct {size_t count; Testenum* values;} ppenum;
};


extern ast_err Enumtest_write(ast_runtime*,Enumtest*);
extern ast_err Enumtest_read(ast_runtime*,Enumtest**);
extern ast_err Enumtest_reclaim(ast_runtime*,Enumtest*);
extern size_t Enumtest_get_size(ast_runtime*,Enumtest*);

#endif /*ENUMTESTS_H*/
