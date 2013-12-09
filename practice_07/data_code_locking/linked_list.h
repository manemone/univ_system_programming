#include <pthread.h>

#ifndef LINKED_LIST_H
#define LINKED_LIST_H

struct entry {
  struct entry *next;
  void *data;
};

struct list {
  struct entry *head;
  struct entry **tail;
  pthread_mutex_t lock;
};

extern struct list *list_init (void);
extern int list_enqueue (struct list *, void *);
extern struct entry *list_dequeue (struct list *);
extern struct entry *list_traverse (struct list *, int (*func)(void *, void *), void *);

#endif
