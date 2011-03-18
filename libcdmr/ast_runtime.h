#ifndef AST_RUNTIME_H
#define AST_RUNTIME_H

#if SIZEOF_SIZE_T == 64
typedef uint64_t ast_size_t;
#else
typedef uint32_t ast_size_t;
#endif

typedef unsigned int bool_t;

typedef struct bytes_t{
    size_t nbytes;
    uint8_t* bytes;
} bytes_t;

/* Define error codes */
typedef int ast_err;
#define AST_NOERR NC_NOERR
#define AST_EOF -1;
#define AST_ENOMEM NC_ENOMEM
#define AST_EFAIL NC_EFAIL /*generic*/
#define AST_EIO NC_EIO
#define AST_ECURL NC_ECURL

/* Define primitive types enum */
typedef enum ast_sort {
ast_double,
ast_float,
ast_int32,
ast_int64,
ast_uint32,
ast_uint64,
ast_sint32,
ast_sint64,
ast_fixed32,
ast_fixed64,
ast_sfixed32,
ast_sfixed64,
ast_string,
ast_bytes,
ast_bool,
ast_enum,
ast_message
} ast_sort;

/* Define wiretypes */
typedef enum ast_wiretype {
ast_varint     = 0, /* int32, int64, uint32, uint64, sint32, sint64, bool, enum*/
ast_64bit      = 1, /* fixed64, sfixed64, double*/
ast_counted    = 2, /* Length-delimited: string, bytes, embedded messages, packed repeated fields*/
ast_startgroup = 3, /* Start group (deprecated) */
ast_endgroup   = 4, /* end group (deprecated) */
ast_32bit      = 5, /* fixed32, sfixed32, float*/
} ast_wiretype;

/* Define the field modes */
typedef enum ast_fieldmode {
ast_required = 0,
ast_optional = 1,
ast_repeated = 2,
} ast_fieldmode;

/* Max depth of the message tree */
#define MAX_STACK_SIZE 1024

typedef enum ast_iomode {AST_READ, AST_WRITE, AST_FREE} ast_iomode;

/* Forward */
typedef struct ast_runtime_ops ast_runtime_ops;

typedef struct ast_runtime {
    uint64_t uid; /* unique number identifying who created runtime object: e.g. bytesio */
    ast_iomode mode; /* Write/Read/Free (WRF) */
    ast_runtime_ops* ops;
    void* stream; /* data representing stream */
    int errno;
} ast_runtime;

struct ast_runtime_ops {
    size_t (*write)(ast_runtime*,size_t,char*); /* writes stream n bytes at a time */
    size_t (*read)(ast_runtime*,size_t,char*); /* reads stream n bytes at a time */
    ast_err (*mark)(ast_runtime*,size_t); /* limit reads to n bytes */
    ast_err (*unmark)(ast_runtime*); /* restore previous markn limit */
    ast_err (*reclaim)(ast_runtime*); /* reclaim (free) this runtime instance */
};

/* Wrappers for rt->mark and unmark */
extern ast_err ast_mark(ast_runtime* rt, size_t avail);
extern ast_err ast_unmark(ast_runtime* rt);

/* Replacements for malloc, realloc and free */
extern void* ast_alloc(ast_runtime*,size_t); /* allocated zero'd memory */
extern void* ast_realloc(ast_runtime*,void*,size_t);
extern void ast_free(ast_runtime*,void*);

extern size_t ast_ctypesize(ast_runtime* rt, ast_sort sort);

/* Define the primitive W/R/F functions */

extern ast_err ast_read_primitive(ast_runtime*, const ast_sort, const int fieldno, void* val);
extern ast_err ast_write_primitive(ast_runtime*, const ast_sort, const int fieldno, const void* val);

extern ast_err ast_read_enum(ast_runtime*, const ast_sort, const int fieldno, void* val);

extern ast_err ast_read_primitive_packed(ast_runtime*, const ast_sort, const int fieldno, void* val);
extern ast_err ast_write_primitive_packed(ast_runtime*, const ast_sort, const int fieldno, const void* val);

/* Append + extend a REPEAT field */
extern ast_err ast_repeat_append(ast_runtime*,ast_sort,void*,void*);

/* Procedure to write out tag */
extern ast_err ast_write_tag(ast_runtime*, const uint32_t, const uint32_t);

/* Procedure to write out count */
extern ast_err ast_write_count(ast_runtime*, const size_t);

/* Procedure to extract tags */
extern ast_err ast_read_tag(ast_runtime* rt, int* wiretypep, int* fieldnop);

/* Procedure to extract count */
extern ast_err ast_read_count(ast_runtime* rt, size_t* countp);

/* Procedure to skip an unknown field */
extern ast_err ast_skip_field(ast_runtime* rt, int wiretype, int fieldno);

/* Reclaim a runtime instance  */
extern ast_err ast_reclaim(ast_runtime* rt);
#define ast_close ast_reclaim /*alias*/

/* Reclaim an allocated string or bytes_t */
extern ast_err ast_reclaim_string(ast_runtime* rt, char* value);
extern ast_err ast_reclaim_bytes(ast_runtime* rt, bytes_t* value);

/* Procedure to calulate size of a tag */
extern size_t ast_get_tagsize(ast_runtime*,ast_sort,int);

/* Procedure to calulate size of a value */
extern size_t ast_get_size(ast_runtime*, ast_sort, void*);

/**************************************************/
/* Misc. */
extern const char* ast_strerror(ast_err err);
extern void ast_logset(int tf);
extern void ast_log(const char* fmt, ...);

/**************************************************/
/* IO */
/**************************************************/

/* Create a runtime readers and writers backed by a byte buffer */
extern ast_err ast_runtime_bytes(void* buffer, size_t bufferlen, ast_iomode, ast_runtime** astp);

#endif /*AST_RUNTIME_H*/
