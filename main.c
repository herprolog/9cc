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
    if (startswith(p, "==") || startswith(p, "!=") ||
        startswith(p, "<=") || startswith(p, ">=")) {
      cur = new_token(TK_RESERVED, cur, p);
      cur->len=2;
      p += 2;
      continue;
    }
    if(strchr("+-*/()<>;=",*p)){
      cur=new_token(TK_RESERVED,cur,p++);
      cur->len=1;
      continue;
    }
    if(isdigit(*p)){
      cur=new_token(TK_NUM,cur,p);
      char *q=p;
      cur->val=strtol(p,&p,10);
      cur->len=p-q;
      continue;
    }
    if(!strncmp(p,"return",6)&& !is_alnum(p[6])) {
      cur=new_token(TK_RETURN,cur,p);
      cur->len=6;
      p+=6;
      continue;
    }
    if(!strncmp(p,"if",2)&&!is_alnum(p[2])) {
      cur=new_token(TK_IF,cur,p);
      cur->len=2;
      p+=2;
      continue;
    }
    if(!strncmp(p,"else",4)&&!is_alnum(p[4])){
      cur=new_token(TK_ELSE,cur,p);
      cur->len=4;
      p+=4;
      continue;
    }
    if(!strncmp(p,"while",5)&&!is_alnum(p[5])){
      cur=new_token(TK_WHILE,cur,p);
      cur->len=5;
      p+=5;
      continue;
    }
    if(!strncmp(p,"for",3)&&!is_alnum(p[3])){
      cur=new_token(TK_FOR,cur,p);
      cur->len=3;
      p+=3;
      continue;
    }
    if(('a'<=*p&&*p<='z')||('A'<=*p&&*p<='Z')){
      char *start=p;
      do{
	p++;
      }while(('a'<=*p&&*p<='z')||
	     ('A'<=*p&&*p<='Z')||
	     ('0'<=*p&&*p<='9')||
	     *p=='_');
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
  printf("main:\n");
  printf("  push rbp\n");
  printf("  mov rbp,rsp\n");
  printf("  sub rsp,208\n");
  for(int i=0;code[i];i++){
    gen(code[i]);
    printf("  pop rax\n");
  }
  printf("  mov rsp,rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
  return 0;
}
