#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define QSIZE 8 /* キューの長さ */

// エンキューのスレッド数とスレッド内でのエンキュー数
#define ENQUEUE_UNIT 3
#define ENQUEUEROR_NUM 5

// デキューのスレッド数とスレッド内でのデキュー数
#define DEQUEUE_UNIT 5 
#define DEQUEUEROR_NUM 3

typedef struct {
  pthread_mutex_t buf_lock; /* 構造体のロック */
  int start;                /* バッファの開始 */
  int num_full;             /* データの数 */
  pthread_cond_t notfull;   /* notfullの条件変数 */
  pthread_cond_t notempty;  /* notemptyの条件変数 */
  void *data[QSIZE];        /* 巡回バッファ */
} circ_buf_t;

circ_buf_t *buffer;

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

  /* let a waiting reader know there’s data */
  pthread_cond_signal(&cbp->notempty);
  pthread_mutex_unlock(&cbp->buf_lock);
}

void *get_cb_data(circ_buf_t *cbp) {
  void *data;
  pthread_mutex_lock(&cbp->buf_lock);
  /* wait while there’s nothing in the buffer */
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

  /* let a waiting writer know there’s room */
  pthread_cond_signal(&cbp->notfull);
  pthread_mutex_unlock(&cbp->buf_lock);
  return (data);
}

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
