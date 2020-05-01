#ifndef PARSE_H
#define PARSE_H

#include "FunPred.H"

enum token 
{ 
  IDENT, LPAREN, INT, FLOAT, STRING, TIMESTAMP, RPAREN, AND, 
  NOT, OR, EQ, LT, GT, LTE, GTE, NE, PLUS, MINUS, TIMES, DIVIDE,
  COMMA, INT2FLOAT, ABS, FLOAT2INT, OTHER
};

struct IdentInfo 
{
  token type;
  int port;
  int offset;
  int size;
};

struct ExprInfo 
{
  token type;
  Expression* obj;
};

class Parse
{
public:
  Parse();
  ~Parse();
  Predicate* parsePred(const char *in);
  Expression* parseExpr(const char *in);
  Predicate* Pred();
  Predicate* SPred();
  Predicate* RelOp(ExprInfo expr);
  ExprInfo Expr();
  ExprInfo SExpr();
  ExprInfo OptAr(ExprInfo id);
  ExprInfo InfAr(ExprInfo id);
  ExprInfo Const();
  void Match(token t);
  Expression* MakeIdent(IdentInfo id);
  IdentInfo MatchIdent();
  token nextToken();
  char currchar();
  char nextchar();
  void install_int();
  void install_float();
  void install_string();
  int fail();
  void retract(int num);


private:

  int       _curr_int;
  float     _curr_float;
  char      *_curr_string;
  // int       _curr_Ts;
  Timestamp _curr_Ts;
  token     _next_token;
  token     _ident_type;
  char      *_input_string;
  int       _token_beginning;
  int       _curr_place;
  int       _state;
  char      _c;
};

#endif

