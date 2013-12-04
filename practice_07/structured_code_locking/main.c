#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include "linked_list.h"

#define PUTTERS_NUM 10
#define TAKERS_NUM 10

int print_entry (void *, void *);
int delete_entry (void *, void *);

void put (void *);
void take (void *);

int main (void) {
  struct list *list;
  struct entry *entry;
  pthread_t putters[PUTTERS_NUM];
  pthread_t takers[TAKERS_NUM];
  int i;

  list = list_init();

  for (i = 0; i < TAKERS_NUM; i++) {
    pthread_create(&takers[i], NULL, (void *)take, (void *)list);
    pthread_create(&putters[i], NULL, (void *)put, (void *)list);
  }
  for (i = 0; i < PUTTERS_NUM; i++) {
    pthread_join(putters[i], NULL);
    pthread_join(takers[i], NULL);
  }
  for (i = 0; i < TAKERS_NUM; i++) {
    pthread_join(takers[i], NULL);
  }

  printf("items remained on the list:\n");
  list_traverse(list, print_entry, NULL);

  free(list);
  return (0);
} 

int print_entry (void *e, void *u) {
  printf("%s\n", (char *)e);
  return (0);
}

int delete_entry (void *e, void *u) {
  char *c1 = e, *c2 = u;
  return (!strcmp(c1, c2));
}

void put (void *_list) {
  char *str = malloc(sizeof(char) * 32);

  snprintf(str, 32, "%d", (int)pthread_self());

  list_enqueue((struct list *)_list, (void*)str);
}

void take (void *_list) {
  struct entry *entry = list_dequeue((struct list *)_list);
  if (entry != NULL) {
    free(entry->data);
    free(entry);
  }
}

