#include "symtab.h"
#include "memmgr.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define SYMTAB_BUCKETS 64

/* Assumimos tamanhos: inteiro=4B, decimal=8B, texto[n]=nB */

/* Hash simples para strings (djb2) */
static unsigned long sym_hash(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + (unsigned long)c;
    }
    return hash;
}

static Scope* scope_new(size_t id, Scope *parent) {
    Scope *s = (Scope*)mm_malloc(sizeof(Scope));
    mm_usage_guard();
    if (!s) return NULL;
    s->id = id;
    s->parent = parent;
    s->bucket_count = SYMTAB_BUCKETS;
    s->buckets = (Symbol**)mm_malloc(sizeof(Symbol*) * s->bucket_count);
    mm_usage_guard();
    if (!s->buckets) {
        mm_free(s);
        return NULL;
    }
    {
        size_t i;
        for (i = 0; i < s->bucket_count; ++i) s->buckets[i] = NULL;
    }
    return s;
}

SymTab* symtab_create(void) {
    SymTab *st = (SymTab*)mm_malloc(sizeof(SymTab));
    mm_usage_guard();
    if (!st) return NULL;
    st->next_id = 1; /* global = 0 */
    st->current = scope_new(0, NULL);
    if (!st->current) {
        mm_free(st);
        return NULL;
    }
    return st;
}

static void free_symbols(Symbol *sym) {
    while (sym) {
        Symbol *next = sym->next;
        mm_free(sym->name);
        mm_free(sym);
        sym = next;
    }
}

void symtab_leave_scope(SymTab *st) {
    if (!st || !st->current) return;
    Scope *to_pop = st->current;
    st->current = to_pop->parent;
    {
        size_t i;
        for (i = 0; i < to_pop->bucket_count; ++i) {
            free_symbols(to_pop->buckets[i]);
        }
    }
    mm_free(to_pop->buckets);
    mm_free(to_pop);
}

void symtab_destroy(SymTab *st) {
    if (!st) return;
    while (st->current) symtab_leave_scope(st);
    mm_free(st);
}

void symtab_enter_scope(SymTab *st) {
    if (!st) return;
    Scope *s = scope_new(st->next_id++, st->current);
    if (s) st->current = s;
}

bool symtab_insert(SymTab *st, const Symbol *sym) {
    if (!st || !st->current || !sym || !sym->name) return false;
    unsigned long h = sym_hash(sym->name) % st->current->bucket_count;

    /* Verifica se já existe no escopo atual */
    {
        Symbol *it;
        for (it = st->current->buckets[h]; it; it = it->next) {
            if (strcmp(it->name, sym->name) == 0) return false; /* duplicado */
        }
    }

    Symbol *copy = (Symbol*)mm_malloc(sizeof(Symbol));
    mm_usage_guard();
    if (!copy) return false;
    memcpy(copy, sym, sizeof(Symbol));

    size_t len = strlen(sym->name);
    copy->name = (char*)mm_malloc(len + 1);
    mm_usage_guard();
    if (!copy->name) {
        mm_free(copy);
        return false;
    }
    strcpy(copy->name, sym->name);
    copy->scope_id = st->current->id;
    copy->next = st->current->buckets[h];
    st->current->buckets[h] = copy;

    return true;
}

Symbol* symtab_lookup(SymTab *st, const char *name) {
    if (!st || !name) return NULL;
    {
        Scope *s;
        for (s = st->current; s; s = s->parent) {
            unsigned long h = sym_hash(name) % s->bucket_count;
            Symbol *it;
            for (it = s->buckets[h]; it; it = it->next) {
                if (strcmp(it->name, name) == 0) return it;
            }
        }
    }
    return NULL;
}

static const char* class_str(SymClass c) {
    switch (c) {
        case SYM_VAR:   return "var";
        case SYM_PARAM: return "param";
        case SYM_FUNC:  return "func";
        default:        return "?";
    }
}

static const char* type_str(const Type *t) {
    if (!t) return "?";
    switch (t->kind) {
        case TY_INT:
            return "int";
        case TY_DEC: {
            static char buf[32];
            sprintf(buf, "decimal[%d.%d]", t->info.dec.a, t->info.dec.b);
            return buf;
        }
        case TY_TXT: {
            static char buf[32];
            sprintf(buf, "texto[%zu]", t->info.txt.n);
            return buf;
        }
        case TY_BOOL:
            return "bool";
        default:
            return "?";
    }
}

/* Imprime todo o conteúdo da tabela de símbolos, preservando a ordem de
   criação de escopos. A função também reporta o pico de memória observado
   pelo gerenciador. */
void symtab_dump(SymTab *st) {
    if (!st) return;

    size_t count = 0;
    {
        Scope *s;
        for (s = st->current; s; s = s->parent) count++;
    }
    Scope **order = (Scope**)mm_malloc(sizeof(Scope*) * count);
    mm_usage_guard();
    {
        size_t i = count;
        Scope *s;
        for (s = st->current; s; s = s->parent) {
            order[--i] = s;
        }
    }

    {
        size_t i;
        for (i = 0; i < count; ++i) {
            Scope *sc = order[i];
            printf("Escopo %zu:\n", sc->id);
            {
                size_t b;
                for (b = 0; b < sc->bucket_count; ++b) {
                    Symbol *sym;
                    for (sym = sc->buckets[b]; sym; sym = sym->next) {
                        printf("  %s (%s, %s, linha %d)\n",
                               sym->name,
                               class_str(sym->sclass),
                               type_str(sym->type),
                               sym->line_decl);
                    }
                }
            }
            printf("\n");
        }
    }
    mm_free(order);

    size_t peak = mm_peak_usage();
    printf("Pico de memória: %zu bytes (inteiro=4B, decimal=8B, texto[n]=nB)\n", peak);
}

