#include <string.h>
#include "token.h"

static struct { const char *kw; TokenType tok; } keywords[] = {
    {"inteiro", TOK_KW_INTEIRO},
    {"se",      TOK_KW_SE},
    {"senao",   TOK_KW_SENAO},
    {"enquanto",TOK_KW_ENQUANTO},
    {"retorne", TOK_KW_RETORNE},
    {NULL, 0}
};

TokenType lookup_keyword(const char *s) {
    int i = 0;
    while (keywords[i].kw) {
        if (strcmp(s, keywords[i].kw) == 0)
            return keywords[i].tok;
        i++;
    }
    return TOK_IDENTIFIER;
}

const char *token_type_name(TokenType t) {
    switch (t) {
        case TOK_EOF:              return "TOK_EOF";
        case TOK_IDENTIFIER:       return "TOK_IDENTIFIER";
        case TOK_INTEGER_LITERAL:  return "TOK_INTEGER_LITERAL";
        case TOK_STRING_LITERAL:   return "TOK_STRING_LITERAL";
        case TOK_KW_INTEIRO:       return "TOK_KW_INTEIRO";
        case TOK_KW_SE:            return "TOK_KW_SE";
        case TOK_KW_SENAO:         return "TOK_KW_SENAO";
        case TOK_KW_ENQUANTO:      return "TOK_KW_ENQUANTO";
        case TOK_KW_RETORNE:       return "TOK_KW_RETORNE";
        case TOK_PLUS:             return "TOK_PLUS";
        case TOK_MINUS:            return "TOK_MINUS";
        case TOK_STAR:             return "TOK_STAR";
        case TOK_SLASH:            return "TOK_SLASH";
        case TOK_CARET:            return "TOK_CARET";
        case TOK_EQ:               return "TOK_EQ";
        case TOK_NEQ:              return "TOK_NEQ";
        case TOK_LT:               return "TOK_LT";
        case TOK_GT:               return "TOK_GT";
        case TOK_LE:               return "TOK_LE";
        case TOK_GE:               return "TOK_GE";
        case TOK_ASSIGN:           return "TOK_ASSIGN";
        case TOK_LPAREN:           return "TOK_LPAREN";
        case TOK_RPAREN:           return "TOK_RPAREN";
        case TOK_LBRACE:           return "TOK_LBRACE";
        case TOK_RBRACE:           return "TOK_RBRACE";
        case TOK_SEMICOLON:        return "TOK_SEMICOLON";
        case TOK_COMMA:            return "TOK_COMMA";
        case TOK_ERROR:            return "TOK_ERROR";
        default:                   return "TOK_UNKNOWN";
    }
}

