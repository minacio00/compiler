#include "semantics.h"
#include "memmgr.h"
#include <stdio.h>
#include <string.h>

/* Cria um tipo básico com metadados zerados */
static Type make_type(TypeKind kind) {
    Type t;
    t.kind = kind;
    if (kind == TY_DEC) {
        t.info.dec.a = 0;
        t.info.dec.b = 0;
    } else if (kind == TY_TXT) {
        t.info.txt.n = 0;
    } else {
        memset(&t.info, 0, sizeof(t.info));
    }
    return t;
}

/* Infere o tipo de uma expressão e anota no nó */
static Type resolve_expr_type(SemaContext *sc, ASTNode *node) {
    Type t = make_type(TY_INT);
    if (!node) return t;

    switch (node->type) {
        case AST_LITERAL:
            switch (node->token.type) {
                case TOK_INTEGER_LITERAL:
                    t = make_type(TY_INT);
                    break;
                case TOK_DECIMAL_LITERAL:
                    t = make_type(TY_DEC);
                    if (node->token.lexeme) {
                        const char *dot = strchr(node->token.lexeme, '.');
                        if (dot) {
                            t.info.dec.a = (int)(dot - node->token.lexeme);
                            t.info.dec.b = (int)strlen(dot + 1);
                        } else {
                            t.info.dec.a = (int)strlen(node->token.lexeme);
                            t.info.dec.b = 0;
                        }
                    }
                    break;
                case TOK_STRING_LITERAL:
                    t = make_type(TY_TXT);
                    if (node->token.lexeme) {
                        size_t len = strlen(node->token.lexeme);
                        if (len >= 2) len -= 2; /* remove aspas */
                        t.info.txt.n = len;
                    }
                    break;
                default:
                    break;
            }
            break;

        case AST_IDENTIFIER: {
            Symbol *sym = symtab_lookup(sc->symtab, node->token.lexeme);
            if (sym && sym->type) t = *sym->type;
            break;
        }

        case AST_BINARY_OP: {
            Type left = resolve_expr_type(sc, node->children[0]);
            Type right = resolve_expr_type(sc, node->children[1]);
            switch (node->token.type) {
                case TOK_PLUS:
                case TOK_MINUS:
                case TOK_STAR:
                case TOK_SLASH:
                case TOK_MODULO:
                case TOK_CARET:
                    if (left.kind == TY_DEC || right.kind == TY_DEC)
                        t = make_type(TY_DEC);
                    else
                        t = make_type(TY_INT);
                    break;
                case TOK_EQ:
                case TOK_NEQ:
                case TOK_LT:
                case TOK_GT:
                case TOK_LE:
                case TOK_GE:
                    t = make_type(TY_BOOL);
                    break;
                case TOK_AND:
                case TOK_OR:
                    t = make_type(TY_BOOL);
                    break;
                default:
                    t = make_type(TY_INT);
                    break;
            }
            break;
        }

        case AST_UNARY_OP:
            if (node->child_count > 0)
                t = resolve_expr_type(sc, node->children[0]);
            break;

        case AST_EXPRESSION:
            if (node->child_count > 0)
                t = resolve_expr_type(sc, node->children[0]);
            break;

        default:
            break;
    }

    node->inferred_type = t;
    return t;
}

/* Analisa os nós da AST e resolve tipos */
static void analyze_node(SemaContext *sc, ASTNode *node) {
    if (!node) return;

    switch (node->type) {
        case AST_DECLARATION: {
            TypeKind kind = TY_INT;
            switch (node->token.type) {
                case TOK_KW_DECIMAL: kind = TY_DEC; break;
                case TOK_KW_TEXTO:   kind = TY_TXT; break;
                case TOK_KW_INTEIRO: default: kind = TY_INT; break;
            }

            int i;
            for (i = 0; i < node->child_count; i++) {
                ASTNode *child = node->children[i];
                if (child->type == AST_IDENTIFIER && child->token.lexeme) {
                    Type *t = (Type*)mm_malloc(sizeof(Type));
                    if (t) *t = make_type(kind);
                    Symbol s = {0};
                    s.name = child->token.lexeme;
                    s.sclass = SYM_VAR;
                    s.type = t;
                    s.line_decl = child->token.line;
                    s.extra = NULL;
                    symtab_insert(sc->symtab, &s);
                } else {
                    analyze_node(sc, child);
                }
            }
            return;
        }

        case AST_BINARY_OP:
        case AST_LITERAL:
        case AST_IDENTIFIER:
        case AST_UNARY_OP:
        case AST_EXPRESSION:
            resolve_expr_type(sc, node);
            return;

        default:
            break;
    }

    int i;
    for (i = 0; i < node->child_count; i++) {
        analyze_node(sc, node->children[i]);
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

