#ifndef t2_H
#define t2_H


typedef struct m2 {
    int field2_f;
} m2;

extern int m2_write(ast_runtime*,m2*);
extern int m2_read(ast_runtime*,m2**);
extern int m2_reclaim(ast_runtime*,m2*);
extern int m2_default(ast_runtime*,m2**);
extern long m2_size(ast_runtime*,m2*);

#endif /*t2_H*/
