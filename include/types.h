#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>

/* Tipos básicos suportados pelo compilador */
typedef enum {
    TY_INT,   /* inteiros */
    TY_DEC,   /* decimais */
    TY_TXT,   /* textos */
    TY_BOOL   /* booleanos */
} TypeKind;

/* Metadados para o tipo decimal[a.b] */
typedef struct {
    int a; /* precisão */
    int b; /* escala */
} DecimalInfo;

/* Metadados para o tipo texto[n] */
typedef struct {
    size_t n; /* tamanho máximo */
} TextInfo;

/* Representação de um tipo */
typedef struct Type {
    TypeKind kind; /* categoria do tipo */
    union {
        DecimalInfo dec;
        TextInfo txt;
    } info; /* informações adicionais */
} Type;

#endif /* TYPES_H */

