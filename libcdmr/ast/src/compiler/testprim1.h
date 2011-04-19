#ifndef testprim1_H
#define testprim1_H


typedef struct PrimRequired {
    uint64_t f_fixed64;
} PrimRequired;

extern int PrimRequired_write(ast_runtime*,PrimRequired*);
extern int PrimRequired_read(ast_runtime*,PrimRequired**);
extern int PrimRequired_reclaim(ast_runtime*,PrimRequired*);
extern long PrimRequired_size(ast_runtime*,PrimRequired*);

typedef struct PrimOptional {
    struct {int exists; int32_t value;} f_int32;
    struct {int exists; int64_t value;} f_int64;
    struct {int exists; uint32_t value;} f_uint32;
    struct {int exists; uint64_t value;} f_uint64;
    struct {int exists; int32_t value;} f_sint32;
    struct {int exists; int64_t value;} f_sint64;
    struct {int exists; uint32_t value;} f_fixed32;
    struct {int exists; uint64_t value;} f_fixed64;
    struct {int exists; int32_t value;} f_sfixed32;
    struct {int exists; int64_t value;} f_sfixed64;
    struct {int exists; double value;} f_double;
    struct {int exists; float value;} f_float;
} PrimOptional;

extern int PrimOptional_write(ast_runtime*,PrimOptional*);
extern int PrimOptional_read(ast_runtime*,PrimOptional**);
extern int PrimOptional_reclaim(ast_runtime*,PrimOptional*);
extern long PrimOptional_size(ast_runtime*,PrimOptional*);

typedef struct PrimRepeated {
    struct {int count; int32_t* values;} f_int32;
    struct {int count; int64_t* values;} f_int64;
    struct {int count; uint32_t* values;} f_uint32;
    struct {int count; uint64_t* values;} f_uint64;
    struct {int count; int32_t* values;} f_sint32;
    struct {int count; int64_t* values;} f_sint64;
    struct {int count; uint32_t* values;} f_fixed32;
    struct {int count; uint64_t* values;} f_fixed64;
    struct {int count; int32_t* values;} f_sfixed32;
    struct {int count; int64_t* values;} f_sfixed64;
    struct {int count; double* values;} f_double;
    struct {int count; float* values;} f_float;
} PrimRepeated;

extern int PrimRepeated_write(ast_runtime*,PrimRepeated*);
extern int PrimRepeated_read(ast_runtime*,PrimRepeated**);
extern int PrimRepeated_reclaim(ast_runtime*,PrimRepeated*);
extern long PrimRepeated_size(ast_runtime*,PrimRepeated*);

#endif /*testprim1_H*/
