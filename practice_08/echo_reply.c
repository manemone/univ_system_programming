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

#include "echo_reply.h"
#include "circular_buffer.h"
#include "logutil.h"

// �v���O������
char *program_name = "sp6-server";

// ����o�b�t�@
circ_buf_t *buffer;

// �V�O�i���n���h�����O�Ɏg�p�����ϐ�
int to_die = 0;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

// ����o�b�t�@�̏�����
void initialize_circular_buffer (void) {
  buffer = (circ_buf_t *)malloc(sizeof(circ_buf_t));
  pthread_mutex_init(&buffer->buf_lock, NULL);
  buffer->start = 0;
  buffer->num_full = 0;
  pthread_cond_init(&buffer->notfull, NULL);
  pthread_cond_init(&buffer->notempty, NULL);
}

// �g�����̕\��
void usage (void) {
  fprintf(stderr, "Usage: %s [option]\n", program_name);
  fprintf(stderr, "option:\n");
  fprintf(stderr, "\t-d\t\t\t\t... debug mode\n");
  fprintf(stderr, "\t-p <port>\n");
  exit(1);
}

int open_accepting_socket (int port) {
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

// ���N�G�X�g����(�N���C�A���g�ւ̉���)�֐�
void respond (int sock) {
  char c;
  FILE *fp;

  send(sock, "HELLO. what you type will be echo back to you.\n", 47, 0);

  fp = fdopen(sock, "rb");
  while ((c = getc(fp)) != EOF) {
    send(sock, &c, 1, 0);
    fprintf(stderr, "%c", c);
    pthread_testcancel();
  }
}

// ���N�G�X�g�����̃N���[���A�b�v
void cleanup_request (void *_req) {
  REQUEST *req = (REQUEST *)_req;
  char message[] = "Bye!\n";

  if (req != NULL) {
    send(req->sock, message, strlen(message), 0);
    close(req->sock);
    fprintf(stderr, "closed socket [%d].\n", req->sock);

    free(req);
    req = NULL;
  }
}

// �X���b�h�쐬�̃��N�G�X�g�I�u�W�F�N�g���쐬
REQUEST *create_thread_request (int sock, void (*func)(int)) {
  REQUEST *req;

  req = (REQUEST *)malloc(sizeof(REQUEST));

  req->sock = sock;
  req->func = func;

  return req;
}

// ���N�G�X�g������
void handle_request (void *arg) {
  REQUEST *req = NULL;
  
  printf("worker thread [%x] created.\n", (int)pthread_self());
  while (1) {
    req = (REQUEST *)get_cb_data(buffer);

    printf("worker thread [%x] handling socket: [%d].\n", (int)pthread_self(), req->sock);
    pthread_cleanup_push(cleanup_request, req);
    (req->func)(req->sock);
    pthread_cleanup_pop(1);
    printf("worker thread [%x] closed socket.\n", (int)pthread_self());
  }
}

// �ڑ����󂯕t���ă��N�G�X�g�L���[�ɒǉ�
void create_request (void *_sock) {
  // accept �֘A
  int sock = (int)_sock;
  struct sockaddr_in client_addr;
  int length;
  int client_sock;

  // poll �֘A
  int client_ready;
  struct pollfd pfd;

  length = sizeof(client_addr);
  pfd.fd = sock;
  pfd.events = POLLIN;
  while (1) {
    // �N���C�A���g����̐ڑ����|�[�����O���đ҂�
    client_ready = poll(&pfd, 1, 1000);
    if (client_ready > 0) {
      client_sock = accept(sock, (struct sockaddr *)&client_addr, &length);
      fprintf(stderr, "opened socket [%d].\n", client_sock);

      // �X���b�h�쐬�̃��N�G�X�g���쐬
      put_cb_data(buffer, (void *)create_thread_request(client_sock, respond));
    }

    // �����ϐ������� exit ����
    pthread_mutex_lock(&m);
    if (to_die) {
      fprintf(stderr, "bye\n");
      exit(0);
    }
    pthread_mutex_unlock(&m);
  }
}

// �V�O�i���n���h���X���b�h
void *handle_signal (void *_signal_set) {
  int sig, err;
  sigset_t *signal_set = (sigset_t *)_signal_set;

  while (1) {
    err = sigwait(signal_set, &sig);

    if (err != 0) {
      fprintf(stderr, "[signal thread] sigwait failed. continuing...\n");
    }
    else if (sig != SIGINT && sig != SIGTERM) {
      fprintf(stderr, "[signal thread] caught unexpected signal. continuing...\n");
    }
    else {
      break;
    }
  }

  pthread_mutex_lock(&m);
  to_die = 1;
  pthread_mutex_unlock(&m);

  // exit(1);

  return NULL;
}

