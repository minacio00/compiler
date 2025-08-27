#include "semantics.h"
#include "memmgr.h"
#include "sema_report.h"
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
static void analyze_node(SemaContext *sc, ASTNode *node, ASTNodeType parent) {
    if (!node) return;

    switch (node->type) {
        case AST_DECLARATION: {
            if (parent == AST_READ_STMT || parent == AST_WRITE_STMT ||
                parent == AST_IF_STMT   || parent == AST_FOR_STMT) {
                sema_report_alert("declaração fora de escopo permitido", node->token.line);
            }

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
                    if (!symtab_insert(sc->symtab, &s)) {
                        sema_report_alert("símbolo redeclarado", child->token.line);
                    }
                } else {
                    analyze_node(sc, child, node->type);
                }
            }
            return;
        }

        case AST_ASSIGNMENT: {
            if (node->child_count >= 2) {
                ASTNode *lhs = node->children[0];
                ASTNode *rhs = node->children[1];
                Symbol *sym = symtab_lookup(sc->symtab, lhs->token.lexeme);
                if (!sym) {
                    sema_report_alert("variável não declarada", lhs->token.line);
                } else {
                    Type rt = resolve_expr_type(sc, rhs);
                    if (sym->type && sym->type->kind != rt.kind) {
                        sema_report_alert("atribuição com tipos incompatíveis", lhs->token.line);
                    }
                }
            }
            break;
        }

        case AST_READ_STMT: {
            if (node->child_count > 0) {
                ASTNode *id = node->children[0];
                if (!symtab_lookup(sc->symtab, id->token.lexeme)) {
                    sema_report_alert("variável não declarada em 'leia'", id->token.line);
                }
            }
            break;
        }

        case AST_WRITE_STMT: {
            int i;
            for (i = 0; i < node->child_count; i++) {
                resolve_expr_type(sc, node->children[i]);
            }
            break;
        }

        case AST_IF_STMT: {
            if (node->child_count > 0) {
                Type cond = resolve_expr_type(sc, node->children[0]);
                if (cond.kind != TY_BOOL) {
                    sema_report_alert("condição do 'se' deve ser booleana", node->children[0]->token.line);
                }
            }
            int i;
            for (i = 1; i < node->child_count; i++) {
                analyze_node(sc, node->children[i], node->type);
            }
            return;
        }

        case AST_FOR_STMT: {
            if (node->child_count >= 4) {
                analyze_node(sc, node->children[0], node->type);
                Type cond = resolve_expr_type(sc, node->children[1]);
                if (cond.kind != TY_BOOL) {
                    sema_report_alert("condição do 'para' deve ser booleana", node->children[1]->token.line);
                }
                analyze_node(sc, node->children[2], node->type);
                analyze_node(sc, node->children[3], node->type);
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
        analyze_node(sc, node->children[i], node->type);
    }
}

static void check_returns(SemaContext *sc, ASTNode *node, TypeKind *ret_kind, int *ret_line, bool *has_ret) {
    if (!node) return;
    if (node->type == AST_RETURN_STMT) {
        *has_ret = true;
        Type t = make_type(TY_INT);
        if (node->child_count > 0) t = resolve_expr_type(sc, node->children[0]);
        if (*ret_line == -1) {
            *ret_kind = t.kind;
            *ret_line = node->token.line;
        } else if (*ret_kind != t.kind) {
            sema_report_alert("tipos de retorno inconsistentes", node->token.line);
        }
        return;
    }
    int i;
    for (i = 0; i < node->child_count; i++) {
        check_returns(sc, node->children[i], ret_kind, ret_line, has_ret);
    }
}

static void analyze_function(SemaContext *sc, ASTNode *func) {
    if (!func || func->child_count == 0) return;

    symtab_enter_scope(sc->symtab);

    int i;
    for (i = 1; i < func->child_count - 1; i++) {
        ASTNode *param = func->children[i];
        if (param->type != AST_DECLARATION) continue;
        TypeKind kind = TY_INT;
        switch (param->token.type) {
            case TOK_KW_DECIMAL: kind = TY_DEC; break;
            case TOK_KW_TEXTO:   kind = TY_TXT; break;
            case TOK_KW_INTEIRO: default: kind = TY_INT; break;
        }
        int j;
        for (j = 0; j < param->child_count; j++) {
            ASTNode *id = param->children[j];
            if (id->type != AST_IDENTIFIER) continue;
            Type *t = (Type*)mm_malloc(sizeof(Type));
            if (t) *t = make_type(kind);
            Symbol s = {0};
            s.name = id->token.lexeme;
            s.sclass = SYM_PARAM;
            s.type = t;
            s.line_decl = id->token.line;
            s.extra = NULL;
            if (!symtab_insert(sc->symtab, &s)) {
                sema_report_alert("parâmetro redeclarado", id->token.line);
            }
        }
    }

    ASTNode *body = func->children[func->child_count - 1];
    analyze_node(sc, body, func->type);

    TypeKind rk = TY_INT;
    int rline = -1;
    bool has_ret = false;
    check_returns(sc, body, &rk, &rline, &has_ret);
    if (!has_ret) {
        sema_report_alert("função sem retorno", func->children[0]->token.line);
    }

    symtab_leave_scope(sc->symtab);
}

static int build_function_index(SemaContext *sc, ASTNode *program, ASTNode **funcs) {
    int count = 0;
    int principal_count = 0;
    int i;
    for (i = 0; i < program->child_count; i++) {
        ASTNode *child = program->children[i];
        if (child->type == AST_FUNCTION_DEF && child->child_count > 0) {
            ASTNode *name = child->children[0];
            const char *fname = name->token.lexeme;
            if (strcmp(fname, "principal") == 0) {
                principal_count++;
                if (child->child_count > 2) {
                    sema_report_alert("principal() não deve ter parâmetros", name->token.line);
                }
            } else {
                if (!fname || strncmp(fname, "__", 2) != 0) {
                    sema_report_alert("nome de função inválido", name->token.line);
                }
            }
            Type *t = (Type*)mm_malloc(sizeof(Type));
            if (t) *t = make_type(TY_INT);
            Symbol s = {0};
            s.name = (char*)fname;
            s.sclass = SYM_FUNC;
            s.type = t;
            s.line_decl = name->token.line;
            s.extra = child;
            if (!symtab_insert(sc->symtab, &s)) {
                sema_report_alert("função redeclarada", name->token.line);
            }
            funcs[count++] = child;
        } else if (child->type == AST_DECLARATION) {
            analyze_node(sc, child, AST_PROGRAM);
        } else {
            analyze_node(sc, child, AST_PROGRAM);
        }
    }
    if (principal_count != 1) {
        sema_report_alert("deve existir exatamente uma função principal()", program->token.line);
    }
    return count;
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
    if (ast->type != AST_PROGRAM) {
        analyze_node(sc, ast, AST_PROGRAM);
        return true;
    }

    ASTNode **funcs = (ASTNode**)mm_malloc(sizeof(ASTNode*) * ast->child_count);
    int count = build_function_index(sc, ast, funcs);
    int i;
    for (i = 0; i < count; i++) {
        analyze_function(sc, funcs[i]);
    }
    mm_free(funcs);
    return true;
}

void sema_destroy(SemaContext* sc) {
    if (!sc) return;
    symtab_destroy(sc->symtab);
    mm_free(sc);
}

