#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "TupleDescription.H"


using namespace std;

void fillWithInt(char *p);
void fillWithFloat(char *p);
void fillWithTimestamp(char *p);
void printInt(const char* p);
void printFloat(const char* p);
char* generateTuple(const char* format);
char* generateTuple(TupleDescription* td);
void printTuple(const char* t, const char* format);
void printTuple(const char* t, TupleDescription* td);
