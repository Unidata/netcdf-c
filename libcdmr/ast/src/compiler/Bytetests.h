#ifndef BYTETESTS_H
#define BYTETESTS_H


/* Forward definitions */
typedef struct BytesRequired BytesRequired;
typedef struct BytesOptional BytesOptional;
typedef struct BytesRepeated BytesRepeated;

struct BytesRequired {
    char* f_string;
    bytes_t f_bytes;
};


extern ast_err BytesRequired_write(ast_runtime*,BytesRequired*);
extern ast_err BytesRequired_read(ast_runtime*,BytesRequired**);
extern ast_err BytesRequired_reclaim(ast_runtime*,BytesRequired*);
extern size_t BytesRequired_get_size(ast_runtime*,BytesRequired*);

struct BytesOptional {
    struct {int defined; char* value;} f_string;
    struct {int defined; bytes_t value;} f_bytes;
};


extern ast_err BytesOptional_write(ast_runtime*,BytesOptional*);
extern ast_err BytesOptional_read(ast_runtime*,BytesOptional**);
extern ast_err BytesOptional_reclaim(ast_runtime*,BytesOptional*);
extern size_t BytesOptional_get_size(ast_runtime*,BytesOptional*);

struct BytesRepeated {
    struct {size_t count; char** values;} f_string;
    struct {size_t count; bytes_t* values;} f_bytes;
};


extern ast_err BytesRepeated_write(ast_runtime*,BytesRepeated*);
extern ast_err BytesRepeated_read(ast_runtime*,BytesRepeated**);
extern ast_err BytesRepeated_reclaim(ast_runtime*,BytesRepeated*);
extern size_t BytesRepeated_get_size(ast_runtime*,BytesRepeated*);

#endif /*BYTETESTS_H*/
