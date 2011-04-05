#ifndef MSGTESTS_H
#define MSGTESTS_H


/* Forward definitions */
typedef struct Msgtest Msgtest;
typedef struct Submsg Submsg;

struct Msgtest {
    Submsg* rmsg;
    struct {int defined; Submsg* value;} omsg;
    struct {size_t count; Submsg** values;} pmsg;
};


extern ast_err Msgtest_write(ast_runtime*,Msgtest*);
extern ast_err Msgtest_read(ast_runtime*,Msgtest**);
extern ast_err Msgtest_reclaim(ast_runtime*,Msgtest*);
extern size_t Msgtest_get_size(ast_runtime*,Msgtest*);

struct Submsg {
    int32_t f_int32;
};


extern ast_err Submsg_write(ast_runtime*,Submsg*);
extern ast_err Submsg_read(ast_runtime*,Submsg**);
extern ast_err Submsg_reclaim(ast_runtime*,Submsg*);
extern size_t Submsg_get_size(ast_runtime*,Submsg*);

#endif /*MSGTESTS_H*/
