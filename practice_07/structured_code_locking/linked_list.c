#include <pthread.h>
#include <stdlib.h>
#include <stdarg.h>
#include "linked_list.h"

#define C_RED "\x1b[31m"
#define C_GREEN "\x1b[32m"
#define C_YELLOW "\x1b[33m"
#define C_DEFAULT "\x1b[39m"

#define C_LOCK_INITIALIZED 1

// 色付き文字を出力
void printf_with_colors(char *, char *, char *, ...);

// pthread_mutex_init(c_lock, NULL);


struct list *list_init (void) {
  struct list *list;

  if (c_lock_initialized != C_LOCK_INITIALIZED) {
    pthread_mutex_init(&c_lock, NULL);
  }

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

  pthread_mutex_lock(&c_lock);

  *list->tail = e;
  list->tail = &e->next;
  printf_with_colors(C_GREEN, C_DEFAULT, "+ %s\n", (char *)data);
  
  pthread_mutex_unlock(&c_lock);

  return (0);
}

struct entry *list_dequeue (struct list *list) {
  struct entry *e;
  struct entry *ret;

  ret = NULL;

  pthread_mutex_lock(&c_lock);

  if (list->head == NULL) {
    printf_with_colors(C_YELLOW, C_DEFAULT, "- failed. the list is empty.\n");
    goto finish;
  }
  e = list->head;
  list->head = e->next;
  if (list->head == NULL) {
    list->tail = &list->head;
  }
   printf_with_colors(C_RED, C_DEFAULT, "- %s\n", (char *)e->data);
  ret = e;

finish:
  pthread_mutex_unlock(&c_lock);
  return ret;
}

struct entry *list_traverse (struct list *list, int (*func)(void *, void *), void *user) {
  struct entry **prev, *n, *next;
  struct entry *ret;

  pthread_mutex_lock(&c_lock);

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
  pthread_mutex_unlock(&c_lock);
  return ret;
}

void printf_with_colors(char *color, char *returning_color, char *format, ...) {
  va_list args;

  printf("%s", color);
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
  printf("%s", returning_color);
}
