#ifndef UTIL_H
#define UTIL_H

void init_scanner(const char *path);

void close_scanner(void);

int peek_char(void);
int advance_char(void);
void retreat_char(void);
int current_line(void);

#endif /* UTIL_H */
