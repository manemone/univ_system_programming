#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include "alloc3.h"

#define STATUS_FAILED -1
#define STATUS_SUCCEEDED 0

#define PTR_NUM 64
#define MSG_LENGTH 1024
#define BUFFER_LENGTH 256

// テストケース
typedef struct test_case {
  int status_code;
  char case_name[MSG_LENGTH];
  char msg[MSG_LENGTH];
  void (*put_msg)(struct test_case*, char*, ...);
  void (*script)(struct test_case*);
} TEST_CASE;

// テスト初期化
void initialize_test_case(TEST_CASE*);

// テスト実行
void check(TEST_CASE*);

// テストケースのメッセージに書式付き文字列を追記
void put_msg(TEST_CASE*, char*, ...);

// テストスクリプト
void alloc3_and_afree3_can_handle_multiple_requests_which_are_not_in_lifo_order (TEST_CASE*);
void alloc3_and_afree3_can_handle_at_least_few_mb_memory_spaces_in_total (TEST_CASE*);
void all_of_allocated_memory_spaces_are_writable_without_any_errors (TEST_CASE*);
void allocated_memory_spaces_are_not_overlapped (TEST_CASE*);

// テストケース一覧
TEST_CASE cases[] = {
  {
    .case_name = "alloc3 and afree3 can handle multiple requests which are not in lifo order",
    .script = alloc3_and_afree3_can_handle_multiple_requests_which_are_not_in_lifo_order
  },
  {
    .case_name = "alloc3 and afree3 can handle at least few mb memory spaces in total",
    .script = alloc3_and_afree3_can_handle_at_least_few_mb_memory_spaces_in_total
  },
  {
    .case_name = "all of allocated memory spaces are writable without any errors",
    .script = all_of_allocated_memory_spaces_are_writable_without_any_errors
  },
  {
    .case_name = "allocated memory spaces are not overlapped",
    .script = allocated_memory_spaces_are_not_overlapped
  },
};

int main(void) {
  int i;

  // 全てのテストコードを実行
  for (i = 0; i < sizeof(cases)/sizeof(cases[0]); i++) {
    check(&cases[i]);
  }

  return 0;
}

/**
 * テストケースの初期化
 **/
void initialize_test_case (TEST_CASE* kase)  {
  kase->status_code = (int)NULL;
  kase->msg[0] = '\n';
  strncpy(kase->msg, "no message given.\n", MSG_LENGTH);
  kase->put_msg = put_msg;
}

/**
 * テストケースを実行する
 **/
void check(TEST_CASE* kase) {
  initialize_test_case(kase);

  // テスト中で使用する乱数発生器にシードを設定
  srand((unsigned int)time(NULL));

  kase->script(kase);

  switch (kase->status_code) {
    case STATUS_FAILED:
      printf("\x1b[31m");
      printf("[FAILED]    %s\n", kase->case_name);
      printf("            %s\n", kase->msg);
      break;
    case STATUS_SUCCEEDED:
      printf("\x1b[32m");
      printf("[SUCCEEDED] %s\n", kase->case_name);
      break;
    default:
      printf("\x1b[33m");
      printf("[UNKNOWN]   %s\n", kase->case_name);
      printf("            %s\n", kase->msg);
      break;
  }
  printf("\x1b[39m");
}

/**
 * テスト結果のメッセージに追記する
 **/
void put_msg(TEST_CASE *kase, char *format, ...) {
  va_list args;
  char *buffer[MSG_LENGTH];

  va_start(args, format);
  vsnprintf((char *)buffer, BUFFER_LENGTH, format, args);
  va_end(args);

  strncpy(kase->msg, (char *)buffer, MSG_LENGTH-strlen(kase->msg));
}

/**
 * LIFO順でない割り付け・解放を多数回繰り返しても失敗しない。
 **/
void alloc3_and_afree3_can_handle_multiple_requests_which_are_not_in_lifo_order (TEST_CASE* kase) {
  char *allocated[PTR_NUM];
  int reqsize = sizeof(char)*(ALLOC_UNIT*2)/(PTR_NUM);
  int i, j, allocated_index;

  for (j = 0; j < 500; j++) {
    for (i = 0; i < PTR_NUM; i++) {
      // ALLOC_UNIT の 2 倍程度のメモリ領域をリクエスト
      allocated[i] = (char *)alloc3(reqsize);
      if (allocated[i] == 0) {
        kase->put_msg(kase, "memory allocation failed. loop: %d, i = %d, requested size = %d\n", j, i, reqsize);
        goto failed;
      }
    }

    // ランダムに開放
    for (allocated_index = rand()%PTR_NUM, i = 0; i < PTR_NUM; i++) {
      while (allocated[allocated_index] == 0) {
        allocated_index = rand()%PTR_NUM;
      }
      afree3(allocated[allocated_index]);
      allocated[allocated_index] = 0;
    }

    // もう一度割付
    for (i = 0; i < PTR_NUM; i++) {
      allocated[i] = (char *)alloc3(reqsize);
      if (allocated[i] == 0) {
        kase->put_msg(kase, "memory allocation failed. loop: %d, i = %d, requested size = %d\n", j, i, reqsize);
        goto failed;
      }
    }

    // 割付とおなじ順で開放
    for (i = 0; i < PTR_NUM; i++) {
      afree3(allocated[i]);
    }
  }

succeeded:
  kase->status_code = STATUS_SUCCEEDED;
  return;
failed:
  for (--i; i >= 0; i--) {
    afree3(allocated[i]);
  }
  kase->status_code = STATUS_FAILED;
  return;
}

/**
 * 合計で数 MB 程度の領域を割りつけてもエラーにならない。
 **/
void alloc3_and_afree3_can_handle_at_least_few_mb_memory_spaces_in_total (TEST_CASE* kase) {
  char *allocated[PTR_NUM];
  int reqsize = sizeof(char)*(5*1024*1024)/(PTR_NUM);
  int i, j;

  for (j = 0; j < 500; j++) {
    for (i = 0; i < PTR_NUM; i++) {
      // 合計で 5 MB 程度の領域を要求
      allocated[i] = (char *)alloc3(reqsize);
      if (allocated[i] == 0) {
        kase->put_msg(kase, "memory allocation failed. loop: %d, i = %d, requested size = %d\n", j, i, reqsize);
        goto failed;
      }
    }

    // 割付とおなじ順で開放
    for (i = 0; i < PTR_NUM; i++) {
      afree3(allocated[i]);
    }
  }

succeeded:
  kase->status_code = STATUS_SUCCEEDED;
  return;
failed:
  kase->status_code = STATUS_FAILED;
  return;
}

/**
 * 割りつけた領域全体に書き込んでもエラーにならない。
 **/
void all_of_allocated_memory_spaces_are_writable_without_any_errors (TEST_CASE* kase) {
  char *allocated[PTR_NUM];
  int reqsize = sizeof(char)*(ALLOC_UNIT*2)/(PTR_NUM);
  int i, j;

  for (i = 0; i < PTR_NUM; i++) {
    // ALLOC_UNIT の 2 倍程度のメモリ領域をリクエスト
    allocated[i] = (char *)alloc3(reqsize);
    if (allocated[i] == 0) {
      kase->put_msg(kase, "memory allocation failed. loop: %d, i = %d, requested size = %d\n", j, i, reqsize);
      goto failed;
    }
  }

  // すべての領域に書き込み
  for (i = 0; i < PTR_NUM; i++) {
    for (j = 0; j < reqsize; j++) {
      // SIGSEGV が発生するとプログラムは止まる
      allocated[i][j] = i;
    }
  }

  // 割付とおなじ順で開放
  for (i = 0; i < PTR_NUM; i++) {
    afree3(allocated[i]);
  }

succeeded:
  kase->status_code = STATUS_SUCCEEDED;
  return;
failed:
  for (--i; i >= 0; i--) {
    afree3(allocated[i]);
  }
  kase->status_code = STATUS_FAILED;
  return;
}

/**
 * 割り付けを受けて、まだ解放されていない領域は、互いに重なっていない。
 **/
void allocated_memory_spaces_are_not_overlapped (TEST_CASE* kase) {
  goto failed;

succeeded:
  kase->status_code = STATUS_SUCCEEDED;
  return;
failed:
  kase->status_code = STATUS_FAILED;
  return;
}

