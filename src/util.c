#include <stdio.h>
#include <stdlib.h>
#include "util.h"

static FILE *src = NULL;
static int  line_num;
static int  last_char;

void init_scanner(const char *path) {
    src = fopen(path, "r");
    if (!src) { perror(path); exit(EXIT_FAILURE); }
    line_num = 1;
    last_char = fgetc(src);
}

void close_scanner(void) {
    if (src) fclose(src);
}

int peek_char(void) {
    return (last_char == EOF ? '\0' : last_char);
}

int advance_char(void) {
    int c = peek_char();
    if (c == '\0') return '\0';
    last_char = fgetc(src);
    if (c == '\n') line_num++;
    return c;
}

void retreat_char(void) {
    ungetc(last_char, src);
    last_char = fgetc(src);
}


int current_line(void) {
    return line_num;
}
