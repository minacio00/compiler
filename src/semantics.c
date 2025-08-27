#include "semantics.h"
#include "memmgr.h"
#include <stdio.h>

static void analyze_node(SemaContext *sc, ASTNode *node) {
    if (!node) return;

    switch (node->type) {
        case AST_DECLARATION:
            if (node->child_count > 0) {
                ASTNode *id = node->children[0];
                if (id && id->type == AST_IDENTIFIER && id->token.lexeme) {
                    Symbol s = {0};
                    s.name = id->token.lexeme;
                    s.sclass = SYM_VAR;
                    s.type = NULL;
                    s.line_decl = id->token.line;
                    s.extra = NULL;
                    symtab_insert(sc->symtab, &s);
                }
            }
            break;
        default:
            break;
    }

    {
        int i;
        for (i = 0; i < node->child_count; i++) {
            analyze_node(sc, node->children[i]);
        }
    }
}

SemaContext* sema_create(size_t mem_limit_bytes) {
    SemaContext *sc = (SemaContext*)mm_malloc(sizeof(SemaContext));
    if (!sc) return NULL;
    sc->mem_limit = mem_limit_bytes;
    sc->symtab = symtab_create();
    if (!sc->symtab) {
        mm_free(sc);
        return NULL;
    }
    return sc;
}

bool semantic_analyze(SemaContext* sc, ASTNode* ast) {
    if (!sc || !ast) return false;
    analyze_node(sc, ast);
    return true;
}

void sema_destroy(SemaContext* sc) {
    if (!sc) return;
    symtab_destroy(sc->symtab);
    mm_free(sc);
}

