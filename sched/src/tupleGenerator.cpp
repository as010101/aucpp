#include "tupleGenerator.H"

void fillWithInt(char *p) {
  int i = rand() % 4;
  *((int*)p) = i;
}


void fillWithTimestamp(char *p) {

  timeval t;
  gettimeofday(&t, NULL);
  *((timeval*)p) = t;
  
  /**
   // Deprecated -- avoid (it should be a timeval, not a time_t
   // In the meantime, just make two ints (timeval = two ints for us)
   time_t t;
   time (&t);
   
   printf("[tupleGenerator] WARNING: fillWithTimestamp is deprecated (not updated to timeval)\n");
   *((time_t*)p) = t;
   */
}
void fillWithFloat(char *p) {
  int i = rand() % 4;
  int j = 1 + (int) (10.0*rand() / (RAND_MAX+1.0));
  float d = i + (float) (j / 100.0);
  
  *((float*)p) = d;
}
void fillWithChar(char *p) {
  char c;
  c = (char) (rand() % 4) + 97; // a through d
  *(p) = c;
}

void printInt(const char* p) {
  printf("%d",*((const int*)p));
}
void printFloat(const char* p) {
  printf("%f",*((const float*)p));
}
void printChar(const char* p) {
  printf("%c",*(p));
}
// a timeval is printed as two ints(longs really)
void printTimestamp(const char* p) {
  printInt(p);
  printf("][");
  printInt(p+sizeof(long));
}

char* generateTuple(const char* format) {
  int memsize = 0;
  // Find out the memory we need
  for (int i = 0; format[i] != '\0'; i++) {
    if (format[i] == 'i') memsize += sizeof(int);
    else if (format[i] == 'f') memsize += sizeof(float);
    else if (format[i] == 'c') memsize += sizeof(char);
    else if (format[i] == 't') memsize += sizeof(timeval);
    else {
      printf("generateTuple: Unexpected format character [%c]!\n", format[i]);
      abort();
    }
  }
  // Now get memory
  char* t = (char*) malloc(memsize);
  // Now actually create the stuff inside
  int offset = 0;
  for (int i = 0; format[i] != '\0'; i++) {
    if (format[i] == 'i') { fillWithInt(t + offset); offset += sizeof(int); }
    else if (format[i] == 'f') { fillWithFloat(t + offset); offset += sizeof(float); }
    else if (format[i] == 'c') { fillWithChar(t + offset); offset += sizeof(char); }
    else if (format[i] == 't') { fillWithTimestamp(t + offset); offset += sizeof(timeval); }
    else {
      printf("generateTuple: Unexpected format character [%c]!\n", format[i]);
      abort();
    }
  }

  return t;
}

void printTuple(const char* t, TupleDescription* td) {
	bool inString = false;
	printf("Tuple: ");
	int offset = 0;
	// First and foremost, the timestamp and sid
	printf("[");
	printTimestamp(t + offset); offset += sizeof(timeval);
	printf("][");
	printInt(t+offset); offset += sizeof(int);
	printf("]");
	// The fields
	for (int i = 0; i < td->getNumOfFields(); i++) {
		if (!inString) printf("[");
		if (td->getFieldType(i) == INT_TYPE) { 
			if (inString) { 
				printf("]"); 
				inString = false; 
				printf("["); 
			} 
				
			printInt(t + offset); offset += sizeof(int); printf("]");
		}
		else if (td->getFieldType(i) == FLOAT_TYPE || 
				 td->getFieldType(i) == DOUBLE_TYPE) { 
			if (inString) { 
				printf("]"); 
				inString = false; 
				printf("[");
			} 

			printFloat(t + offset); 
			offset += sizeof(float); 
			printf("]");
		}
		else if (td->getFieldType(i) == STRING_TYPE) {
			inString = true;
			for (int j = 0; j < td->getFieldSize(i); j++) { 
				printChar(t + offset); 
				offset += sizeof(char); 
			}
		}
		else if (td->getFieldType(i) == TIMESTAMP_TYPE) {
			if (inString) { 
				printf("]"); 
				inString = false; 
				printf("[");
			} 

			printTimestamp(t + offset); 
			offset += sizeof(timeval); 
			printf("]");
		}
		else {
			printf("generateTuple: Unexpected type [%d]!\n", td->getFieldType(i));
			abort();
		}
	}
	if (inString) printf("]");
	printf("\n");
}
void printTuple(const char* t, const char* format) {
  bool inString = false;
  printf("Tuple [%s]: ",format);
  int offset = 0;
  for (int i = 0; format[i] != '\0'; i++) {
    if (!inString) printf("[");
    if (format[i] == 'i') { if (inString) { printf("]"); inString = false; printf("["); } printInt(t + offset); offset += sizeof(int); printf("]");}
    else if (format[i] == 'f') { if (inString) { printf("]"); inString = false; printf("[");} printFloat(t + offset); offset += sizeof(float); printf("]");}
    else if (format[i] == 'c') { printChar(t + offset); offset += sizeof(char); inString = true; }
    else if (format[i] == 't') { if (inString) { printf("]"); inString = false; printf("[");} printTimestamp(t + offset); offset += sizeof(timeval); printf("]");}
    else {
      printf("generateTuple: Unexpected format character [%c]!\n", format[i]);
      abort();
    }
  }
  if (inString) printf("]");
  printf("\n");

}

// This version will generate a tuple to fit the description passed
char* generateTuple(TupleDescription *td) {
  int memsize = 0;
  // Find out the memory we need (you could do td->getSize() you know?)
  // NOTE: TupleDescription does NOT INCLUDE: Timestamp (timeval) and sid (int)
  memsize += sizeof(timeval) + sizeof(int);
  for (int i = 0; i < td->getNumOfFields(); i++) {
    if (td->getFieldType(i) == INT_TYPE) memsize += sizeof(int);
    else if (td->getFieldType(i) == FLOAT_TYPE) memsize += sizeof(float);
    else if (td->getFieldType(i) == DOUBLE_TYPE) memsize += sizeof(float); // euh.. hope double == float
    else if (td->getFieldType(i) == STRING_TYPE) memsize += sizeof(char) * td->getFieldSize(i);
	else if (td->getFieldType(i) == TIMESTAMP_TYPE) memsize += sizeof(timeval);
     else {
      printf("generateTuple: Unexpected type [%d]!\n", td->getFieldType(i));
      abort();
    }
    
  }
  
  // Now get memory
  char* t = (char*) malloc(memsize);
  // Now actually create the stuff inside
  int offset = 0;
  // Timestamp
  fillWithTimestamp(t+offset);
  offset += sizeof(timeval);
  // Sid - hard code to 1
  *((int*)t+offset) = 1;
  // The fields now
  
  for (int i = 0; i < td->getNumOfFields(); i++) {
    if (td->getFieldType(i) == INT_TYPE) { fillWithInt(t+offset); offset += sizeof(int); }
    else if (td->getFieldType(i) == FLOAT_TYPE) { fillWithFloat(t+offset); offset += sizeof(float); }
    else if (td->getFieldType(i) == DOUBLE_TYPE) { fillWithFloat(t+offset); offset += sizeof(float); }
    else if (td->getFieldType(i) == STRING_TYPE) {
      for (int j = 0; j < td->getFieldSize(i); j++) { fillWithChar(t+offset); offset+= sizeof(char); }
    }
	else if (td->getFieldType(i) == TIMESTAMP_TYPE) { fillWithTimestamp(t+offset); offset += sizeof(timeval);}
    else {
      printf("generateTuple: Unexpected type [%d]!\n", td->getFieldType(i));
      abort();
    }
    
  }

  return t;
}
