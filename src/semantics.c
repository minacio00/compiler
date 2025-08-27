#include "semantics.h"
#include "memmgr.h"
#include <stdio.h>

static void analyze_node(SemaContext *sc, ASTNode *node, Scope *scope) {
    int i;
    if (!node) return;
    switch (node->type) {
        case AST_DECLARATION:
            if (node->child_count > 0) {
                ASTNode *id = node->children[0];
                if (id && id->type == AST_IDENTIFIER && id->token.lexeme) {
                    symtab_add(scope, id->token.lexeme);
                }
            }
            break;
        default:
            break;
    }
    for (i = 0; i < node->child_count; i++) {
        analyze_node(sc, node->children[i], scope);
    }
}

SemaContext* sema_create(size_t mem_limit_bytes) {
    SemaContext *sc = (SemaContext*)mm_malloc(sizeof(SemaContext));
    if (!sc) return NULL;
    sc->mem_limit = mem_limit_bytes;
    sc->global_scope = scope_create(NULL);
    if (!sc->global_scope) {
        mm_free(sc);
        return NULL;
    }
    return sc;
}

bool semantic_analyze(SemaContext* sc, ASTNode* ast) {
    if (!sc || !ast) return false;
    analyze_node(sc, ast, sc->global_scope);
    return true;
}

void symtab_print(SemaContext* sc) {
    if (!sc) return;
    printf("\033[34m=== TABELA DE SÍMBOLOS ===\033[0m\n");
    symtab_print_scope(sc->global_scope, 0);
}

void sema_destroy(SemaContext* sc) {
    if (!sc) return;
    scope_destroy(sc->global_scope);
    mm_free(sc);
}

