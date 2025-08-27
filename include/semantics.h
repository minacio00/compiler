#ifndef SEMANTICS_H
#define SEMANTICS_H

#include <stddef.h>
#include <stdbool.h>
#include "parser.h"
#include "symtab.h"
#include "types.h"

typedef struct {
    size_t mem_limit;
    SymTab *symtab;
} SemaContext;

SemaContext* sema_create(size_t mem_limit_bytes);
bool semantic_analyze(SemaContext* sc, ASTNode* ast);
/* Imprime a tabela de símbolos acumulada e relatório de memória. */
void symtab_print(SemaContext* sc);
void sema_destroy(SemaContext* sc);

#endif /* SEMANTICS_H */
