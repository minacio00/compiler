#ifndef SEMANTICS_H
#define SEMANTICS_H

#include <stddef.h>
#include <stdbool.h>
#include "parser.h"
#include "symtab.h"
#include "types.h"

typedef struct {
    size_t mem_limit;
    Scope *global_scope;
} SemaContext;

SemaContext* sema_create(size_t mem_limit_bytes);
bool semantic_analyze(SemaContext* sc, ASTNode* ast);
void symtab_print(SemaContext* sc);
void sema_destroy(SemaContext* sc);

#endif /* SEMANTICS_H */
