#include "Parse.H"

Parse::Parse() {
  _curr_string = NULL;
  _input_string = NULL;
}

Parse::~Parse() {
  if (_curr_string)
    delete[] _curr_string; 
  if (_input_string)
    delete[] _input_string;
}

Predicate* Parse::parsePred(const char *in) 
{
  _input_string =new char[strlen(in) + 1];
  strcpy(_input_string,in);
  _token_beginning = 0;
  _state = 0;
  _curr_place = -1;
  _next_token = nextToken();
  return Pred();
}

Expression* Parse::parseExpr(const char *in) 
{
  _input_string =new char[strlen(in) + 1];
  strcpy(_input_string,in);
  _token_beginning = 0;
  _state = 0;
  _curr_place = -1;
  _next_token = nextToken();
  return Expr().obj;
}


Predicate* Parse::Pred() 
{
  Predicate *left;
  Predicate *right;
  
  switch (_next_token) 
    {
    case IDENT:
    case LPAREN:
    case INT:
    case FLOAT:
    case STRING:
    case TIMESTAMP:
    case INT2FLOAT:
    case ABS:
      return SPred();
    case NOT:
      Match(NOT);
      Match(LPAREN);
      left = Pred();
      Match(RPAREN);
      return new NOTPredicate(left); 
    case AND:
      Match(AND);
      Match(LPAREN);
      //left = SPred();
      left = Pred();		// tatbul@cs.brown.edu
      Match(COMMA);
      right = Pred();
      Match(RPAREN);
      return new ANDPredicate(left, right);
    case OR:
      Match(OR);
      Match(LPAREN);
      //left = SPred();
      left = Pred();		// tatbul@cs.brown.edu
      Match(COMMA);
      right = Pred();
      Match(RPAREN);
      return new ORPredicate(left, right);
    default:
      std::cout<<"non-predicate token read\n";
      exit(1);
    }
}


Predicate* Parse::SPred() 
{
  ExprInfo left;
  ExprInfo right;
  Predicate *pred;

  switch (_next_token) 
    {
    case IDENT:
    case LPAREN:
    case INT:
    case FLOAT:
    case STRING:
    case TIMESTAMP:
    case INT2FLOAT:
    case ABS:
      left = Expr();
      pred = RelOp(left);
      right = Expr();
      pred->setExpression(right.obj);
      return pred;
    default:
      std::cout<<"non-simple predicate token read\n";
      exit(1);
    }
}


Predicate* Parse::RelOp(ExprInfo expr) 
{
  switch (_next_token) 
    {
    case EQ:
      Match(EQ);
      switch (expr.type) 
	{ 
	case INT:
	  return new IntEqualPredicate(expr.obj);
	case FLOAT:
	  return new FloatEqualPredicate(expr.obj);
	case STRING:
	  return new StringEqualPredicate(expr.obj);
	case TIMESTAMP:
	  return new TsEqualPredicate(expr.obj);
	}
    case LT: 
      Match(LT);
      switch (expr.type) 
	{
	case INT:
	  return new IntLTPredicate(expr.obj);
	case FLOAT:
	  return new FloatLTPredicate(expr.obj);
	case TIMESTAMP:
	  return new TsLTPredicate(expr.obj);
	}
    case GT: 
      Match(GT);
      switch (expr.type) 
	{
	case INT:
	  return new IntGTPredicate(expr.obj);
	case FLOAT:
	  return new FloatGTPredicate(expr.obj);
	case TIMESTAMP:
	  return new TsGTPredicate(expr.obj);
	}
    case LTE:
      Match(LTE);
      switch (expr.type) 
	{
	case INT:
	  return new IntLTEPredicate(expr.obj);
	case FLOAT:
	  return new FloatLTEPredicate(expr.obj);
	case TIMESTAMP:
	  return new TsLTEPredicate(expr.obj);
	}
    case GTE: 
      Match(GTE);
      switch (expr.type) 
	{
	case INT:
	  return new IntGTEPredicate(expr.obj);
	case FLOAT:
	  return new FloatGTEPredicate(expr.obj);
	case TIMESTAMP:
	  return new TsGTEPredicate(expr.obj);
	}
    case NE: 
      Match(NE);
      switch (expr.type) 
	{
	case INT:
	  return new IntNEPredicate(expr.obj);
	case FLOAT:
	  return new FloatNEPredicate(expr.obj);
	case STRING:
	  return new StringNEPredicate(expr.obj);
	case TIMESTAMP:
	  return new TsNEPredicate(expr.obj);
	}
    default:
      std::cout<<"non-relational operator token read\n";
      exit(1);
    }
}


ExprInfo Parse::SExpr() 
{
  switch (_next_token) 
    {
    case IDENT: {
      IdentInfo id = MatchIdent();
      ExprInfo tmp = {_ident_type, MakeIdent(id)};
      return tmp;
    }
    case INT:
    case FLOAT:
    case STRING:
    case TIMESTAMP:
      return Const();
    case LPAREN: 
      {
	Match(LPAREN);
	ExprInfo e = Expr();
	Match(RPAREN);
	return e;
      }
    case INT2FLOAT: 
      {
	Match(INT2FLOAT);
	Match(LPAREN);
	ExprInfo tmp = {FLOAT, new Int2Float(Expr().obj)};
	Match(RPAREN);
	return tmp;
      }
    case ABS:
      {
	Match(ABS);
	Match(LPAREN);
	switch(_next_token)
	  {
	  case INT:
	    {
	      ExprInfo tmp = {INT, new IntAbsoluteFunction(Expr().obj)};
	      Match(RPAREN);
	      return tmp;
	    }
	  case FLOAT:
	    {
	      ExprInfo tmp = {FLOAT, new FloatAbsoluteFunction(Expr().obj)};
	      Match(RPAREN);
	      return tmp;
	  }
	  default:
	    std::cout<<"non-numerical token read\n";
	    exit(1);
	    
	  }
      }
    case FLOAT2INT: 
      {
	Match(FLOAT2INT);
	Match(LPAREN);
	ExprInfo tmp = {INT, new Float2Int(Expr().obj)};
	Match(RPAREN);
	return tmp;
      }
    default:
      std::cout<<"non-expression token read\n";
      exit(1);
    }
}


ExprInfo Parse::Expr() 
{
  switch (_next_token) 
    {
    case IDENT:
    case INT:
    case FLOAT:
    case STRING:
    case TIMESTAMP:
    case LPAREN: 
    case INT2FLOAT:
    case ABS:
    case FLOAT2INT:
      {
	return OptAr(SExpr());
      }
    default:
      std::cout<<"non-expression token read\n";
      exit(1);
    }
}


ExprInfo Parse::OptAr(ExprInfo e) 
{
  switch (_next_token) 
    {
    case PLUS:
    case MINUS:
    case TIMES:
    case DIVIDE: 
      {
	ExprInfo e1 = InfAr(e);
	ExprInfo e2 = Expr();
	e1.obj->setExpression(e2.obj);
	return e1;
      }
    default: 
      {
	return e;
      }
    }
}


ExprInfo Parse::InfAr(ExprInfo e) 
{
  switch (_next_token) 
    {
    case PLUS: 
      {
	switch(e.type) 
	  {
	  case INT: 
	    {
	      ExprInfo tmp = {INT, new IntAddFunction(e.obj)};
	      Match(PLUS);
	      return tmp;
	    }
	  case FLOAT: 
	    {
	      ExprInfo tmp = {FLOAT, new FloatAddFunction(e.obj)};
	      Match(PLUS);
	      return tmp;
	    }
	  }
      }
    case MINUS: 
      {
	switch(e.type) 
	  {
	  case INT: 
	    {
	      ExprInfo tmp = {INT, new IntSubtractFunction(e.obj)};
	      Match(MINUS);
	      return tmp;
	    }
	  case FLOAT:
	    {
	      ExprInfo tmp = {FLOAT, new FloatSubtractFunction(e.obj)};
	      Match(MINUS);
	      return tmp;
	    }
	  }
      }
  case TIMES: 
    {
      switch(e.type) 
	{
	case INT: 
	  {
	    ExprInfo tmp = {INT, new IntMultiplyFunction(e.obj)};
	    Match(TIMES);
	    return tmp;
	  }
	case FLOAT: {
	  ExprInfo tmp = {FLOAT, new FloatMultiplyFunction(e.obj)};
	  Match(TIMES);
	  return tmp;
	}
	}
    }
    case DIVIDE: 
      { 
	switch(e.type) 
	  {
	  case INT: 
	    {
	      ExprInfo tmp = {INT, new IntDivideFunction(e.obj)};
	      Match(DIVIDE);
	      return tmp;
	    }
	  case FLOAT: 
	    {
	      ExprInfo tmp = {FLOAT, new FloatDivideFunction(e.obj)};
	      Match(DIVIDE);
	      return tmp;
	    }
	  }
      }
    default:
      std::cout<<"non-primitive function token read\n";
      exit(1);
    }
}


ExprInfo Parse::Const() 
{
  switch (_next_token) 
    {
    case INT: 
      {
	ExprInfo tmp = {INT, new IntConstant(_curr_int)};
	Match(INT);
	return tmp;
      }
    case FLOAT: 
      {
	ExprInfo tmp = {FLOAT, new FloatConstant(_curr_float)};
	Match(FLOAT);
	return tmp;
      }
    case STRING: 
      {
	ExprInfo tmp = {STRING, new StringConstant(_curr_string)};
	Match(STRING);
	delete[] _curr_string;
	return tmp;
      }
    case TIMESTAMP:
      {
	ExprInfo tmp = {TIMESTAMP, new TsConstant(_curr_Ts)};
	Match(TIMESTAMP); 
	return tmp;
      }
    default: 
      {
	std::cout<<"non-Constant token read\n";
	exit(1);
      }
    }
}


void Parse::Match(token t) 
{
  if (_next_token == t)
    _next_token = nextToken();
  else 
    {
      std::cout<<"unexpected parameter to Match\n";
      exit(1);
    }
}


Expression* Parse::MakeIdent(IdentInfo id) 
{
  switch (id.type) 
    {
    case INT:
      return new FieldExt(id.port, 'i', id.offset, id.size);
    case FLOAT:
      return new FieldExt(id.port, 'f', id.offset, id.size);
    case STRING:
      return new FieldExt(id.port, 's', id.offset, id.size);
    case TIMESTAMP:
      return new FieldExt(id.port, 't', id.offset, id.size);
    }
}


IdentInfo Parse::MatchIdent() 
{
  token type;
  int port;
  int offset;
  int size;
  if (_next_token == IDENT) 
    {
      port = atoi(_curr_string);
      int x = 1;
      while (_curr_string[x] != ':') 
	x++;
      x++;
      switch (_curr_string[x]) 
	{
	case 'i':
	  type = INT;
	  break;
	case 'f':
	  type = FLOAT;
	  break;
	case 't':
	  type = TIMESTAMP;
	  break;
	case 's':
	  type = STRING;
	  break;
	default:
	  std::cout<<"offset type unknown\n";
	  exit(1);
	  break;
	} //end of switch
      
      x += 2;
      offset = atoi(_curr_string + x);
      while (_curr_string[x] != ':') 
	x++;
      x++;
      size = atoi(_curr_string + x);
      
      IdentInfo tmp = {type, port, offset, size};
      _ident_type = type;
      _next_token = nextToken();
      return tmp;
    }
  else 
    {
      std::cout<<"unexpected parameter to MatchIdent\n";
      exit(1);
    }
}

char Parse::currchar() 
{
  return _input_string[_curr_place];
}

char Parse::nextchar() 
{
  _curr_place++;
  return currchar();
}

void Parse::retract(int num) 
{
  _curr_place = _curr_place - num;
}

void Parse::install_int() 
{
  _curr_int = atoi(_input_string + _token_beginning);  
}

void Parse::install_float() 
{
  _curr_float = atof(_input_string + _token_beginning);  
}

void Parse::install_string() 
{
  _curr_string = new char[_curr_place - _token_beginning];
  strncpy(_curr_string, _input_string + _token_beginning + 1, _curr_place - _token_beginning - 1);
  _curr_string [_curr_place - _token_beginning - 1] = '\0';
}

//Need to write install_timestamp

int Parse::fail() 
{
  cout<<"Unexpected input to token"<<endl;
  exit(1);
}

token Parse::nextToken() 
{
  _state = 0;
  _token_beginning = _curr_place + 1;
  while (true) 
    {
      switch(_state) 
	{
	case 0: 
	  {
	    _c = nextchar();
	    //c is a lookahead character
	    // PORTABILITY FLAW: looking only for \n is not good
	    //   should also look for \r\n
	    //   but there shouldn't be any returns in the input string anayway
	    //   so I'm not too concerned
	    if ((_c == ' ') || (_c == '\t') || (_c == '\n')) 
	      {
		_state = 0;
		_token_beginning++;
	      }
	    else if (_c == '+') return (PLUS);
	    else if (_c == '-') return (MINUS);
	    else if (_c == '*') return (TIMES);
	    else if (_c == '/') return (DIVIDE);
	    else if (_c == '(') return (LPAREN);
	    else if (_c == ')') return (RPAREN);
	    else if (_c == ',') return (COMMA);
	    else if (_c == '<') _state = 1;
	    else if (_c == '=') _state = 5;
	    else if (_c == '!') _state = 41;
	    else if (_c == '>') _state = 6;
	    else if (_c == '\'') _state = 9;
	    else if (isdigit(_c)) _state = 12;
	    else if (isalpha(_c)) _state = 22;
	    else if (_c == ':') _state = 28;
	    else if (_c == '\0') _state = 32;
	    else _state = fail();
	    break;
	  }
	case 1: 
	  {
	    _c = nextchar();
	    if (_c == '=') _state = 2;
	    else if (_c == '>') _state = 3;
	    else _state = 4;
	    break;
	  }
	case 2:
	  return (LTE);
	case 3:
	  return (NE);
	case 4: 
	  {
	    retract(1);
	    return (LT);
	  }
	case 5:
	  return (EQ);
	case 6: 
	  {
	    _c = nextchar();
	    if (_c == '=') _state = 7;
	    else _state = 8;
	    break;
	  }
	case 7:
	  return (GTE);
	case 8: 
	  {
	    retract(1);
	    return (GT);
	  }
	case 9: 
	  {
	    _c = nextchar();
	    if (_c != '\'') _state = 10;
	    else _state = 11;
	    break;
	  }
	case 10: 
	  {
	    char temp = currchar();
	    _c = nextchar();
	    if (_c != '\'') _state = 10;
	    else if (temp == '\\') _state = 10;
	    else _state = 11;
	    break;
	  }
	case 11: 
	  {
	    install_string();
	    return (STRING);
	  }
	case 12: 
	  {
	    _c = nextchar();
	    if (isdigit(_c)) _state = 12;
	    else if (_c == '.') _state = 14;
	    else if (_c == 'E') _state = 16;
	    else _state = 21;
	    break;
	  }
	  //I deleted case 13 so for now there is none
	case 14: 
	  {
	    _c = nextchar();
	    if (isdigit(_c)) _state = 15;
	    else _state = fail();
	    break;
	  }
	case 15: 
	  {
	    _c = nextchar();
	    if (isdigit(_c)) _state = 15;
	    else if (_c == 'E') _state = 16;
	    else _state = 20;
	    break;
	  }
	case 16: 
	  {
	    if ((_c == '+') || (_c == '-')) _state = 17;
	    else if (isdigit(_c)) _state = 18;
	    else _state = fail();
	    break;
	  }
	case 17:
	  {
	    if (isdigit(_c)) _state = 18;
	    else _state = fail();
	    break;
	  }
	case 18:
	  {
	    if (isdigit(_c)) _state = 18;
	    else _state = 19;
	    break;
	  }
	case 19: 
	  {
	    retract(1);
	    install_float();
	    return (FLOAT);
	  }
	case 20:
	  {
	    retract(1);
	    install_float();
	    return(FLOAT);
	  }
	case 21: 
	  {
	    retract(1);
	    install_int();
	    return(INT);
	  }
	case 22:
	  {
	    _c = currchar();
	    if ((_c == 'A') || (_c == 'a')) _state = 23;
	    else if ((_c == 'O') || (_c == 'o')) _state = 25;
	    else if ((_c == 'N') || (_c == 'n')) _state = 26;
	    else if ((_c == 'I') || (_c == 'i')) _state = 33;
	    else if ((_c == 'F') || (_c == 'f')) _state = 43;
	    else _state = fail();
	    break;
	  }
	case 23: 
	  {
	    _c = nextchar();
	    if ((_c == 'N') || (_c == 'n')) _state = 24;
	    else if ((_c == 'B') || (_c == 'b')) _state = 42;
	    else _state = fail();
	    break;
	  }
	case 24: 
	  {
	    _c = nextchar();
	    if ((_c == 'D') || (_c == 'd'))
	      return (AND);
	    else _state = fail();
	    break;
	  }
	case 25:
	  {
	    _c = nextchar();
	    if ((_c == 'R') || (_c == 'r'))
	      return (OR);
	    else _state = fail();
	    break;
	  }
	case 26: 
	  {
	    _c = nextchar();
	    if ((_c == 'O') || (_c == 'o')) _state = 27;
	    else _state = fail();
	    break;
	  }
	case 27:
	  {
	    _c = nextchar();
	    if ((_c == 'T') || (_c == 't'))
	      return (NOT);
	    else _state = fail();
	    break;
	  }
	case 28:
	  {
	    _c = nextchar();
	    if (_c != ':') _state = 28;
	    else _state = 29;
	    break;
	  }
	case 29: 
	  {
	    _c = nextchar();
	    if (_c != ':') _state = 29;
	    else _state = 30;
	    break;
	  }
	case 30:
	  {
	    _c = nextchar();
	    if (_c != ':') _state = 30;
	    else _state = 31;
	    break;
	  }
	case 31:
	  _c = nextchar();
	  if (_c != ':') _state = 31;
	  else 
	    {
	      install_string();
	      return (IDENT);
	    }
	  break;
	case 32: 
	  {
	    return (OTHER);
	  }
	case 33:
	  {
	    _c = nextchar();
	    if ((_c == 'N') || (_c == 'n')) _state = 34;
	    else _state = fail();
	    break;
	  }
	case 34:
	  {
	    _c = nextchar();
	    if ((_c == 'T') || (_c == 't')) _state = 35;
	    else _state = fail();
	    break;
	  }
	case 35:
	  {
	    _c = nextchar();
	    if (_c == '2') _state = 36;
	    else _state = fail();
	    break;
	  }
	case 36:
	  {
	    _c = nextchar();
	    if ((_c == 'F') || (_c == 'f')) _state = 37;
	    else _state = fail();
	    break;
	  }
	case 37:
	  {
	    _c = nextchar();
	    if ((_c == 'L') || (_c == 'l')) _state = 38;
	    else _state = fail();
	    break;
	  }
	case 38:
	  {
	    _c = nextchar();
	    if ((_c == 'O') || (_c == 'o')) _state = 39;
	    else _state = fail();
	    break;
	  }
	case 39:
	  {
	    _c = nextchar();
	    if ((_c == 'A') || (_c == 'a')) _state = 40;
	    else _state = fail();
	    break;
	  }
	case 40:
	  {
	    _c = nextchar();
	    if ((_c == 'T') || (_c == 't'))
	      return (INT2FLOAT);
	    else _state = fail();
	    break;
	  }
	case 41: 
	  {
	    _c = nextchar();
	    if (_c == '=') _state = 3;
	    else _state = fail();
	    break;
	  }
	case 42:
	  {
	    _c = nextchar();
	    if ((_c == 'S') || (_c == 's')) 
	      return ABS;
	    else _state = fail();
	    break;
	  }
	case 43:
	  {
	    // FLOAT2INT
	    _c = nextchar();
	    if (!((_c == 'L') || (_c == 'l'))) {_state = fail(); break;}
	    _c = nextchar();
	    if (!((_c == 'O') || (_c == 'o'))) {_state = fail(); break;}
	    _c = nextchar();
	    if (!((_c == 'A') || (_c == 'a'))) {_state = fail(); break;}
	    _c = nextchar();
	    if (!((_c == 'T') || (_c == 't'))) {_state = fail(); break;}
	    _c = nextchar();
	    if (!((_c == '2') || (_c == '2'))) {_state = fail(); break;}
	    _c = nextchar();
	    if (!((_c == 'I') || (_c == 'i'))) {_state = fail(); break;}
	    _c = nextchar();
	    if (!((_c == 'N') || (_c == 'n'))) {_state = fail(); break;}
	    _c = nextchar();
	    if (!((_c == 'T') || (_c == 't'))) {_state = fail(); break;}
	    return (FLOAT2INT);
	    break;
	  }
	  //Need to add checking for Timestamp
	}
    }
}
