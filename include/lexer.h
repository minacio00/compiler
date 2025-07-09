#ifndef LEXER_H
#define LEXER_H
#include "token.h"

int lex_file(const char *path);

Token next_token(void);

#endif /* LEXER_H */
