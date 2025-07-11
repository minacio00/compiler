
#ifndef MEMMGR_H
#define MEMMGR_H

#include <stddef.h>

void mm_init(size_t max_bytes);

void *mm_malloc(size_t size);
void mm_free(void *ptr);
void *mm_realloc(void *ptr, size_t new_size);

size_t mm_current_usage(void);
size_t mm_max_usage(void);


void mm_cleanup(void);


#endif 

