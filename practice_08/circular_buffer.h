#include <pthread.h>

#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#define QSIZE 100 /* キューの長さ */

typedef struct {
  pthread_mutex_t buf_lock; /* 構造体のロック */
  int start;                /* バッファの開始 */
  int num_full;             /* データの数 */
  pthread_cond_t notfull;   /* notfullの条件変数 */
  pthread_cond_t notempty;  /* notemptyの条件変数 */
  void *data[QSIZE];        /* 巡回バッファ */
} circ_buf_t;

#endif
