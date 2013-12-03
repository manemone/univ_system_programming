#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#include "logutil.h"

#define DEFAULT_SERVER_PORT 10000
#ifdef SOMAXCONN
#define LISTEN_BACKLOG SOMAXCONN
#else
#define LISTEN_BACKLOG 5
#endif

char *program_name = "sp6-server";

int
open_accepting_socket(int port)
{
        struct sockaddr_in self_addr;
        socklen_t self_addr_size;
        int sock, sockopt;

        memset(&self_addr, 0, sizeof(self_addr));
        self_addr.sin_family = AF_INET;
        self_addr.sin_addr.s_addr = INADDR_ANY;
        self_addr.sin_port = htons(port);
        self_addr_size = sizeof(self_addr);
        sock = socket(PF_INET, SOCK_STREAM, 0);
        if (sock < 0)
                logutil_fatal("accepting socket: %d", errno);
        sockopt = 1;
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
            &sockopt, sizeof(sockopt)) == -1)
                logutil_warning("SO_REUSEADDR: %d", errno);
        if (bind(sock, (struct sockaddr *)&self_addr, self_addr_size) < 0)
                logutil_fatal("bind accepting socket: %d", errno);
        if (listen(sock, LISTEN_BACKLOG) < 0)
                logutil_fatal("listen: %d", errno);
        return (sock);
}

void
usage(void)
{
        fprintf(stderr, "Usage: %s [option]\n", program_name);
        fprintf(stderr, "option:\n");
        fprintf(stderr, "\t-d\t\t\t\t... debug mode\n");
        fprintf(stderr, "\t-p <port>\n");
        exit(1);
}

void
respond (void *_sock) {
  char c;
  FILE *fp;
  int sock = (int)_sock;

  send(sock, "HELLO. what you type will be echo back to you.\n", 47, 0);

  fp = fdopen(sock, "rb");
  while ((c = getc(fp)) != EOF) {
    send(sock, &c, 1, 0);
    fprintf(stderr, "%c", c);
  }

  send(sock, "disconnected\n", 13, 0);
  close(sock);
  fprintf(stderr, "socket [%d] disconnected.\n", sock);
}

void
main_loop (int sock) {
  struct sockaddr_in client_addr;
  int length;
  int client_sock;
  pthread_t responder;
  int i, j, cnt;

  for (cnt = 0;;cnt++) {
    length = sizeof(client_addr);
    client_sock = accept(sock, (struct sockaddr *)&client_addr, &length);
    fprintf(stderr, "opened socket [%d].\n", client_sock);

    printf("accepted connection from %d, port=%d\n",
        inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    pthread_create(&responder, NULL, (void *)respond, (void *)client_sock);
    pthread_detach(responder);
  }
}

int
main(int argc, char **argv)
{
        char *port_number = NULL;
        int ch, sock, server_port = DEFAULT_SERVER_PORT;
        int debug_mode = 0;

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

        /*
         * 無限ループでsockをacceptし，acceptしたらそのクライアント用
         * のスレッドを作成しプロトコル処理を続ける．
         */
        main_loop(sock);

        /*NOTREACHED*/
        return (0);
}

