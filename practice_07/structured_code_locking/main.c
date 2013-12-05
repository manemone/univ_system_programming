#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include "linked_list.h"

#define PUTTERS_NUM 10
#define TAKERS_NUM 10
#define MESSAGE_LENGTH 32

// エンキュー／デキュースレッドに渡す構造体
typedef struct {
  struct list *list;
  void *data;
} ENTRY_PACK;

int print_entry (void *, void *);
int delete_entry (void *, void *);

void put (void *);
void take (void *);

int main (void) {
  struct list *list;
  struct entry *entry;
  pthread_t putters[PUTTERS_NUM];
  pthread_t takers[TAKERS_NUM];

  // エンキュースレッドに渡すデータ
  char **messages = (char **)malloc(sizeof(char **)*PUTTERS_NUM);
  ENTRY_PACK entry_packs[PUTTERS_NUM];

  int i;

  list = list_init();

  // エンキュースレッドに渡すデータを生成。
  // スレッド起動ループ内でひとつずつ作成せず事前に作成するのは
  // スレッド起動ループ 1回のコストを抑えて、各スレッドの起動をなるべく同時にしたいから。
  for (i = 0; i < PUTTERS_NUM; i++) {
    messages[i] = (char *)malloc(sizeof(char *)*MESSAGE_LENGTH);
    snprintf(messages[i], MESSAGE_LENGTH, "entry [%d]", i);
    entry_packs[i].list = list;
    entry_packs[i].data = messages[i];
  }

  // スレッドの起動
  printf("creating putters and takers...\n");
  for (i = 0; i < TAKERS_NUM; i++) {
    pthread_create(&takers[i], NULL, (void *)take, (void *)list);
    pthread_create(&putters[i], NULL, (void *)put, (void *)&entry_packs[i]);
  }

  // スレッドの待ち合わせ
  for (i = 0; i < PUTTERS_NUM; i++) {
    pthread_join(putters[i], NULL);
  }
  for (i = 0; i < TAKERS_NUM; i++) {
    pthread_join(takers[i], NULL);
  }

  // リストに残った要素を表示
  printf("\nentries remain on the list:\n");
  list_traverse(list, print_entry, NULL);

  // リストに残った要素を削除
  printf("\ndeque/free-ing remaining entries...\n");
  while((entry = list_dequeue(list)) != NULL) {
    free(entry->data);
    free(entry);
  }

  free(list);
  return (0);
} 

int print_entry (void *e, void *u) {
  printf("* %s\n", (char *)e);
  return (0);
}

int delete_entry (void *e, void *u) {
  char *c1 = e, *c2 = u;
  return (!strcmp(c1, c2));
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

