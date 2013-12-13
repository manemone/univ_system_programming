#include <stdio.h>
#include <pthread.h>
#include "circular_buffer.h"
#include "echo_reply.h"

// 巡回バッファのクリーンアップハンドラ。
// ミューテクスをアンロックしておく。
void cleanup_cbp_lock (void *_cbp) {
  circ_buf_t *cbp = (circ_buf_t *)_cbp; 

  pthread_mutex_unlock(&cbp->buf_lock);
}

void put_cb_data(circ_buf_t *cbp, void *data) {
  pthread_mutex_lock(&cbp->buf_lock);
  pthread_cleanup_push(cleanup_cbp_lock, cbp);

  /* wait while the buffer is full */
  while (cbp->num_full == QSIZE) {
    pthread_cond_wait(&cbp->notfull, &cbp->buf_lock);
  }
  cbp->data[(cbp->start + cbp->num_full) % QSIZE] = data;
  cbp->num_full++;
  
  /* let a waiting reader know there's data */
  pthread_cond_signal(&cbp->notempty);
  pthread_cleanup_pop(1);
}

void *get_cb_data(circ_buf_t *cbp) {
  void *data;

  pthread_mutex_lock(&cbp->buf_lock);
  pthread_cleanup_push(cleanup_cbp_lock, cbp);

  /* wait while there's nothing in the buffer */
  while (cbp->num_full == 0) {
    pthread_cond_wait(&cbp->notempty, &cbp->buf_lock);
  }
  data = cbp->data[cbp->start];
  cbp->start = (cbp->start + 1) % QSIZE;
  cbp->num_full--;
  
  /* let a waiting writer know there's room */
  pthread_cond_signal(&cbp->notfull);
  pthread_cleanup_pop(1);
  return (data);
}

