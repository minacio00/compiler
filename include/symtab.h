#ifndef SYMTAB_H
#define SYMTAB_H

#include <stddef.h>
#include <stdbool.h>
#include "types.h"

/* Classes possíveis de símbolos */
typedef enum {
    SYM_VAR,
    SYM_PARAM,
    SYM_FUNC
} SymClass;

/* Estrutura de um símbolo individual */
typedef struct Symbol {
    char *name;            /* nome do símbolo */
    SymClass sclass;       /* var, param ou func */
    Type *type;            /* tipo associado */
    size_t scope_id;       /* escopo onde foi declarado */
    int line_decl;         /* linha da declaração */
    void *extra;           /* informação adicional */
    struct Symbol *next;   /* próximo na lista do bucket */
} Symbol;

/* Escopo com tabela de dispersão de símbolos */
typedef struct Scope {
    size_t id;             /* identificador do escopo */
    Symbol **buckets;      /* buckets da hash table */
    size_t bucket_count;   /* quantidade de buckets */
    struct Scope *parent;  /* escopo pai (aninhamento) */
} Scope;

/* Tabela de símbolos contendo pilha de escopos */
typedef struct SymTab {
    Scope *current;        /* escopo atual (topo da pilha) */
    size_t next_id;        /* próximo id de escopo */
} SymTab;

SymTab* symtab_create(void);
void symtab_destroy(SymTab *st);

void symtab_enter_scope(SymTab *st);
void symtab_leave_scope(SymTab *st);

bool symtab_insert(SymTab *st, const Symbol *sym);
Symbol* symtab_lookup(SymTab *st, const char *name);

/* Imprime a tabela de símbolos. Usado internamente pela camada semântica. */
void symtab_dump(SymTab *st);

#endif /* SYMTAB_H */

