#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "error.h"

void lex_error(int line, const char *fmt, ...) {
    va_list ap;
    fprintf(stderr, "Lexical error (line %d): ", line);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}
