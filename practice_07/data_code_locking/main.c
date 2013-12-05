#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include "linked_list.h"

#define PUTTERS_NUM 10
#define TAKERS_NUM 10
#define MESSAGE_LENGTH 32

// テスト用スレッドの組
typedef struct {
  pthread_t *threads;
  int length;
} THREAD_SET;

// エンキュー／デキュースレッドに渡す構造体
typedef struct {
  struct list *list;
  void *data;
} ENTRY_PACK;

int print_entry (void *, void *);
int delete_entry (void *, void *);

// スレッドの管理
void create_test_threads(struct list *, char *, THREAD_SET *, THREAD_SET *);
void join_test_threads(THREAD_SET *, THREAD_SET *);

// リストの後始末
void settle_list(struct list *, char *);

void put (void *);
void take (void *);

int main (void) {
  struct list *list;
  THREAD_SET putter_set = {length: PUTTERS_NUM};
  THREAD_SET taker_set = {length: TAKERS_NUM};

  list = list_init();

  create_test_threads(list, "A", &putter_set, &taker_set);
  join_test_threads(&putter_set, &taker_set);

  settle_list(list, "A");

  return (0);
} 

void create_test_threads(struct list *list, char *name, THREAD_SET *putter_set, THREAD_SET *taker_set) {
  struct entry *entry;
  int i;

  // エンキュースレッドに渡すデータ
  char **messages = (char **)malloc(sizeof(char *)*putter_set->length);
  ENTRY_PACK *entry_packs = (ENTRY_PACK *)malloc(sizeof(ENTRY_PACK)*putter_set->length);

  // エンキュースレッドに渡すデータを生成。
  // スレッド起動ループ内でひとつずつ作成せず事前に作成するのは
  // スレッド起動ループ 1回のコストを抑えて、各スレッドの起動をなるべく同時にしたいから。
  for (i = 0; i < putter_set->length; i++) {
    messages[i] = (char *)malloc(sizeof(char)*MESSAGE_LENGTH);
    snprintf(messages[i], MESSAGE_LENGTH, "list: %s, entry [%d]", name, i);
    entry_packs[i].list = list;
    entry_packs[i].data = messages[i];
  }

  // スレッドの起動
  putter_set->threads = (pthread_t *)malloc(sizeof(pthread_t)*putter_set->length);
  taker_set->threads = (pthread_t *)malloc(sizeof(pthread_t)*taker_set->length);
  for (i = 0; i < putter_set->length || i < taker_set->length; i++) {
    if (i < taker_set->length) {
      pthread_create(&taker_set->threads[i], NULL, (void *)take, (void *)list);
    }
    if (i < putter_set->length) {
      pthread_create(&putter_set->threads[i], NULL, (void *)put, (void *)&entry_packs[i]);
    }
  }
}

void join_test_threads(THREAD_SET *putter_set, THREAD_SET *taker_set) {
  int i;

  for (i = 0; i < putter_set->length; i++) {
    pthread_join(putter_set->threads[i], NULL);
  }
  for (i = 0; i < taker_set->length; i++) {
    pthread_join(taker_set->threads[i], NULL);
  }

  // スレッド変数の free
  free(putter_set->threads);
  free(taker_set->threads);
}

void settle_list(struct list *list, char *name) {
  struct entry *entry;

  // リストに残った要素を表示
  printf("\nentries remain on the list %s:\n", name);
  list_traverse(list, print_entry, NULL);

  // リストに残った要素を削除
  printf("\ndeque/free-ing remaining entries in the list %s...\n", name);
  while((entry = list_dequeue(list)) != NULL) {
    free(entry->data);
    free(entry);
  }

  free(list);
}

void put (void *_entry_pack) {
  ENTRY_PACK *entry_pack = (ENTRY_PACK *)_entry_pack;

  list_enqueue(entry_pack->list, entry_pack->data);
}

void take (void *_list) {
  struct entry *entry = list_dequeue((struct list *)_list);

  if (entry != NULL) {
    free(entry->data);
    free(entry);
  }
}

int print_entry (void *e, void *u) {
  printf("* %s\n", (char *)e);
  return (0);
}

int delete_entry (void *e, void *u) {
  char *c1 = e, *c2 = u;
  return (!strcmp(c1, c2));
}

