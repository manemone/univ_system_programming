#include <pthread.h>

#ifndef ECHO_REPLY_H
#define ECHO_REPLY_H

#define DEFAULT_SERVER_PORT 10000

// プログラム名
extern char *program_name;

// シグナルハンドリングに使用される変数
extern int to_die;
extern pthread_mutex_t m;

void *handle_signal (void *);
#endif

#ifdef SOMAXCONN
#define LISTEN_BACKLOG SOMAXCONN
#else
#define LISTEN_BACKLOG 5
#endif
