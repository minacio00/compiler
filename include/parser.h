#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "types.h"

/* Tipos de nós da árvore sintática */
typedef enum {
    AST_PROGRAM,
    AST_DECLARATION,
    AST_ASSIGNMENT,
    AST_EXPRESSION,
    AST_IF_STMT,
    AST_WHILE_STMT,
    AST_FOR_STMT,
    AST_READ_STMT,
    AST_WRITE_STMT,
    AST_BLOCK,
    AST_BINARY_OP,
    AST_UNARY_OP,
    AST_LITERAL,
    AST_IDENTIFIER,
    AST_FUNCTION_DEF,
    AST_FUNCTION_CALL,
    AST_RETURN_STMT
} ASTNodeType;

/* Estrutura para nó da árvore sintática abstrata */
typedef struct ASTNode {
    ASTNodeType type;
    Token token;                    /* Token associado ao nó */
    struct ASTNode **children;      /* Array de ponteiros para filhos */
    int child_count;                /* Número de filhos */
    int child_capacity;             /* Capacidade do array de filhos */
    char *value;                    /* Valor opcional do nó */
    Type inferred_type;             /* Tipo inferido pelo analisador semântico */
} ASTNode;

/* Estrutura do analisador sintático */
typedef struct {
    Token current_token;            /* Token atual sendo processado */
    Token previous_token;           /* Token anterior */
    int had_error;                  /* Flag de erro durante parsing */
    int panic_mode;                 /* Flag de modo pânico para recuperação de erro */
} Parser;

/* Funções principais do analisador sintático */
Parser* parser_init(void);
void parser_free(Parser *parser);
ASTNode* parse_program(Parser *parser);
void print_ast(ASTNode *node, int depth);
void free_ast(ASTNode *node);

/* Funções auxiliares */
ASTNode* ast_node_create(ASTNodeType type, Token token);
void ast_node_add_child(ASTNode *parent, ASTNode *child);

/* Funções de parsing específicas */
ASTNode* parse_declaration(Parser *parser);
ASTNode* parse_statement(Parser *parser);
ASTNode* parse_assignment(Parser *parser);
ASTNode* parse_expression(Parser *parser);
ASTNode* parse_if_statement(Parser *parser);
ASTNode* parse_while_statement(Parser *parser);
ASTNode* parse_for_statement(Parser *parser);
ASTNode* parse_read_statement(Parser *parser);
ASTNode* parse_write_statement(Parser *parser);
ASTNode* parse_block(Parser *parser);
ASTNode* parse_function_definition(Parser *parser);

/* Funções de verificação e controle */
int check_token(Parser *parser, TokenType type);
int match_token(Parser *parser, TokenType type);
void advance_token(Parser *parser);
void synchronize(Parser *parser);
void parser_error(Parser *parser, const char *message);

/* Funções de validação sintática */
int validate_declaration_sequence(ASTNode *program);
int validate_spacing_rules(ASTNode *program);
int validate_variable_usage(ASTNode *program);

#endif /* PARSER_H */
