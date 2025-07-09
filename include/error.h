#ifndef ERROR_H
#define ERROR_H
#include <stdarg.h>

void lex_error(int line, const char *fmt, ...);

#endif /* ERROR_H */
