#ifndef PRIMTESTS_H
#define PRIMTESTS_H


/* Forward definitions */
typedef struct PrimRepeated PrimRepeated;
typedef struct PrimRequired PrimRequired;
typedef struct PrimOptional PrimOptional;

struct PrimRepeated {
    struct {size_t count; int32_t* values;} f_int32;
    struct {size_t count; int64_t* values;} f_int64;
    struct {size_t count; uint32_t* values;} f_uint32;
    struct {size_t count; uint64_t* values;} f_uint64;
    struct {size_t count; int32_t* values;} f_sint32;
    struct {size_t count; int64_t* values;} f_sint64;
    struct {size_t count; uint32_t* values;} f_fixed32;
    struct {size_t count; uint64_t* values;} f_fixed64;
    struct {size_t count; int32_t* values;} f_sfixed32;
    struct {size_t count; int64_t* values;} f_sfixed64;
    struct {size_t count; double* values;} f_double;
    struct {size_t count; float* values;} f_float;
};


extern ast_err PrimRepeated_write(ast_runtime*,PrimRepeated*);
extern ast_err PrimRepeated_read(ast_runtime*,PrimRepeated**);
extern ast_err PrimRepeated_reclaim(ast_runtime*,PrimRepeated*);
extern size_t PrimRepeated_get_size(ast_runtime*,PrimRepeated*);

struct PrimRequired {
    int32_t f_int32;
    int64_t f_int64;
    uint32_t f_uint32;
    uint64_t f_uint64;
    int32_t f_sint32;
    int64_t f_sint64;
    uint32_t f_fixed32;
    uint64_t f_fixed64;
    int32_t f_sfixed32;
    int64_t f_sfixed64;
    double f_double;
    float f_float;
};


extern ast_err PrimRequired_write(ast_runtime*,PrimRequired*);
extern ast_err PrimRequired_read(ast_runtime*,PrimRequired**);
extern ast_err PrimRequired_reclaim(ast_runtime*,PrimRequired*);
extern size_t PrimRequired_get_size(ast_runtime*,PrimRequired*);

struct PrimOptional {
    struct {int defined; int32_t value;} f_int32;
    struct {int defined; int64_t value;} f_int64;
    struct {int defined; uint32_t value;} f_uint32;
    struct {int defined; uint64_t value;} f_uint64;
    struct {int defined; int32_t value;} f_sint32;
    struct {int defined; int64_t value;} f_sint64;
    struct {int defined; uint32_t value;} f_fixed32;
    struct {int defined; uint64_t value;} f_fixed64;
    struct {int defined; int32_t value;} f_sfixed32;
    struct {int defined; int64_t value;} f_sfixed64;
    struct {int defined; double value;} f_double;
    struct {int defined; float value;} f_float;
};


extern ast_err PrimOptional_write(ast_runtime*,PrimOptional*);
extern ast_err PrimOptional_read(ast_runtime*,PrimOptional**);
extern ast_err PrimOptional_reclaim(ast_runtime*,PrimOptional*);
extern size_t PrimOptional_get_size(ast_runtime*,PrimOptional*);

#endif /*PRIMTESTS_H*/
