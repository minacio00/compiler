
#ifndef MEMMGR_H
#define MEMMGR_H

#include <stddef.h>

void mm_init(size_t max_bytes);

void *mm_malloc(size_t size);
void mm_free(void *ptr);
void *mm_realloc(void *ptr, size_t new_size);

size_t mm_current_usage(void);
/* Retorna o limite máximo configurado para o gerenciador */
size_t mm_max_usage(void);
/* Retorna o maior pico de uso registrado */
size_t mm_peak_usage(void);
void mm_usage_guard(void);


void mm_cleanup(void);


#endif 

