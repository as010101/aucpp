#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "Parse.H"
#include "FunPred.H"
#include "tupleGenerator.H"

using namespace std;

//char* s1 = "AND(:0:i:5:3: >= 34, :0:i:2:3: + 4.0 < 7)";
//char* s2 = "AND(:0:i:5:3: <> 34, :0:f:2:3: * 4.0 > 7.0)";
//char* s3 = "AND(:0:i:5:3: > 34, AND(:0:f:2:3: * :0:i:5:3: / 5 < 75.0, :0:s:12:2: = 'as'))";
//char* s4 = ":0:s:0:2: <> 'as'";

const char* s = "AND(:0:i:0:4: > 10, AND(:0:f:4:4: * :0:f:8:4: / 5000.0  < 250.0, :0:s:12:2: = 'bb'))";

const char* s1 = "AND(:0:f:4:4: * :0:f:8:4: / 5000.0 < 9999999.0, :0:s:12:2: = 'bb')";
const char* ls = ":0:f:4:4: * :0:f:8:4: / 5000.0 < 9999999.0";
const char* rs = ":0:s:12:2: = 'bb'";
const char* desc = "iffcc";

const char* a = " :0:i:0:4: = 167";

int main () {
  Parse* p = new Parse();
  Predicate* pred;
  char* t;


  pred = p->parsePred(a);

  for (int i = 0; i < 7; i++) {
    t = generateTuple(desc);
    printTuple(t,desc);
    printf("Evaluate returns for tuple above: %d\n\n", pred->evaluate(t));
    printf("\n");
  }

}

