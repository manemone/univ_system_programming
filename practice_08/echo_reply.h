#include <pthread.h>

#ifndef ECHO_REPLY_H
#define ECHO_REPLY_H

#define DEFAULT_SERVER_PORT 10000

#define WORKER_NUM 3

// スレッド作成のリクエスト
typedef struct {
  int sock; 
  void (*func)(int);
} REQUEST;

// プログラム名
extern char *program_name;

// シグナルハンドリングに使用される変数
extern int to_die;
extern pthread_mutex_t m;

void initialize_circular_buffer (void);
void respond(int);

void *handle_signal (void *);
void create_request (void *);
void handle_request (void *);
#endif

#ifdef SOMAXCONN
#define LISTEN_BACKLOG SOMAXCONN
#else
#define LISTEN_BACKLOG 5
#endif
