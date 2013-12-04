#include <pthread.h>
#include <stdlib.h>
#include "linked_list.h"

// リストへの mutex
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

struct list *list_init (void) {
  struct list *list;

  list = malloc(sizeof *list);
  if (list == NULL)
    return (NULL);
  list->head = NULL;
  list->tail = &list->head;
  return (list);
}

int list_enqueue (struct list *list, void *data) {
  struct entry *e;

  e = malloc(sizeof *e);
  if (e == NULL)
    return (1);

  e->next = NULL;
  e->data = data;

  pthread_mutex_lock(&lock);

  *list->tail = e;
  list->tail = &e->next;
  printf("+ %s\n", (char *)data);
  
  pthread_mutex_unlock(&lock);

  return (0);
}

struct entry *list_dequeue (struct list *list) {
  struct entry *e;
  struct entry *ret;

  ret = NULL;

  pthread_mutex_lock(&lock);

  if (list->head == NULL) {
    printf("- NULL!\n");
    goto finish;
  }
  e = list->head;
  list->head = e->next;
  if (list->head == NULL) {
    list->tail = &list->head;
  }
   printf("- %s\n", (char *)e->data);
  ret = e;

finish:
  pthread_mutex_unlock(&lock);
  return ret;
}

struct entry *list_traverse (struct list *list, int (*func)(void *, void *), void *user) {
  struct entry **prev, *n, *next;
  struct entry *ret;

  pthread_mutex_lock(&lock);

  if (list == NULL) {
    ret = NULL;
    goto finish;
  }

  prev = &list->head;
  for (n = list->head; n != NULL; n = next) {
    next = n->next;
    switch (func(n->data, user)) {
      case 0:
        /* continues */
        prev = &n->next;
        break;
      case 1:
        /* delete the entry */
        *prev = next;
        if (next == NULL)
          list->tail = prev;
        ret = n;
        goto finish;
      case -1:
      default:
        /* traversal stops */
        ret = NULL;
        goto finish;
    }
  }
  ret = NULL;

finish:
  pthread_mutex_unlock(&lock);
  return ret;
}

