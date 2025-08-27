#include "parser.h"
#include "lexer.h"
#include "memmgr.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ========== Declarações antecipadas das funções de parsing ========== */

ASTNode* parse_program(Parser *parser);
ASTNode* parse_declaration(Parser *parser);
ASTNode* parse_statement(Parser *parser);
ASTNode* parse_assignment(Parser *parser);
ASTNode* parse_expression(Parser *parser);
ASTNode* parse_logical_or(Parser *parser);
ASTNode* parse_logical_and(Parser *parser);
ASTNode* parse_equality(Parser *parser);
ASTNode* parse_comparison(Parser *parser);
ASTNode* parse_term(Parser *parser);
ASTNode* parse_factor(Parser *parser);
ASTNode* parse_unary(Parser *parser);
ASTNode* parse_primary(Parser *parser);
ASTNode* parse_if_statement(Parser *parser);
ASTNode* parse_while_statement(Parser *parser);
ASTNode* parse_for_statement(Parser *parser);
ASTNode* parse_read_statement(Parser *parser);
ASTNode* parse_write_statement(Parser *parser);
ASTNode* parse_block(Parser *parser);
ASTNode* parse_function_definition(Parser *parser);

/* Variável global para o parser atual */
static Parser *current_parser = NULL;

/* ========== Funções de criação e gerenciamento do AST ========== */

ASTNode* ast_node_create(ASTNodeType type, Token token) {
    ASTNode *node = mm_malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Erro: não foi possível alocar memória para nó AST\n");
        exit(EXIT_FAILURE);
    }
    
    node->type = type;
    node->token = token;
    node->children = NULL;
    node->child_count = 0;
    node->child_capacity = 0;
    node->value = NULL;
    node->inferred_type.kind = TY_INT;
    node->inferred_type.info.dec.a = 0;
    node->inferred_type.info.dec.b = 0;
    
    return node;
}

void ast_node_add_child(ASTNode *parent, ASTNode *child) {
    if (!parent || !child) return;
    
    if (parent->child_count >= parent->child_capacity) {
        int new_capacity = parent->child_capacity == 0 ? 4 : parent->child_capacity * 2;
        ASTNode **new_children = mm_realloc(parent->children, 
                                           new_capacity * sizeof(ASTNode*));
        if (!new_children) {
            fprintf(stderr, "Erro: não foi possível realocar memória para filhos do AST\n");
            exit(EXIT_FAILURE);
        }
        parent->children = new_children;
        parent->child_capacity = new_capacity;
    }
    
    parent->children[parent->child_count++] = child;
}

void free_ast(ASTNode *node) {
    if (!node) return;
    
    int i;
    for (i = 0; i < node->child_count; i++) {
        free_ast(node->children[i]);
    }
    
    if (node->children) mm_free(node->children);
    if (node->value) mm_free(node->value);
    mm_free(node);
}

void print_ast(ASTNode *node, int depth) {
    if (!node) return;
    
    int i;
    /* Indentação */
    for (i = 0; i < depth; i++) {
        printf("  ");
    }
    
    /* Imprimir tipo do nó */
    const char *type_names[] = {
        "PROGRAM", "DECLARATION", "ASSIGNMENT", "EXPRESSION", "IF_STMT",
        "WHILE_STMT", "FOR_STMT", "READ_STMT", "WRITE_STMT", "BLOCK",
        "BINARY_OP", "UNARY_OP", "LITERAL", "IDENTIFIER", "FUNCTION_DEF",
        "FUNCTION_CALL", "RETURN_STMT"
    };
    
    printf("%s", type_names[node->type]);
    if (node->token.lexeme && strlen(node->token.lexeme) > 0) {
        printf(" '%s'", node->token.lexeme);
    }
    if (node->value) {
        printf(" (%s)", node->value);
    }
    printf("\n");
    
    /* Imprimir filhos */
    for (i = 0; i < node->child_count; i++) {
        print_ast(node->children[i], depth + 1);
    }
}

/* ========== Funções de controle do parser ========== */

Parser* parser_init(void) {
    Parser *parser = mm_malloc(sizeof(Parser));
    if (!parser) {
        fprintf(stderr, "Erro: não foi possível alocar memória para parser\n");
        exit(EXIT_FAILURE);
    }
    
    parser->current_token = next_token();
    parser->previous_token = (Token){.type = TOK_EOF, .lexeme = "", .line = 0};
    parser->had_error = 0;
    parser->panic_mode = 0;
    
    current_parser = parser;
    return parser;
}

void parser_free(Parser *parser) {
    if (parser) {
        mm_free(parser);
    }
    current_parser = NULL;
}

void advance_token(Parser *parser) {
    parser->previous_token = parser->current_token;
    parser->current_token = next_token();
}

int check_token(Parser *parser, TokenType type) {
    return parser->current_token.type == type;
}

int match_token(Parser *parser, TokenType type) {
    if (check_token(parser, type)) {
        advance_token(parser);
        return 1;
    }
    return 0;
}

void parser_error(Parser *parser, const char *message) {
    if (parser->panic_mode) return;
    
    parser->panic_mode = 1;
    parser->had_error = 1;
    
    fprintf(stderr, "\033[31mErro sintático na linha %d: %s\033[0m\n", 
            parser->current_token.line, message);
    
    if (parser->current_token.type != TOK_EOF) {
        fprintf(stderr, "Token atual: '%s' (%s)\n", 
                parser->current_token.lexeme,
                token_type_name(parser->current_token.type));
    }
}

void synchronize(Parser *parser) {
    parser->panic_mode = 0;
    
    while (parser->current_token.type != TOK_EOF) {
        if (parser->previous_token.type == TOK_SEMICOLON) return;
        
        switch (parser->current_token.type) {
            case TOK_KW_INTEIRO:
            case TOK_KW_DECIMAL:
            case TOK_KW_TEXTO:
            case TOK_KW_SE:
            case TOK_KW_ENQUANTO:
            case TOK_KW_PARA:
            case TOK_KW_LEIA:
            case TOK_KW_ESCREVA:
            case TOK_KW_FUNCAO:
            case TOK_KW_RETORNE:
                return;
            default:
                break;
        }
        
        advance_token(parser);
    }
}

/* ========== Funções de parsing específicas ========== */

ASTNode* parse_program(Parser *parser) {
    ASTNode *program = ast_node_create(AST_PROGRAM, parser->current_token);
    
    /* Verificar se começa com 'principal' ou função */
    if (check_token(parser, TOK_KW_PRINCIPAL)) {
        advance_token(parser);
        
        if (!match_token(parser, TOK_LPAREN)) {
            parser_error(parser, "Esperado '(' após 'principal'");
            return program;
        }
        
        if (!match_token(parser, TOK_RPAREN)) {
            parser_error(parser, "Esperado ')' após '('");
            return program;
        }
        
        if (!match_token(parser, TOK_LBRACE)) {
            parser_error(parser, "Esperado '{' após 'principal()'");
            return program;
        }
        
        /* Parse do corpo do programa principal */
        while (!check_token(parser, TOK_RBRACE) && !check_token(parser, TOK_EOF)) {
            ASTNode *stmt = parse_statement(parser);
            if (stmt) {
                ast_node_add_child(program, stmt);
            }
            
            if (parser->panic_mode) {
                synchronize(parser);
            }
        }
        
        if (!match_token(parser, TOK_RBRACE)) {
            parser_error(parser, "Esperado '}' para fechar programa principal");
        }
    } else {
        /* Parse de funções e declarações globais */
        while (!check_token(parser, TOK_EOF)) {
            if (check_token(parser, TOK_KW_FUNCAO)) {
                ASTNode *func = parse_function_definition(parser);
                if (func) {
                    ast_node_add_child(program, func);
                }
            } else {
                ASTNode *stmt = parse_statement(parser);
                if (stmt) {
                    ast_node_add_child(program, stmt);
                }
            }
            
            if (parser->panic_mode) {
                synchronize(parser);
            }
        }
    }
    
    return program;
}

ASTNode* parse_declaration(Parser *parser) {
    ASTNode *decl = ast_node_create(AST_DECLARATION, parser->current_token);
    
    /* Tipo da variável */
    TokenType type = parser->current_token.type;
    if (type != TOK_KW_INTEIRO && type != TOK_KW_DECIMAL && type != TOK_KW_TEXTO) {
        parser_error(parser, "Esperado tipo de variável (inteiro, decimal, texto)");
        return decl;
    }
    advance_token(parser);
    
    /* Primeira variável */
    if (!check_token(parser, TOK_IDENTIFIER)) {
        parser_error(parser, "Esperado nome de variável");
        return decl;
    }
    
    ASTNode *var = ast_node_create(AST_IDENTIFIER, parser->current_token);
    ast_node_add_child(decl, var);
    advance_token(parser);
    
    /* Possível inicialização ou array */
    if (match_token(parser, TOK_ASSIGN)) {
        ASTNode *expr = parse_expression(parser);
        if (expr) {
            ast_node_add_child(decl, expr);
        }
    } else if (match_token(parser, TOK_LBRACKET)) {
        ASTNode *size = parse_expression(parser);
        if (size) {
            ast_node_add_child(decl, size);
        }
        
        if (!match_token(parser, TOK_RBRACKET)) {
            parser_error(parser, "Esperado ']' após tamanho do array");
        }
    }
    
    /* Variáveis adicionais separadas por vírgula */
    while (match_token(parser, TOK_COMMA)) {
        if (!check_token(parser, TOK_IDENTIFIER)) {
            parser_error(parser, "Esperado nome de variável após vírgula");
            return decl;
        }
        
        ASTNode *additional_var = ast_node_create(AST_IDENTIFIER, parser->current_token);
        ast_node_add_child(decl, additional_var);
        advance_token(parser);
        
        /* Possível inicialização ou array para variável adicional */
        if (match_token(parser, TOK_ASSIGN)) {
            ASTNode *expr = parse_expression(parser);
            if (expr) {
                ast_node_add_child(decl, expr);
            }
        } else if (match_token(parser, TOK_LBRACKET)) {
            ASTNode *size = parse_expression(parser);
            if (size) {
                ast_node_add_child(decl, size);
            }
            
            if (!match_token(parser, TOK_RBRACKET)) {
                parser_error(parser, "Esperado ']' após tamanho do array");
            }
        }
    }
    
    return decl;
}

ASTNode* parse_statement(Parser *parser) {
    switch (parser->current_token.type) {
        case TOK_KW_INTEIRO:
        case TOK_KW_DECIMAL:
        case TOK_KW_TEXTO: {
            ASTNode *decl = parse_declaration(parser);
            if (!match_token(parser, TOK_SEMICOLON)) {
                parser_error(parser, "Esperado ';' após declaração");
            }
            return decl;
        }
        
        case TOK_IDENTIFIER: {
            ASTNode *assign = parse_assignment(parser);
            if (!match_token(parser, TOK_SEMICOLON)) {
                parser_error(parser, "Esperado ';' após atribuição");
            }
            return assign;
        }
        
        case TOK_KW_SE:
            return parse_if_statement(parser);
        
        case TOK_KW_ENQUANTO:
            return parse_while_statement(parser);
        
        case TOK_KW_PARA:
            return parse_for_statement(parser);
        
        case TOK_KW_LEIA:
            return parse_read_statement(parser);
        
        case TOK_KW_ESCREVA:
            return parse_write_statement(parser);
        
        case TOK_KW_RETORNE: {
            ASTNode *ret = ast_node_create(AST_RETURN_STMT, parser->current_token);
            advance_token(parser);
            
            if (!check_token(parser, TOK_SEMICOLON)) {
                ASTNode *expr = parse_expression(parser);
                if (expr) {
                    ast_node_add_child(ret, expr);
                }
            }
            
            if (!match_token(parser, TOK_SEMICOLON)) {
                parser_error(parser, "Esperado ';' após retorno");
            }
            return ret;
        }
        
        case TOK_LBRACE:
            return parse_block(parser);
        
        default:
            parser_error(parser, "Comando não reconhecido");
            advance_token(parser);
            return NULL;
    }
}

ASTNode* parse_assignment(Parser *parser) {
    ASTNode *assign = ast_node_create(AST_ASSIGNMENT, parser->current_token);
    
    /* Variável à esquerda */
    ASTNode *var = ast_node_create(AST_IDENTIFIER, parser->current_token);
    ast_node_add_child(assign, var);
    advance_token(parser);
    
    if (!match_token(parser, TOK_ASSIGN)) {
        parser_error(parser, "Esperado '=' em atribuição");
        return assign;
    }
    
    /* Expressão à direita */
    ASTNode *expr = parse_expression(parser);
    if (expr) {
        ast_node_add_child(assign, expr);
    }
    
    return assign;
}

ASTNode* parse_expression(Parser *parser) {
    return parse_logical_or(parser);
}

ASTNode* parse_logical_or(Parser *parser) {
    ASTNode *expr = parse_logical_and(parser);
    
    while (match_token(parser, TOK_OR)) {
        Token operator = parser->previous_token;
        ASTNode *right = parse_logical_and(parser);
        
        ASTNode *binary = ast_node_create(AST_BINARY_OP, operator);
        ast_node_add_child(binary, expr);
        ast_node_add_child(binary, right);
        expr = binary;
    }
    
    return expr;
}

ASTNode* parse_logical_and(Parser *parser) {
    ASTNode *expr = parse_equality(parser);
    
    while (match_token(parser, TOK_AND)) {
        Token operator = parser->previous_token;
        ASTNode *right = parse_equality(parser);
        
        ASTNode *binary = ast_node_create(AST_BINARY_OP, operator);
        ast_node_add_child(binary, expr);
        ast_node_add_child(binary, right);
        expr = binary;
    }
    
    return expr;
}

ASTNode* parse_equality(Parser *parser) {
    ASTNode *expr = parse_comparison(parser);
    
    while (match_token(parser, TOK_EQ) || match_token(parser, TOK_NEQ)) {
        Token operator = parser->previous_token;
        ASTNode *right = parse_comparison(parser);
        
        ASTNode *binary = ast_node_create(AST_BINARY_OP, operator);
        ast_node_add_child(binary, expr);
        ast_node_add_child(binary, right);
        expr = binary;
    }
    
    return expr;
}

ASTNode* parse_comparison(Parser *parser) {
    ASTNode *expr = parse_term(parser);
    
    while (match_token(parser, TOK_GT) || match_token(parser, TOK_GE) ||
           match_token(parser, TOK_LT) || match_token(parser, TOK_LE)) {
        Token operator = parser->previous_token;
        ASTNode *right = parse_term(parser);
        
        ASTNode *binary = ast_node_create(AST_BINARY_OP, operator);
        ast_node_add_child(binary, expr);
        ast_node_add_child(binary, right);
        expr = binary;
    }
    
    return expr;
}

ASTNode* parse_term(Parser *parser) {
    ASTNode *expr = parse_factor(parser);
    
    while (match_token(parser, TOK_MINUS) || match_token(parser, TOK_PLUS)) {
        Token operator = parser->previous_token;
        ASTNode *right = parse_factor(parser);
        
        ASTNode *binary = ast_node_create(AST_BINARY_OP, operator);
        ast_node_add_child(binary, expr);
        ast_node_add_child(binary, right);
        expr = binary;
    }
    
    return expr;
}

ASTNode* parse_factor(Parser *parser) {
    ASTNode *expr = parse_unary(parser);
    
    while (match_token(parser, TOK_SLASH) || match_token(parser, TOK_STAR) ||
           match_token(parser, TOK_MODULO) || match_token(parser, TOK_CARET)) {
        Token operator = parser->previous_token;
        ASTNode *right = parse_unary(parser);
        
        ASTNode *binary = ast_node_create(AST_BINARY_OP, operator);
        ast_node_add_child(binary, expr);
        ast_node_add_child(binary, right);
        expr = binary;
    }
    
    return expr;
}

ASTNode* parse_unary(Parser *parser) {
    if (match_token(parser, TOK_MINUS) || match_token(parser, TOK_PLUS)) {
        Token operator = parser->previous_token;
        ASTNode *right = parse_unary(parser);
        
        ASTNode *unary = ast_node_create(AST_UNARY_OP, operator);
        ast_node_add_child(unary, right);
        return unary;
    }
    
    return parse_primary(parser);
}

ASTNode* parse_primary(Parser *parser) {
    if (match_token(parser, TOK_INTEGER_LITERAL) ||
        match_token(parser, TOK_DECIMAL_LITERAL) ||
        match_token(parser, TOK_STRING_LITERAL)) {
        return ast_node_create(AST_LITERAL, parser->previous_token);
    }
    
    if (match_token(parser, TOK_IDENTIFIER)) {
        ASTNode *id = ast_node_create(AST_IDENTIFIER, parser->previous_token);
        
        /* Verificar se é chamada de função */
        if (match_token(parser, TOK_LPAREN)) {
            ASTNode *call = ast_node_create(AST_FUNCTION_CALL, parser->previous_token);
            ast_node_add_child(call, id);
            
            /* Parse argumentos */
            if (!check_token(parser, TOK_RPAREN)) {
                do {
                    ASTNode *arg = parse_expression(parser);
                    if (arg) {
                        ast_node_add_child(call, arg);
                    }
                } while (match_token(parser, TOK_COMMA));
            }
            
            if (!match_token(parser, TOK_RPAREN)) {
                parser_error(parser, "Esperado ')' após argumentos da função");
            }
            
            return call;
        }
        
        return id;
    }
    
    if (match_token(parser, TOK_LPAREN)) {
        ASTNode *expr = parse_expression(parser);
        if (!match_token(parser, TOK_RPAREN)) {
            parser_error(parser, "Esperado ')' após expressão");
        }
        return expr;
    }
    
    parser_error(parser, "Esperado expressão");
    return NULL;
}

ASTNode* parse_if_statement(Parser *parser) {
    ASTNode *if_stmt = ast_node_create(AST_IF_STMT, parser->current_token);
    advance_token(parser); /* consume 'se' */
    
    if (!match_token(parser, TOK_LPAREN)) {
        parser_error(parser, "Esperado '(' após 'se'");
        return if_stmt;
    }
    
    ASTNode *condition = parse_expression(parser);
    if (condition) {
        ast_node_add_child(if_stmt, condition);
    }
    
    if (!match_token(parser, TOK_RPAREN)) {
        parser_error(parser, "Esperado ')' após condição do 'se'");
        return if_stmt;
    }
    
    ASTNode *then_stmt = parse_statement(parser);
    if (then_stmt) {
        ast_node_add_child(if_stmt, then_stmt);
    }
    
    if (match_token(parser, TOK_KW_SENAO)) {
        ASTNode *else_stmt = parse_statement(parser);
        if (else_stmt) {
            ast_node_add_child(if_stmt, else_stmt);
        }
    }
    
    return if_stmt;
}

ASTNode* parse_while_statement(Parser *parser) {
    ASTNode *while_stmt = ast_node_create(AST_WHILE_STMT, parser->current_token);
    advance_token(parser); /* consume 'enquanto' */
    
    if (!match_token(parser, TOK_LPAREN)) {
        parser_error(parser, "Esperado '(' após 'enquanto'");
        return while_stmt;
    }
    
    ASTNode *condition = parse_expression(parser);
    if (condition) {
        ast_node_add_child(while_stmt, condition);
    }
    
    if (!match_token(parser, TOK_RPAREN)) {
        parser_error(parser, "Esperado ')' após condição do 'enquanto'");
        return while_stmt;
    }
    
    ASTNode *body = parse_statement(parser);
    if (body) {
        ast_node_add_child(while_stmt, body);
    }
    
    return while_stmt;
}

ASTNode* parse_for_statement(Parser *parser) {
    ASTNode *for_stmt = ast_node_create(AST_FOR_STMT, parser->current_token);
    advance_token(parser); /* consume 'para' */
    
    if (!match_token(parser, TOK_LPAREN)) {
        parser_error(parser, "Esperado '(' após 'para'");
        return for_stmt;
    }
    
    /* Inicialização */
    ASTNode *init = parse_assignment(parser);
    if (init) {
        ast_node_add_child(for_stmt, init);
    }
    
    if (!match_token(parser, TOK_SEMICOLON)) {
        parser_error(parser, "Esperado ';' após inicialização do 'para'");
        return for_stmt;
    }
    
    /* Condição */
    ASTNode *condition = parse_expression(parser);
    if (condition) {
        ast_node_add_child(for_stmt, condition);
    }
    
    if (!match_token(parser, TOK_SEMICOLON)) {
        parser_error(parser, "Esperado ';' após condição do 'para'");
        return for_stmt;
    }
    
    /* Incremento */
    ASTNode *increment = parse_assignment(parser);
    if (increment) {
        ast_node_add_child(for_stmt, increment);
    }
    
    if (!match_token(parser, TOK_RPAREN)) {
        parser_error(parser, "Esperado ')' após incremento do 'para'");
        return for_stmt;
    }
    
    ASTNode *body = parse_statement(parser);
    if (body) {
        ast_node_add_child(for_stmt, body);
    }
    
    return for_stmt;
}

ASTNode* parse_read_statement(Parser *parser) {
    ASTNode *read_stmt = ast_node_create(AST_READ_STMT, parser->current_token);
    advance_token(parser); /* consume 'leia' */
    
    if (!match_token(parser, TOK_LPAREN)) {
        parser_error(parser, "Esperado '(' após 'leia'");
        return read_stmt;
    }
    
    if (!check_token(parser, TOK_IDENTIFIER)) {
        parser_error(parser, "Esperado variável em 'leia'");
        return read_stmt;
    }
    
    ASTNode *var = ast_node_create(AST_IDENTIFIER, parser->current_token);
    ast_node_add_child(read_stmt, var);
    advance_token(parser);
    
    if (!match_token(parser, TOK_RPAREN)) {
        parser_error(parser, "Esperado ')' após variável em 'leia'");
        return read_stmt;
    }
    
    if (!match_token(parser, TOK_SEMICOLON)) {
        parser_error(parser, "Esperado ';' após 'leia'");
    }
    
    return read_stmt;
}

ASTNode* parse_write_statement(Parser *parser) {
    ASTNode *write_stmt = ast_node_create(AST_WRITE_STMT, parser->current_token);
    advance_token(parser); /* consume 'escreva' */
    
    if (!match_token(parser, TOK_LPAREN)) {
        parser_error(parser, "Esperado '(' após 'escreva'");
        return write_stmt;
    }
    
    /* Parse argumentos de escrita */
    if (!check_token(parser, TOK_RPAREN)) {
        do {
            ASTNode *arg = parse_expression(parser);
            if (arg) {
                ast_node_add_child(write_stmt, arg);
            }
        } while (match_token(parser, TOK_COMMA));
    }
    
    if (!match_token(parser, TOK_RPAREN)) {
        parser_error(parser, "Esperado ')' após argumentos de 'escreva'");
        return write_stmt;
    }
    
    if (!match_token(parser, TOK_SEMICOLON)) {
        parser_error(parser, "Esperado ';' após 'escreva'");
    }
    
    return write_stmt;
}

ASTNode* parse_block(Parser *parser) {
    ASTNode *block = ast_node_create(AST_BLOCK, parser->current_token);
    advance_token(parser); /* consume '{' */
    
    while (!check_token(parser, TOK_RBRACE) && !check_token(parser, TOK_EOF)) {
        ASTNode *stmt = parse_statement(parser);
        if (stmt) {
            ast_node_add_child(block, stmt);
        }
        
        if (parser->panic_mode) {
            synchronize(parser);
        }
    }
    
    if (!match_token(parser, TOK_RBRACE)) {
        parser_error(parser, "Esperado '}' para fechar bloco");
    }
    
    return block;
}

ASTNode* parse_function_definition(Parser *parser) {
    ASTNode *func_def = ast_node_create(AST_FUNCTION_DEF, parser->current_token);
    advance_token(parser); /* consume 'funcao' */
    
    /* Tipo de retorno */
    if (check_token(parser, TOK_KW_INTEIRO) || 
        check_token(parser, TOK_KW_DECIMAL) || 
        check_token(parser, TOK_KW_TEXTO)) {
        advance_token(parser);
    }
    
    /* Nome da função */
    if (!check_token(parser, TOK_IDENTIFIER)) {
        parser_error(parser, "Esperado nome da função");
        return func_def;
    }
    
    ASTNode *name = ast_node_create(AST_IDENTIFIER, parser->current_token);
    ast_node_add_child(func_def, name);
    advance_token(parser);
    
    if (!match_token(parser, TOK_LPAREN)) {
        parser_error(parser, "Esperado '(' após nome da função");
        return func_def;
    }
    
    /* Parâmetros */
    if (!check_token(parser, TOK_RPAREN)) {
        do {
            ASTNode *param = parse_declaration(parser);
            if (param) {
                ast_node_add_child(func_def, param);
            }
        } while (match_token(parser, TOK_COMMA));
    }
    
    if (!match_token(parser, TOK_RPAREN)) {
        parser_error(parser, "Esperado ')' após parâmetros");
        return func_def;
    }
    
    /* Corpo da função */
    ASTNode *body = parse_block(parser);
    if (body) {
        ast_node_add_child(func_def, body);
    }
    
    return func_def;
}

/* ========== Funções de validação sintática ========== */

int validate_declaration_sequence(ASTNode *program) {
    if (!program || program->type != AST_PROGRAM) return 0;
    
    int found_non_declaration = 0;
    int i;
    
    for (i = 0; i < program->child_count; i++) {
        ASTNode *child = program->children[i];
        
        if (child->type == AST_DECLARATION) {
            if (found_non_declaration) {
                fprintf(stderr, "Erro: declaração após comando não-declarativo na linha %d\n",
                       child->token.line);
                return 0;
            }
        } else {
            found_non_declaration = 1;
        }
    }
    
    return 1;
}

int validate_spacing_rules(ASTNode *program) {
    if (!program || program->type != AST_PROGRAM) return 1;
    
    /* Verificação básica de espaçamento */
    /* Esta função pode ser expandida para verificar regras específicas */
    
    /* Por exemplo:
     * - Verificar se há espaços entre operadores
     * - Verificar espaçamento em declarações
     * - Verificar espaçamento em estruturas de controle
     * - Verificar indentação consistente
     */
    
    /* Por enquanto, retorna sucesso */
    /* Em uma implementação completa, seria necessário analisar o código fonte original */
    /* para verificar espaços, não apenas a AST */
    
    return 1;
}

int validate_variable_usage(ASTNode *program) {
    (void)program; /* Evitar warning de parâmetro não utilizado */
    /* Implementação para verificar se variáveis foram declaradas antes de usar */
    /* Pode ser expandida com tabela de símbolos */
    return 1;
}


