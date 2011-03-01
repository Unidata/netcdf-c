#ifndef AST_DEBUG_H
#define AST_DEBUG_H

#define DEBUG

#ifdef DEBUG
extern int ast_catch(int);
#endif

/* Provide an error catcher */
#ifdef DEBUG
#define CATCH(status) ((status)?ast_catch(status):(status))
#define THROW(status,go) {ast_catch(status);goto go;}
#define ERR(status,err,go) {status=err;ast_catch(status);goto go;}
#else
#define CATCH(status,status,go) (status)
#define THROW(status,go) {goto go;}
#define ERR(status,err,go) {goto go;}
#endif

#endif /*AST_DEBUG_H*/
