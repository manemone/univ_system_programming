#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <syslog.h>
#include <pthread.h>
#include <signal.h>

#include <unistd.h>
#include <pthread.h>

#include "echo_reply.h"
#include "logutil.h"

int main (int argc, char **argv) {
  char *port_number = NULL;
  int ch, sock, server_port = DEFAULT_SERVER_PORT;
  int debug_mode = 0;
  pthread_t signal_handler;
  pthread_t request_creator;
  pthread_t request_handlers[WORKER_NUM];
  void *return_values[WORKER_NUM];
  sigset_t signal_set;
  int i;

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
  // 以後起動されるスレッドにはこのマスクが継承される
  sigemptyset(&signal_set);
  sigaddset(&signal_set, SIGINT);
  sigaddset(&signal_set, SIGTERM);
  pthread_sigmask(SIG_BLOCK, &signal_set, NULL);

  // シグナルハンドラスレッドの起動
  pthread_create(&signal_handler, NULL, (void *)handle_signal, &signal_set);

  // リクエストキューの初期化処理
  initialize_circular_buffer();

  // 接続を受け付けてスレッド作成のリクエストをキューに入れるスレッドの起動
  pthread_create(&request_creator, NULL, (void *)create_request, (void *)sock);

  // キューからリクエストを取り出し、応答処理を行うスレッドの起動
  for (i = 0; i < WORKER_NUM; i++) {
    pthread_create(&request_handlers[i], NULL, (void *)handle_request, NULL);
  }

  for (i = 0; i < WORKER_NUM; i++) {
    pthread_join(request_handlers[i], &return_values[i]);
    // NEVER REACHED
  }
}


