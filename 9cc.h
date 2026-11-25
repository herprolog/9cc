#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum{
  TK_RESERVED, // symbol
  TK_IDENT, // identifier
  TK_NUM, // integer
  TK_EOF, //end of input
  TK_RETURN, // return
  TK_IF, // if
  TK_ELSE, // if-else
  TK_WHILE, //while
  TK_FOR, //for
}TokenKind;

typedef struct Token Token;

struct Token{
  TokenKind kind;
  Token *next;
  int val;
  char *str;
  int len;
};

typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_EQ,  // ==
  ND_NE,  // !=
  ND_LT,  // <
  ND_LE,  // <=
  ND_NUM, // Integer
  ND_LVAR, // variable
  ND_ASSIGN, // assignment
  ND_RETURN, // return
  ND_IF, // if
  ND_WHILE, // while
  ND_FOR, // for
  ND_BLOCK, // block
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind; // Node kind
  Node *lhs;     // Left-hand side
  Node *rhs;     // Right-hand side
  Node *third;   // else
  Node *body;    // for/while content
  Node **stmts;  // block statement
  int val;       // Used if kind == ND_NUM
  int offset;    // Used if kind == ND_LVAR
  int stmts_len; // stmts length
  int stmts_cap; // stmts capacity
};

extern Node *code[100];

Node *expr();

extern char *user_input;

extern Token *token;

void gen(Node *node);

void error_at(char *loc, char *fmt, ...);

bool consume(char *op);

bool consume_token(TokenKind kind);

Token *consume_ident();

void expect(char *op);

int expect_number();

bool at_eof();

Token *new_token(TokenKind kind,Token *cur,char *str);

bool startswith(char *p,char *q);

Token *tokenize();

void error(char *fmt, ...);

void program();
