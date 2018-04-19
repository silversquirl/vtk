#ifdef DEBUG
#include <stdio.h>
#undef DEBUG
#define DEBUG(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)
#else
#define DEBUG(...)
#endif
