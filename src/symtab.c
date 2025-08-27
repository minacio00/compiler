#include "symtab.h"
#include "memmgr.h"
#include <string.h>
#include <stdio.h>

Scope* scope_create(Scope *parent) {
    Scope *scope = (Scope*)mm_malloc(sizeof(Scope));
    if (!scope) return NULL;
    scope->symbols = NULL;
    scope->parent = parent;
    scope->children = NULL;
    scope->next = NULL;
    scope->depth = parent ? parent->depth + 1 : 0;
    if (parent) {
        scope->next = parent->children;
        parent->children = scope;
    }
    return scope;
}

static void free_symbols(Symbol *sym) {
    while (sym) {
        Symbol *next = sym->next;
        mm_free(sym->name);
        mm_free(sym);
        sym = next;
    }
}

void scope_destroy(Scope *scope) {
    if (!scope) return;
    Scope *child = scope->children;
    while (child) {
        Scope *next_child = child->next;
        scope_destroy(child);
        child = next_child;
    }
    free_symbols(scope->symbols);
    mm_free(scope);
}

Symbol* symtab_add(Scope *scope, const char *name) {
    if (!scope || !name) return NULL;
    Symbol *sym = (Symbol*)mm_malloc(sizeof(Symbol));
    if (!sym) return NULL;
    size_t len = strlen(name);
    sym->name = (char*)mm_malloc(len + 1);
    if (!sym->name) {
        mm_free(sym);
        return NULL;
    }
    strcpy(sym->name, name);
    sym->next = scope->symbols;
    scope->symbols = sym;
    return sym;
}

Symbol* symtab_lookup(Scope *scope, const char *name) {
    while (scope) {
        Symbol *sym = scope->symbols;
        while (sym) {
            if (strcmp(sym->name, name) == 0) return sym;
            sym = sym->next;
        }
        scope = scope->parent;
    }
    return NULL;
}

static void print_scope(const Scope *scope, int indent) {
    if (!scope) return;
    int i;
    for (i = 0; i < indent; i++) printf("  ");
    printf("Escopo (profundidade %zu)\n", scope->depth);
    Symbol *sym = scope->symbols;
    while (sym) {
        for (i = 0; i < indent + 1; i++) printf("  ");
        printf("%s\n", sym->name);
        sym = sym->next;
    }
    Scope *child = scope->children;
    while (child) {
        print_scope(child, indent + 1);
        child = child->next;
    }
}

void symtab_print_scope(Scope *scope, int indent) {
    print_scope(scope, indent);
}

