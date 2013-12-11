#include <stdio.h>
#include <pthread.h>
#include "circular_buffer.h"
#include "echo_reply.h"

void put_cb_data(circ_buf_t *cbp, void *data) {
  pthread_mutex_lock(&cbp->buf_lock);
  /* wait while the buffer is full */
  while (cbp->num_full == QSIZE) {
    printf("WAITING FOR DEQUEUEING...\n");
    pthread_cond_wait(&cbp->notfull, &cbp->buf_lock);
  }
  cbp->data[(cbp->start + cbp->num_full) % QSIZE] = data;
  cbp->num_full++;
  
  /* DEBUG */
  printf("+\n");
  if (cbp->num_full == QSIZE) {
    printf("FULL.\n\n");
  }

  /* let a waiting reader know there's data */
  pthread_cond_signal(&cbp->notempty);
  pthread_mutex_unlock(&cbp->buf_lock);
}

void *get_cb_data(circ_buf_t *cbp) {
  void *data;

  pthread_mutex_lock(&cbp->buf_lock);
  /* wait while there's nothing in the buffer */
  while (cbp->num_full == 0) {
    printf("WAITING FOR ENQUEUEING...\n");
    pthread_cond_wait(&cbp->notempty, &cbp->buf_lock);
  }
  data = cbp->data[cbp->start];
  cbp->start = (cbp->start + 1) % QSIZE;
  cbp->num_full--;
  
  /* DEBUG */
  printf("-\n");
  if (cbp->num_full == 0) {
    printf("EMPTY.\n\n");
  }

  /* let a waiting writer know there's room */
  pthread_cond_signal(&cbp->notfull);
  pthread_mutex_unlock(&cbp->buf_lock);
  return (data);
}


