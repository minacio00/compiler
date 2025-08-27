#ifndef SYMTAB_H
#define SYMTAB_H

#include <stddef.h>

/* Símbolo individual */
typedef struct Symbol {
    char *name;
    struct Symbol *next;
} Symbol;

/* Escopo contendo símbolos */
typedef struct Scope {
    Symbol *symbols;
    struct Scope *parent;
    struct Scope *children;
    struct Scope *next;
    size_t depth;
} Scope;

Scope* scope_create(Scope *parent);
void scope_destroy(Scope *scope);
Symbol* symtab_add(Scope *scope, const char *name);
Symbol* symtab_lookup(Scope *scope, const char *name);
void symtab_print_scope(Scope *scope, int indent);

#endif /* SYMTAB_H */
