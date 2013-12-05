#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
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

// スレッド
void put (void *);
void take (void *);
void block (void *);

int main (void) {
  struct list *list_a, *list_b;
  THREAD_SET putter_set_a = {length: PUTTERS_NUM};
  THREAD_SET taker_set_a = {length: TAKERS_NUM};

  THREAD_SET putter_set_b = {length: PUTTERS_NUM};
  THREAD_SET taker_set_b = {length: TAKERS_NUM};

  pthread_t list_a_blocker;

  list_a = list_init();
  list_b = list_init();

  pthread_create(&list_a_blocker, NULL, (void *)block, (void *)list_a);
  pthread_detach(list_a_blocker);

  create_test_threads(list_a, "A", &putter_set_a, &taker_set_a);
  create_test_threads(list_b, "B", &putter_set_b, &taker_set_b);

  join_test_threads(&putter_set_a, &taker_set_a);
  join_test_threads(&putter_set_b, &taker_set_b);

  settle_list(list_a, "A");
  settle_list(list_b, "B");

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

void block (void *_list) {
  struct list *list = (struct list *)_list;
  pthread_mutex_lock(&list->lock);
  printf("BLOCKING LIST!\n");
  usleep(1*1000*1000);
  pthread_mutex_unlock(&list->lock);
}

int print_entry (void *e, void *u) {
  printf("* %s\n", (char *)e);
  return (0);
}

int delete_entry (void *e, void *u) {
  char *c1 = e, *c2 = u;
  return (!strcmp(c1, c2));
}

