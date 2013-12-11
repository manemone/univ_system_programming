#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <poll.h>

#include <unistd.h>
#include <pthread.h>

#include "circular_buffer.h"
#include "echo_reply.h"
#include "logutil.h"

// 巡回バッファ
circ_buf_t *buffer;

/* エンキューのスレッド */
void enqueue (void *_no) {
  int no = (int)_no;
  int i;

  for (i = 0; i < ENQUEUE_UNIT; i++) {
    put_cb_data(buffer, (void *)no);
  }
}

/* デキューのスレッド */
void dequeue (void *_no) {
  int no = (int)_no;
  int i;

  for (i = 0; i < DEQUEUE_UNIT; i++) {
    get_cb_data(buffer);
  }
}

// 巡回バッファの初期化
void initialize_circular_buffer (void) {
  buffer = (circ_buf_t *)malloc(sizeof(circ_buf_t));
  pthread_mutex_init(&buffer->buf_lock, NULL);
  buffer->start = 0;
  buffer->num_full = 0;
  pthread_cond_init(&buffer->notfull, NULL);
  pthread_cond_init(&buffer->notempty, NULL);
}

// 巡回バッファ操作スレッドの起動とジョイン
void operate_circular_buffer (void) {
  pthread_t enquerors[ENQUEUEROR_NUM];
  pthread_t dequerors[DEQUEUEROR_NUM];
  int i;

  for (i = 0; i < ENQUEUEROR_NUM; i++) {
    pthread_create(&enquerors[i], NULL, (void *)enqueue, (void *)i);
  }
  for (i = 0; i < DEQUEUEROR_NUM; i++) {
    pthread_create(&dequerors[i], NULL, (void *)dequeue, (void *)i);
  }

  for (i = 0; i < ENQUEUEROR_NUM; i++) {
    pthread_join(enquerors[i], NULL);
  }
  for (i = 0; i < DEQUEUEROR_NUM; i++) {
    pthread_join(dequerors[i], NULL);
  }
}

int main (int argc, char **argv) {
  char *port_number = NULL;
  int ch, sock, server_port = DEFAULT_SERVER_PORT;
  int debug_mode = 0;
  pthread_t signal_handler;
  sigset_t signal_set;

  initialize_circular_buffer();
  // operate_circular_buffer();

  while ((ch = getopt(argc, argv, "dp:")) != -1) {
    switch (ch) {
      case 'd':
        debug_mode = 1;
        break;
      case 'p':
        port_number = optarg;
        break;
      case '?':
      default:
        usage();
    }
  }
  argc -= optind;
  argv += optind;

  if (port_number != NULL)
    server_port = strtol(port_number, NULL, 0);

  /* server_portでlistenし，socket descriptorをsockに代入 */
  sock = open_accepting_socket(server_port);

  if (!debug_mode) {
    logutil_syslog_open(program_name, LOG_PID, LOG_LOCAL0);
    daemon(0, 0);
  }

  // シグナルマスクの作成
  sigemptyset(&signal_set);
  sigaddset(&signal_set, SIGINT);
  sigaddset(&signal_set, SIGTERM);
  pthread_sigmask(SIG_BLOCK, &signal_set, NULL);

  // シグナルハンドラスレッドの起動
  pthread_create(&signal_handler, NULL, (void *)handle_signal, &signal_set);

  /*
   * 無限ループでsockをacceptし，acceptしたらそのクライアント用
   * のスレッドを作成しプロトコル処理を続ける．
   */
  main_loop(sock);
}


