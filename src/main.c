#include <stdio.h>
#include <stdlib.h>
#include "memmgr.h"
#include "lexer.h"
#include "parser.h"
#include "util.h"
#include "error.h"
#include "semantics.h"

ASTNode* parse_file(const char *path) {
    init_scanner(path);
    
    Parser *parser = parser_init();
    if (!parser) {
        fprintf(stderr, "Erro: não foi possível inicializar o parser\n");
        close_scanner();
        return NULL;
    }
    
    printf("\033[34m=== ANÁLISE SINTÁTICA ===\033[0m\n");
    ASTNode *ast = parse_program(parser);
    
    if (parser->had_error) {
        printf("\033[31mErros encontrados durante a análise sintática.\033[0m\n");
        free_ast(ast);
        parser_free(parser);
        close_scanner();
        return NULL;
    }
    
    printf("\033[32mAnálise sintática concluída com sucesso!\033[0m\n\n");
    
    /* Validações adicionais */
    printf("\033[34m=== VALIDAÇÕES SINTÁTICAS ===\033[0m\n");
    
    if (!validate_declaration_sequence(ast)) {
        printf("\033[31mErro: sequência de declarações inválida\033[0m\n");
        free_ast(ast);
        parser_free(parser);
        close_scanner();
        return NULL;
    }
    printf("\033[32m✓ Sequência de declarações válida\033[0m\n");
    
    if (!validate_spacing_rules(ast)) {
        printf("\033[31mErro: regras de espaçamento não respeitadas\033[0m\n");
        free_ast(ast);
        parser_free(parser);
        close_scanner();
        return NULL;
    }
    printf("\033[32m✓ Regras de espaçamento respeitadas\033[0m\n");
    
    if (!validate_variable_usage(ast)) {
        printf("\033[31mErro: uso inválido de variáveis\033[0m\n");
        free_ast(ast);
        parser_free(parser);
        close_scanner();
        return NULL;
    }
    printf("\033[32m✓ Uso de variáveis válido\033[0m\n");

    /* Imprimir AST */
    printf("\n\033[34m=== ÁRVORE SINTÁTICA ABSTRATA ===\033[0m\n");
    print_ast(ast, 0);

    parser_free(parser);
    close_scanner();

    return ast;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo-fonte>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const size_t LIMITE_MEMORIA = 2048 * 1024;

    mm_init(LIMITE_MEMORIA);
    fprintf(stderr, "\033[32mLimite máximo de memória: %zu bytes\033[0m\n", (size_t)LIMITE_MEMORIA);
    
    /* Análise léxica */
    printf("\033[34m=== ANÁLISE LÉXICA ===\033[0m\n");
    int lex_result = lex_file(argv[1]);
    
    if (lex_result != EXIT_SUCCESS) {
        printf("\033[31mErros encontrados durante a análise léxica.\033[0m\n");
        mm_cleanup();
        return lex_result;
    }
    printf("\033[32mAnálise léxica concluída com sucesso!\033[0m\n\n");
    
    /* Análise sintática */
    ASTNode *ast = parse_file(argv[1]);
    if (!ast) {
        mm_cleanup();
        return EXIT_FAILURE;
    }

    /* Análise semântica */
    SemaContext *sc = sema_create(LIMITE_MEMORIA);
    if (!sc) {
        free_ast(ast);
        mm_cleanup();
        return EXIT_FAILURE;
    }
    if (!semantic_analyze(sc, ast)) {
        printf("\033[31mErros encontrados durante a análise semântica.\033[0m\n");
    } else {
        printf("\033[32mAnálise semântica concluída com sucesso!\033[0m\n");
    }
    symtab_print(sc);
    sema_destroy(sc);

    /* Limpeza da AST */
    free_ast(ast);
    
    /* Relatório de memória */
    printf("\n\033[34m=== RELATÓRIO DE MEMÓRIA ===\033[0m\n");
    printf("Uso atual: %zu bytes\n", mm_current_usage());
    printf("Pico de uso: %zu bytes\n", mm_max_usage());
    
    mm_cleanup();

    return EXIT_SUCCESS;
}

