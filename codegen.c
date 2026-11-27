#include "9cc.h"

static int labelseq=0;

void gen_lval(Node *node){
  if(node->kind!=ND_LVAR)
    error("left assignment is not a variable");

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n",node->offset);
  printf("  push rax\n");
}

void gen_function(Function *fn){
  static char *argreg[]={
    "edi","rsi","rdx","rcx","r8","r9"
  };
  if(fn->param_cnt>6)
    error("too many parameters");
  printf("%.*s:\n",fn->len,fn->name);
  printf("  push rbp\n");
  printf("  mov rbp rsp\n");
  printf("  sub rsp,%d\n",fn->stack_size);
  for(int i=0;i<fn->param_cnt;i++)
    printf("  mov [rbp-%d],%s\n",fn->param_offsets[i],argreg[i]);
  gen(fn->body);
  printf("  pop rax\n");
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
}
  
void gen(Node *node) {
  if(node->kind==ND_BLOCK){
    for(int i=0;i<node->stmts_len;i++){
      gen(node->stmts[i]);
      printf("  pop rax\n");
    }
    printf("  push 0\n");
    return;
  }
  if(node->kind==ND_IF){
    int label=labelseq++;
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    if(node->third){
      printf("  je .Lelse%d\n",label);
      gen(node->rhs);
      printf("  pop rax\n");
      printf("  jmp .Lend%d\n",label);
      printf(".Lelse%d:\n",label);
      gen(node->third);
      printf("  pop rax\n");
      printf(".Lend%d:\n",label);
    }else{
      printf("  je .Lend%d\n",label);
      gen(node->rhs);
      printf("  pop rax\n");
      printf(".Lend%d:\n",label);
    }
    printf("  push 0\n");
    return;
  }
  if(node->kind==ND_WHILE){
    int label=labelseq++;
    printf(".Lbegin%d:\n",label);
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  cmp rax,0\n");
    printf("  je .Lend%d\n",label);
    gen(node->body);
    printf("  pop rax\n");
    printf("  jmp .Lbegin%d\n",label);
    printf(".Lend%d:\n",label);
    printf("  push 0\n");
    return;
  }
  if(node->kind==ND_FOR){
    int label=labelseq++;
    if(node->lhs){
      gen(node->lhs);
      printf("  pop rax\n");
    }
    printf(".Lbegin%d:\n",label);
    if(node->rhs){
      gen(node->rhs);
      printf("  pop rax\n");
      printf("  cmp rax,0\n");
      printf("  je .Lend%d\n",label);
    }
    gen(node->body);
    printf("  pop rax\n");
    if(node->third){
      gen(node->third);
      printf("  pop rax\n");
    }
    printf(" jmp .Lbegin%d\n",label);
    printf(".Lend%d:\n",label);
    printf("  push 0\n");
    return;
  }
  if (node->kind == ND_RETURN) {
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  }
  switch(node->kind){
  case ND_NUM:
    printf("  push %d\n", node->val);
    return;
  case ND_LVAR:
    gen_lval(node);
    printf("  pop rax\n");
    printf("  mov rax,[rax]\n");
    printf("  push rax\n");
    return;
  case ND_ASSIGN:
    gen_lval(node->lhs);
    gen(node->rhs);
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax],rdi\n");
    printf("  push rdi\n");
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
    case ND_ADD:
      printf("  add rax, rdi\n");
      break;
    case ND_SUB:
      printf("  sub rax, rdi\n");
      break;
    case ND_MUL:
      printf("  imul rax, rdi\n");
      break;
    case ND_DIV:
      printf("  cqo\n");
      printf("  idiv rdi\n");
      break;
    case ND_EQ:
      printf("  cmp rax, rdi\n");
      printf("  sete al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_NE:
      printf("  cmp rax, rdi\n");
      printf("  setne al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LT:
      printf("  cmp rax, rdi\n");
      printf("  setl al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LE:
      printf("  cmp rax, rdi\n");
      printf("  setle al\n");
      printf("  movzb rax, al\n");
      break;
  }

  printf("  push rax\n");
}
