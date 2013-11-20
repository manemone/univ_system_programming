#ifndef ALLOC3_H
#define ALLOC3_H

#define ALLOC_UNIT (64 * 1024)

extern void *alloc3(int n);
extern void afree3(void *p);

#endif

