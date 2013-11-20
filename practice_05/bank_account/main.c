#include <stdio.h>
#include <pthread.h>
#include "banking.h"

#define OPERATOR_NUM 3

void save (void*);
void waste (void*);

int main (void) {
  pthread_t savers[OPERATOR_NUM];
  pthread_t wasters[OPERATOR_NUM];
  int i;

  for (i = 0; i < OPERATOR_NUM; i++) {
    pthread_create(&savers[i], NULL, (void *)save, (void *)i);
    pthread_create(&wasters[i], NULL, (void *)waste, (void *)i);
  }

  for (i = 0; i < OPERATOR_NUM; i++) {
    pthread_join(savers[i], NULL);
    pthread_join(wasters[i], NULL);
  }

  printf("----\n");
  printf("$ %d lasted.\n", get_balance());

  return 0;
}

/**
 * お金を貯める
 **/
void save (void *_no) {
  int no = (int)_no;
  int value = 500;

  printf("+ %d, saver %d\n", value, no);
  deposit(value);
}

/**
 * お金を使う
 **/
void waste (void *_no) {
  int no = (int)_no;
  int value = 510;
  int succeeded;

  printf("- %d, waster %d\n", value, no);
  succeeded = withdraw(value);

  if (succeeded == 0) {
    printf("! no enough cash!: waster %d\n", no);
  }
}
