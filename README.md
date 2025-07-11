# Compilador - Analisador Léxico

## Alunos: Amado Pinheiro; Mateus Amelio

Este projeto implementa um analisador léxico simples escrito em C. Ele reconhece tokens de uma linguagem fictícia para fins acadêmicos e sinaliza erros léxicos durante a leitura de um arquivo fonte.

## Estrutura do projeto

```
.
├── Dockerfile        # Ambiente de compilação em contêiner
├── Makefile          # Regras para compilar o analisador
├── include/          # Arquivos de cabeçalho (.h)
├── src/              # Código‑fonte em C
├── tests/            # Exemplos de arquivos de entrada
└── build/            # Objetos compilados (gerado pelo make)
```

### Componentes

- **src/main.c** – ponto de entrada que apenas chama `lex_file`.
- **src/lexer.c** – implementação do analisador léxico.
- **src/util.c** – utilidades para leitura de caracteres do arquivo.
- **src/token.c** – definição e nomeação dos tokens e palavras‑chave.
- **src/error.c** – tratamento de mensagens de erro.
- **include/** – diretório com os respectivos cabeçalhos das unidades acima.
- **tests/** – contém pequenos programas de exemplo usados para testar o léxico.

## Comandos `make`

- `make` – compila o projeto gerando o executável `lex` e o diretório `build/`.
- `make clean` – remove arquivos objetos e o executável.

## Como executar

1. Compile o projeto:

```bash
make
```

2. Execute o analisador apontando para um arquivo fonte. Por exemplo:

```bash
./lex tests/error_test.src
```

A saída listará os tokens identificados ou mensagens de erro. O arquivo acima produz:

```
   1: TOK_KW_INTEIRO  'inteiro'
   1: TOK_IDENTIFIER  '!x'
   1: TOK_ASSIGN      '='
   1: TOK_INTEGER_LITERAL '5'
   1: TOK_SEMICOLON   ';'
   2: TOK_EOF         ''
```
