#include "9cc.h"

Token *token;
char *user_input;

void error(char *fmt, ...){
  va_list ap;
  va_start(ap,fmt);

  vfprintf(stderr,fmt,ap);
  fprintf(stderr,"\n");
  exit(1);
}

void error_at(char *loc, char *fmt, ...){
  va_list ap;
  va_start(ap,fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s",pos, " ");
  fprintf(stderr, "^ ");
  vfprintf(stderr,fmt,ap);
  fprintf(stderr,"\n");
  exit(1);
}

bool consume(char *op){
  if(token->kind!=TK_RESERVED||strlen(op)!=token->len||memcmp(token->str,op,token->len))return false;
  token=token->next;
  return true;
}

bool consume_token(TokenKind kind){
  if(token->kind!=kind)
    return false;
  token=token->next;
  return true;
}

Token *consume_ident(){
  if(token->kind!=TK_IDENT)
    return NULL;
  Token *tok=token;
  token=token->next;
  return tok;
}

void expect(char *op){
  if(token->kind!=TK_RESERVED||strlen(op)!=token->len||memcmp(token->str,op,token->len))error_at(token->str, "expected '%s'",op);
  token=token->next;
}

int expect_number(){
  if(token->kind!=TK_NUM)error_at(token->str, "expected a number");
  int val=token->val;
  token=token->next;
  return val;
}

bool at_eof(){
  return token->kind==TK_EOF;
}

Token *new_token(TokenKind kind,Token *cur,char *str){
  Token *tok=calloc(1,sizeof(Token));
  tok->kind=kind;
  tok->str=str;
  cur->next=tok;
  return tok;
}

bool startswith(char *p,char *q){
  return memcmp(p,q,strlen(q))==0;
}

int is_alnum(char c) {
  return ('a' <= c && c <= 'z') ||
         ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') ||
         (c == '_');
}

int judge_ident(char *p,char *ident_word,int len){
  return (!strncmp(p,ident_word,len)&&!is_alnum(p[len]));
}

void create_cur(TokenKind ident,Token **cur,char **p,int len){
  *cur=new_token(ident,*cur,*p);
  (*cur)->len=len;
  *p+=len;
}

Token *tokenize(){
  Token head;
  head.next=NULL;
  Token *cur=&head;
  char *p = user_input;
  while(*p){
    if(isspace(*p)){
      p++;
      continue;
    }
    if(startswith(p,"==")||startswith(p,"!=")||
       startswith(p,"<=")||startswith(p,">=")){
      create_cur(TK_RESERVED,&cur,&p,2);
      continue;
    }
    if(strchr("+-*/()<>;={},&*",*p)){
      create_cur(TK_RESERVED,&cur,&p,1);
      continue;
    }
    if(isdigit(*p)){
      cur=new_token(TK_NUM,cur,p);
      char *q=p;
      cur->val=strtol(p,&p,10);
      cur->len=p-q;
      continue;
    }
    if(judge_ident(p,"return",6)) {
      create_cur(TK_RETURN,&cur,&p,6);
      continue;
    }
    if(judge_ident(p,"if",2)) {
      create_cur(TK_IF,&cur,&p,2);
      continue;
    }
    if(judge_ident(p,"else",4)){
      create_cur(TK_ELSE,&cur,&p,4);
      continue;
    }
    if(judge_ident(p,"while",5)){
      create_cur(TK_WHILE,&cur,&p,5);
      continue;
    }
    if(judge_ident(p,"for",3)){
      create_cur(TK_FOR,&cur,&p,3);
      continue;
    }
    if(isalpha(*p)){
      char *start=p;
      do{
	p++;
      }while(is_alnum(*p));
      cur=new_token(TK_IDENT,cur,start);
      cur->len=p-start;
      continue;
    }
    error_at(p, "invalid token");
  }
  new_token(TK_EOF,cur,p);
  return head.next;
}

int main(int argc,char **argv){
  if(argc!=2)
    error("%s: Invalid number of arguments", argv[0]);
  user_input = argv[1];
  token = tokenize();
  program();
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  for(Function *fn=functions;fn;fn=fn->next){
    gen_function(fn);
  }
  printf(".section .note.GNU-stack,\"\",@progbits\n");
  return 0;
}
