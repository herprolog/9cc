#include "9cc.h"

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_num(int val){
  Node *node=calloc(1,sizeof(Node));
  node->kind=ND_NUM;
  node->val=val;
  return node;
}

typedef struct LVar LVar;

struct LVar{
  LVar *next;
  char *name;
  int len;
  int offset;
};

LVar *locals;

LVar *find_lvar(Token *tok){
  for(LVar *var=locals;var;var=var->next)
    if(var->len==tok->len&&!memcmp(tok->str,var->name,var->len))
      return var;
  return NULL;
}

LVar *new_lvar(Token *tok){
  LVar *lvar=calloc(1,sizeof(LVar));
  lvar->next=locals;
  lvar->name=tok->str;
  lvar->len=tok->len;
  if(locals)
    lvar->offset=locals->offset+8;
  else
    lvar->offset=8;
  locals=lvar;
  return lvar;

void program();
Node *stmt();
Node *assign();
Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

Function functions;;

Node *assign(){
  Node *node=equality();
  if(consume("="))
    node=new_node(ND_ASSIGN,node,assign());
  return node;
}

// expr = equality
extern Node *expr() {
  return assign();
}

Node *stmt() {
  Node *node;
  if(consume("{")){
    node=calloc(1,sizeof(Node));
    node->kind=ND_BLOCK;
    while(!consume("}")){
      if(at_eof())
	error_at(token->str,"expected '}'");
      if(node->stmts_len==node->stmts_cap){
	node->stmts_cap=node->stmts_cap?node->stmts_cap*2:4;
	node->stmts=realloc(node->stmts,sizeof(Node*)*node->stmts_cap);
      }
      node->stmts[node->stmts_len++]=stmt();
    }
    return node;
  }
  if(consume_token(TK_IF)){
    node=calloc(1,sizeof(Node));
    node->kind=ND_IF;
    expect("(");
    node->lhs=expr();
    expect(")");
    node->rhs=stmt();
    if(consume_token(TK_ELSE))
      node->third=stmt();
    return node;
  }
  if(consume_token(TK_WHILE)){
    node=calloc(1,sizeof(Node));
    node->kind=ND_WHILE;
    expect("(");
    node->lhs=expr();
    expect(")");
    node->body=stmt();
    return node;
  }
  if(consume_token(TK_FOR)){
    node=calloc(1,sizeof(Node));
    node->kind=ND_FOR;
    expect("(");
    if(!consume(";")){
      node->lhs=expr();
      expect(";");
    }
    if(!consume(";")){
      node->rhs=expr();
      expect(";");
    }
    if(!consume(")")){
      node->third=expr();
      expect(")");
    }
    node->body=stmt();
    return node;
  }
  if (consume_token(TK_RETURN)) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = expr();
  } else {
    node = expr();
  }

  if (!consume(";"))
    error_at(token->str, "You need ';'");
  return node;
}

void program(){
  Function head=();
  Function *cur=&head;
  while(!at_eof()){
    Function *fn=calloc(1,sizeof(Function));
    Token *name=consume_ident();
    if(!name)
      error_at(token->str,"expexted a function name");
    fn->name=name->str;
    fn->le=name->len;
    locals=NULL;
    expect("(");
    if(!consume(")")){
      do{
        Token *param=consume_ident();
        if(!param)
          erro_at(token->str,"expected a parameter name");
        LVar *var=new_lvar(param);
        fn->param_cnt++;
        fn->param_offsets=realloc(
      			    fn->param_offsets,
      			    sizeof(int)*fn->param_cnt
      			   );
        fn->param_offsets[fn->param_cnt-1]=var->offset;
      }while(consume(","));
      expect(")");
    }
    fn->body=stmt();
    if(locals)
      fn->stack_size=locals_offsets;
    cur->next=fn;
    cur=fn;
  }
  functions=head.next;
}

Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("=="))
      node = new_node(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_node(ND_NE, node, relational());
    else
      return node;
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<"))
      node = new_node(ND_LT, node, add());
    else if (consume("<="))
      node = new_node(ND_LE, node, add());
    else if (consume(">"))
      node = new_node(ND_LT, add(), node);
    else if (consume(">="))
      node = new_node(ND_LE, add(), node);
    else
      return node;
  }
}

// add = mul ("+" mul | "-" mul)*
Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+"))
      node = new_node(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_node(ND_SUB, node, mul());
    else
      return node;
  }
}


// mul = unary ("*" unary | "/" unary)*
Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*"))
      node = new_node(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_node(ND_DIV, node, unary());
    else
      return node;
  }
}

// unary = ("+" | "-")? unary
//       | primary
Node *unary() {
  if (consume("+"))
    return unary();
  if (consume("-"))
    return new_node(ND_SUB, new_num(0), unary());
  return primary();
}

// primary = "(" expr ")" | num | ident
Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }
  if(token->kind==TK_NUM)
    return new_num(expect_number());
  
  Token *tok = consume_ident();
  if (tok) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;

    LVar *lvar = find_lvar(tok);
    if (lvar) {
      node->offset = lvar->offset;
    } else {
      lvar = calloc(1, sizeof(LVar));
      lvar->next = locals;
      lvar->name = tok->str;
      lvar->len = tok->len;

      if(locals)
	lvar->offset = locals->offset + 8;
      else
	lvar->offset = 8;
      
      node->offset = lvar->offset;
      locals = lvar;
    }
    return node;
  }

  error_at(token->str,"expected an expression");
}
