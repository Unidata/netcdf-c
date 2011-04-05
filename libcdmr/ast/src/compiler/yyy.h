#ifndef YYY_H
#define YYY_H


/* Forward definitions */
typedef struct Annotation Annotation;
typedef struct m1 m1;

struct Annotation {
};


extern ast_err Annotation_write(ast_runtime*,Annotation*);
extern ast_err Annotation_read(ast_runtime*,Annotation**);
extern ast_err Annotation_reclaim(ast_runtime*,Annotation*);
extern size_t Annotation_get_size(ast_runtime*,Annotation*);

struct m1 {
    int32_t f1;
    struct {int defined; Annotation* value;} f2;
};


extern ast_err m1_write(ast_runtime*,m1*);
extern ast_err m1_read(ast_runtime*,m1**);
extern ast_err m1_reclaim(ast_runtime*,m1*);
extern size_t m1_get_size(ast_runtime*,m1*);

#endif /*YYY_H*/
