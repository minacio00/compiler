#include "memmgr.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static size_t mm_limit       = 0;  
static size_t mm_current     = 0;  
static size_t mm_high_water  = 0;  

typedef struct BlockHeader {
    size_t size;                
    struct BlockHeader *next;   
} BlockHeader;

static BlockHeader *mm_head = NULL; 
static void mm_check(size_t new_current) {
    if (new_current > mm_limit && mm_limit > 0) {
        fprintf(stderr, "\033[31mMemória Insuficiente\033[0m\n");
        exit(EXIT_FAILURE);
    }
    if (new_current >= mm_limit * 0.9 && new_current < mm_limit) {
        fprintf(stderr, "Alerta: uso de memória entre 90%% e 99%%\n");
    }
    if (new_current > mm_high_water) mm_high_water = new_current;
}

void mm_init(size_t max_bytes) {
    mm_limit      = max_bytes;
    mm_current    = 0;
    mm_high_water = 0;
    mm_head       = NULL;
}

void *mm_malloc(size_t size) {
    size_t total = sizeof(BlockHeader) + size;
    if (mm_limit == 0) {
        fprintf(stderr, "Memória não inicializada\n");
        exit(EXIT_FAILURE);
    }
    mm_check(mm_current + total);

    BlockHeader *h = (BlockHeader *)malloc(total);
    if (!h) {
        fprintf(stderr, "\033[31mMemória Insuficiente\033[0m\n");
        exit(EXIT_FAILURE);
    }

    h->size = size;
    h->next = mm_head; /* encadeia */
    mm_head = h;

    mm_current += total;
    mm_check(mm_current);
    return (void *)(h + 1); /* retorna ponteiro após o cabeçalho */
}

void *mm_realloc(void *ptr, size_t new_size) {
    if (!ptr) return mm_malloc(new_size);

    BlockHeader *oldh = (BlockHeader *)ptr - 1;
    size_t old_total  = sizeof(BlockHeader) + oldh->size;
    size_t new_total  = sizeof(BlockHeader) + new_size;

    if (new_total > old_total) mm_check(mm_current + (new_total - old_total));

    BlockHeader *newh = (BlockHeader *)realloc(oldh, new_total);
    if (!newh) {
        fprintf(stderr, "\033[31mMemória Insuficiente\033[0m\n");
        exit(EXIT_FAILURE);
    }

    /* Se o bloco realocou em outro endereço, precisamos ajustar a lista */
    if (mm_head == oldh) {
        mm_head = newh;
    } else {
        BlockHeader *p = mm_head;
        while (p && p->next != oldh) p = p->next;
        if (p) p->next = newh;
    }

    newh->size = new_size;
    mm_current += (new_total - old_total);
    mm_check(mm_current);
    return (void *)(newh + 1);
}

void mm_free(void *ptr) {
    if (!ptr) return;
    BlockHeader *h = (BlockHeader *)ptr - 1;

    /* remove da lista */
    if (mm_head == h) {
        mm_head = h->next;
    } else {
        BlockHeader *p = mm_head;
        while (p && p->next != h) p = p->next;
        if (p) p->next = h->next;
    }

    size_t total = sizeof(BlockHeader) + h->size;
    mm_current -= total;
    free(h);
}

size_t mm_current_usage(void) { return mm_current; }
/* Limite máximo estabelecido via mm_init */
size_t mm_max_usage(void) { return mm_limit; }
/* Maior pico de uso observado */
size_t mm_peak_usage(void) { return mm_high_water; }

void mm_usage_guard(void) {
    size_t usage = mm_current_usage();
    size_t limit = mm_max_usage();
    if (limit > 0) {
        if (usage >= limit) {
            fprintf(stderr, "Memória Insuficiente\n");
            exit(EXIT_FAILURE);
        } else if (usage >= (size_t)(0.9 * (double)limit)) {
            fprintf(stderr, "Alerta: uso de memória entre 90%% e 99%%\n");
        }
    }
}

void mm_cleanup(void) {
    BlockHeader *h = mm_head;
    while (h) {
        BlockHeader *next = h->next;
        free(h);
        h = next;
    }
    mm_head    = NULL;
    mm_current = 0;
}

