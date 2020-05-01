#include <stdio.h>
#include <time.h>

/* This program computes and prints the following for a specific platform:
 * - the number of bits in the type clock_t
 * - the maximum value that can be returned by clock()
 * - the value of CLOCKS_PER_SEC
 * - the maximum number of seconds that can be recorded as elapsed time using clock()
 * - the minimum elapsed time detected by clock()
 */
main() {
  clock_t tv, tv2;
  int i, min, diff, bits;
  bits = sizeof(tv) << 3;
  tv = (clock_t) -1 ^ (1 << (bits - 1));
  printf("Number of bits in clock_t: %d\n", bits);
  printf("Maximum clock_t: %d\n", tv);
  printf("CLOCKS_PER_SEC: %d\n", CLOCKS_PER_SEC);
  printf("Maximum computable elapsed time: %d seconds.\n", tv / CLOCKS_PER_SEC);
  min = tv;
  for (i = 1; i < 10; i++) {
    tv = clock();
    while ((tv2 = clock()) == tv);
    diff = tv2 - tv;
    tv = tv2;
    min = diff < min ? diff : min;
  }
  printf("Minimum detected elapsed time: %f seconds.\n", 
	 (double) min / (double) CLOCKS_PER_SEC);
}
