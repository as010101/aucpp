#ifndef sm_constants_H
#define sm_constants_H

// We make this one bytes smaller than 2GB, because of a very stupid reason...
// fstat(...) populates a 'struct stat' structure, which in turn uses
// an 'off_t' to represent a file size. 'off_t' is a signed long, which
// means it's not going to be able to store +2GB on an Intel-based
// machine. It maxes out at 2^32 - 1. Idiots. They should have used
// size_t, not off_t.
// So... even though Linux thinks it can have 2GB files, fstat(...) doesn't
// support them. So we set this value to one number lower than we want to.
#define SM_MAX_FILE_BYTES 2147483647UL

#define SM_DATA_FILE_GROWTH_FACTOR 0.1

// The maximum fill fraction that a PageDir has before a growth of its
// storage is triggered.
#define SM_PAGE_DIR_GROWTH_TRIGGER 0.9

#endif
