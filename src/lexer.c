#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "lexer.h"
#include "util.h"
#include "error.h"
#include "token.h"

/* Helpers for building lexemes */
static char *make_lexeme(const char *start, size_t len) {
    char *s = malloc(len+1);
    if (!s) exit(EXIT_FAILURE);
    memcpy(s, start, len);
    s[len] = '\0';
    return s;
}

static void skip_irrelevant(void) {
    int c;
    while ((c = peek_char())) {
        if (isspace(c)) { advance_char(); continue; }
        if (c == '/' && (advance_char(), peek_char() == '/')) {
            /* single-line comment */
            while (peek_char() && advance_char() != '\n');
            continue;
        }
        if (c == '/' && (advance_char(), peek_char() == '*')) {
            /* block comment */
            advance_char();
            while (1) {
                int d = advance_char();
                if (d == '\0') lex_error(current_line(), "Comentário não terminado");
                if (d == '*' && peek_char() == '/') { advance_char(); break; }
            }
            continue;
        }
        break;
    }
}

Token next_token(void) {
    skip_irrelevant();
    int c = peek_char();
    if (c == '\0') {
        return (Token){.type = TOK_EOF, .lexeme = "", .line = current_line()};
    }

    /* Variável: ![a-z][a-zA-Z0-9]* */
    if (c == '!') {
        int start_line = current_line();
        advance_char();
        if (peek_char() < 'a' || peek_char() > 'z') {
            lex_error(start_line,
                      "Nome inválido para variável: esperado [a–z] após '!', recebido '%c'",
                      peek_char());
        }
        char buf[256];
        int len = 0;
        buf[len++] = '!';
        buf[len++] = advance_char();
        while (isalnum(peek_char())) {
            if (len < (int)sizeof(buf)-1) buf[len++] = advance_char();
            else advance_char();
        }
        buf[len] = '\0';
        return (Token){
            .type   = TOK_IDENTIFIER,
            .lexeme = make_lexeme(buf, len),
            .line   = start_line
        };
    }

    /* Nome de função: __[a-zA-Z0-9][a-zA-Z0-9]* */
    if (c == '_') {
        int start_line = current_line();
        /* verifica dois underlines */
        if (peek_char() != '_' ) {
            lex_error(start_line,
                      "Nome de função inválido: deve começar com '__' seguido de letra ou dígito");
        }
        /* consome primeiro '_' */
        advance_char();
        if (peek_char() != '_') {
            lex_error(start_line,
                      "Nome de função inválido: deve começar com '__' seguido de letra ou dígito");
        }
        /* consome segundo '_' */
        char buf[256];
        int len = 0;
        buf[len++] = advance_char();
        buf[len++] = advance_char();
        /* agora peek_char() é o primeiro caractere do nome da função */
        if (!isalnum(peek_char())) {
            lex_error(start_line,
                      "Nome de função inválido: deve vir letra ou dígito após '__'");
        }
        while (isalnum(peek_char())) {
            if (len < (int)sizeof(buf)-1) buf[len++] = advance_char();
            else advance_char();
        }
        buf[len] = '\0';
        return (Token){
            .type   = TOK_IDENTIFIER,
            .lexeme = make_lexeme(buf, len),
            .line   = start_line
        };
    }

    /* Palavra-chave ou erro: [a-zA-Z][a-zA-Z0-9_]* */
    if (isalpha(c)) {
        int start_line = current_line();
        char buf[256];
        int len = 0;
        while (isalnum(peek_char()) || peek_char() == '_') {
            if (len < (int)sizeof(buf)-1) buf[len++] = advance_char();
            else advance_char();
        }
        buf[len] = '\0';
        TokenType type = lookup_keyword(buf);
        if (type == TOK_IDENTIFIER) {
            lex_error(start_line,
                      "Identificador inválido: '%s' não é palavra-chave, nem nome de função nem variável",
                      buf);
        }
        return (Token){
            .type   = type,
            .lexeme = make_lexeme(buf, len),
            .line   = start_line
        };
    }

    /* Número literal (inteiro ou decimal) */
    if (isdigit(c)) {
        int start_line = current_line();
        char buf[64]; int len = 0;
        /* parte inteira */
        while (isdigit(peek_char())) {
            if (len < (int)sizeof(buf)-1) buf[len++] = advance_char();
            else advance_char();
        }
        /* ponto decimal? */
        if (peek_char() == '.') {
            if (len < (int)sizeof(buf)-1) buf[len++] = advance_char();
            else advance_char();
            if (!isdigit(peek_char())) {
                lex_error(start_line,
                          "Número decimal inválido: faltando dígitos após o ponto '.'");
            }
            while (isdigit(peek_char())) {
                if (len < (int)sizeof(buf)-1) buf[len++] = advance_char();
                else advance_char();
            }
            buf[len] = '\0';
            return (Token){.type=TOK_DECIMAL_LITERAL, .lexeme=make_lexeme(buf,len), .line=start_line};
        }
        buf[len] = '\0';
        return (Token){.type=TOK_INTEGER_LITERAL, .lexeme=make_lexeme(buf,len), .line=start_line};
    }

    /* Literal de string */
    if (c == '"') {
        int start_line = current_line(); advance_char();
        char buf[512]; int len = 0;
        while (peek_char() && peek_char() != '"') {
            if (peek_char() == '\\') buf[len++] = advance_char();
            buf[len++] = advance_char();
            if (len >= (int)sizeof(buf)-1) lex_error(start_line, "String muito longa");
        }
        if (peek_char() != '"') lex_error(start_line, "String sem terminação");
        advance_char();
        buf[len] = '\0';
        return (Token){.type=TOK_STRING_LITERAL, .lexeme=make_lexeme(buf,len), .line=start_line};
    }

    /* Operadores e pontuação */
    int start_line = current_line();
    char first = advance_char();
    switch (first) {
        case '+': return (Token){.type=TOK_PLUS,     .lexeme="+", .line=start_line};
        case '-': return (Token){.type=TOK_MINUS,    .lexeme="-", .line=start_line};
        case '*': return (Token){.type=TOK_STAR,     .lexeme="*", .line=start_line};
        case '/': return (Token){.type=TOK_SLASH,    .lexeme="/", .line=start_line};
        case '^': return (Token){.type=TOK_CARET,    .lexeme="^", .line=start_line};
        case '=':
            if (peek_char() == '=') { advance_char();
                return (Token){.type=TOK_EQ, .lexeme=make_lexeme("==",2), .line=start_line}; }
            return (Token){.type=TOK_ASSIGN, .lexeme=make_lexeme("=",1), .line=start_line};
        case '<':
            if (peek_char()=='=') { advance_char(); return (Token){.type=TOK_LE,  .lexeme="<=", .line=start_line}; }
            if (peek_char()=='>') { advance_char(); return (Token){.type=TOK_NEQ, .lexeme="<>", .line=start_line}; }
            return (Token){.type=TOK_LT, .lexeme="<", .line=start_line};
        case '>':
            if (peek_char()=='=') { advance_char(); return (Token){.type=TOK_GE, .lexeme=">=", .line=start_line}; }
            return (Token){.type=TOK_GT, .lexeme=">", .line=start_line};
        case '(': return (Token){.type=TOK_LPAREN, .lexeme="(", .line=start_line};
        case ')': return (Token){.type=TOK_RPAREN, .lexeme=")", .line=start_line};
        case '{': return (Token){.type=TOK_LBRACE, .lexeme="{", .line=start_line};
        case '}': return (Token){.type=TOK_RBRACE, .lexeme="}", .line=start_line};
        case '[': return (Token){.type=TOK_LBRACKET, .lexeme="[", .line=start_line};
        case ']': return (Token){.type=TOK_RBRACKET, .lexeme="]", .line=start_line};
        case ';': return (Token){.type=TOK_SEMICOLON,.lexeme=";", .line=start_line};
        case ',': return (Token){.type=TOK_COMMA,   .lexeme=",", .line=start_line};
        default:
            lex_error(start_line, "Caractere inesperado: '%c'", first);
            return (Token){.type=TOK_ERROR, .lexeme=make_lexeme("",0), .line=start_line};
    }
}

int lex_file(const char *path) {
    init_scanner(path);
    Token tok;
    do {
        tok = next_token();
        printf("%4d: %-15s '%s'\n", tok.line,
               token_type_name(tok.type), tok.lexeme);
    } while (tok.type != TOK_EOF);
    close_scanner();
    return 0;
}
