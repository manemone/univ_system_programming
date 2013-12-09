#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "circular_buffer.h"

circ_buf_t *buffer;

/* エンキューのスレッド */
void enqueue (void *_no) {
  int no = (int)_no;
  int i;

  for (i = 0; i < ENQUEUE_UNIT; i++) {
    put_cb_data(buffer, (void *)no);
  }
}

/* デキューのスレッド */
void dequeue (void *_no) {
  int no = (int)_no;
  int i;

  for (i = 0; i < DEQUEUE_UNIT; i++) {
    get_cb_data(buffer);
  }
}

int main (void) {
  pthread_t enquerors[ENQUEUEROR_NUM];
  pthread_t dequerors[DEQUEUEROR_NUM];
  int i;

  // initializing circular buffer
  buffer = (circ_buf_t *)malloc(sizeof(circ_buf_t));
  pthread_mutex_init(&buffer->buf_lock, NULL);
  buffer->start = 0;
  buffer->num_full = 0;
  pthread_cond_init(&buffer->notfull, NULL);
  pthread_cond_init(&buffer->notempty, NULL);

  for (i = 0; i < ENQUEUEROR_NUM; i++) {
    pthread_create(&enquerors[i], NULL, (void *)enqueue, (void *)i);
  }
  for (i = 0; i < DEQUEUEROR_NUM; i++) {
    pthread_create(&dequerors[i], NULL, (void *)dequeue, (void *)i);
  }

  for (i = 0; i < ENQUEUEROR_NUM; i++) {
    pthread_join(enquerors[i], NULL);
  }
  for (i = 0; i < DEQUEUEROR_NUM; i++) {
    pthread_join(dequerors[i], NULL);
  }
}


