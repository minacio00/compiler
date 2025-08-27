#ifndef TYPES_H
#define TYPES_H

typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_VOID
} TypeKind;

typedef struct Type {
    TypeKind kind;
} Type;

#endif /* TYPES_H */
