#ifndef t_H
#define t_H



typedef enum PhoneType {
    MOBILE=0,
    HOME=1,
    WORK=2
} PhoneType;

typedef struct t {
    int f1_f;
    struct {int exists; float value;} f2_f;
    struct {int count; int* values;} f3_f;
    m2* f4_f;
} t;

extern int t_write(ast_runtime*,t*);
extern int t_read(ast_runtime*,t**);
extern int t_reclaim(ast_runtime*,t*);
extern long t_size(ast_runtime*,t*);

#endif /*t_H*/
