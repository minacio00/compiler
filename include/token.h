#ifndef TOKEN_H
#define TOKEN_H

/* Todas as categorias de token possíveis */
typedef enum {
    TOK_EOF,
    TOK_IDENTIFIER,

    /* Literais */
    TOK_INTEGER_LITERAL,
    TOK_DECIMAL_LITERAL,
    TOK_STRING_LITERAL,

    /* Palavras reservadas */
    TOK_KW_INTEIRO,
    TOK_KW_DECIMAL,
    TOK_KW_TEXTO,
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

    /* Operadores */
    TOK_PLUS,    /* + */
    TOK_MINUS,   /* - */
    TOK_STAR,    /* * */
    TOK_SLASH,   /* / */
    TOK_CARET,   /* ^ */
    TOK_EQ,      /* == */
    TOK_NEQ,     /* <> */
    TOK_LT,      /* < */
    TOK_GT,      /* > */
    TOK_LE,      /* <= */
    TOK_GE,      /* >= */
    TOK_AND,     /* && */
    TOK_OR,      /* || */
    TOK_ASSIGN,  /* = */

    /* Pontuação */
    TOK_LPAREN,   /* ( */
    TOK_RPAREN,   /* ) */
    TOK_LBRACE,   /* { */
    TOK_RBRACE,   /* } */
    TOK_LBRACKET, /* [ */
    TOK_RBRACKET, /* ] */
    TOK_SEMICOLON,/* ; */
    TOK_COMMA,    /* , */

    /* Erro */
    TOK_ERROR
} TokenType;

/* Representa um token com tipo, lexema e linha */
typedef struct {
    TokenType type;
    char *lexeme;
    int line;
} Token;

/* lookup de palavra-chave */
TokenType lookup_keyword(const char *s);
/* nome textual do token para debug */
const char *token_type_name(TokenType t);

#endif /* TOKEN_H */
