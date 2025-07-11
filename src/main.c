#include <stdio.h>
#include <stdlib.h>
#include "memmgr.h"
#include "lexer.h"
#include "error.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo-fonte>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const size_t LIMITE_MEMORIA = 2048 * 1024;

    mm_init(LIMITE_MEMORIA);
   fprintf(stderr, "\033[32mLimite máximo de memória: %zu bytes\033[0m\n", (size_t)LIMITE_MEMORIA);
    int result = lex_file(argv[1]);

    mm_cleanup();

    return result;
}

