#ifndef TOKEN_H
#define TOKEN_H

/* TokenType: all possible token categories */
typedef enum {
    TOK_EOF,
    TOK_IDENTIFIER,
    TOK_INTEGER_LITERAL,
    TOK_DECIMAL_LITERAL,
    TOK_STRING_LITERAL,

    /* Keywords */
    TOK_KW_INTEIRO,
    TOK_KW_DECIMAL,
    TOK_KW_SE,
    TOK_KW_SENAO,
    TOK_KW_ENQUANTO,
    TOK_KW_PARA,
    TOK_KW_RETORNE,
    TOK_KW_RETORNO,
    TOK_KW_PRINCIPAL,
    TOK_KW_FUNCAO,
    TOK_KW_LEIA,
    TOK_KW_ESCREVA,

    /* Operators */
    TOK_PLUS, TOK_MINUS, TOK_STAR, TOK_SLASH, TOK_CARET,
    TOK_EQ, TOK_NEQ, TOK_LT, TOK_GT, TOK_LE, TOK_GE,
    TOK_ASSIGN,

    /* Punctuation */
    TOK_LPAREN, TOK_RPAREN,
    TOK_LBRACE, TOK_RBRACE,
    TOK_LBRACKET, TOK_RBRACKET,
    TOK_SEMICOLON, TOK_COMMA,

    TOK_ERROR
} TokenType;

/* Token: a lexeme with type and location */
typedef struct {
    TokenType type;
    char *lexeme;   /* null-terminated text */
    int line;       /* line number in source */
} Token;

/* Lookup: returns keyword token if matches, else TOK_IDENTIFIER */
TokenType lookup_keyword(const char *s);
const char *token_type_name(TokenType t);

#endif /* TOKEN_H */
