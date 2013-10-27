#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include "alloc2.h"

#define STATUS_FAILED -1
#define STATUS_SUCCEEDED 0

#define PTR_NUM 64
#define MSG_LENGTH 1024
#define BUFFER_LENGTH 256

// テストケース
typedef struct test_case {
  int status_code;
  char case_name[MSG_LENGTH];
  char buffer[MSG_LENGTH];
  char msg[MSG_LENGTH];
  void (*put_msg)(struct test_case*, char*, ...);
  void (*script)(struct test_case*);
} TEST_CASE;

// テスト関数の関数ポインタ
typedef void (*TEST_FUNC2)(TEST_CASE*);

// テスト初期化
void initialize_test_case(TEST_CASE*);

// テスト実行
void check(TEST_CASE*);

// テストケースのメッセージに書式付き文字列を追記
void put_msg(TEST_CASE*, char*, ...);

// テストスクリプト
void alloc2_and_afree2_can_handle_multiple_requests_within_the_size_limit (TEST_CASE*);
void alloc2_and_afree2_can_handle_multiple_requests_which_are_not_in_lifo_order (TEST_CASE*);
void alloc2_fails_on_overlimit_memory_request (TEST_CASE*);
void allocated_memory_spaces_are_not_overlapped (TEST_CASE*);

// テストケース一覧
TEST_CASE cases[] = {
  {
    .case_name = "alloc2 and afree2 can handle multiple requests within the size limit",
    .script = alloc2_and_afree2_can_handle_multiple_requests_within_the_size_limit
  },
  {
    .case_name = "alloc2 and afree2 can handle multiple requests which are not in lifo order",
    .script = alloc2_and_afree2_can_handle_multiple_requests_which_are_not_in_lifo_order
  },
  {
    .case_name = "alloc2 fails on overlimit memory request",
    .script = alloc2_fails_on_overlimit_memory_request
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
void initialize_test_case (TEST_CASE* attr)  {
  attr->status_code = (int)NULL;
  attr->msg[0] = '\n';
  strncpy(attr->msg, "no message given.\n", MSG_LENGTH);
  attr->put_msg = put_msg;
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

  printf("put_msg() called.\n");
  va_start(args, format);
  vsnprintf((char *)buffer, BUFFER_LENGTH, format, args);
  va_end(args);

  strncpy(kase->msg, (char *)buffer, MSG_LENGTH-strlen(kase->msg));
}

/**
 * 上限を越えない範囲で割り付け・解放を多数回繰り返しても失敗しない。
 **/
void alloc2_and_afree2_can_handle_multiple_requests_within_the_size_limit (TEST_CASE* kase) {
  char *allocated[PTR_NUM];
  int reqsize = sizeof(char)*ALLOCSIZE/(PTR_NUM*2);
  int i, j;

  // だいたい ALLOCSIZE の半分が埋まるように alloc2
  for (j = 0; j < 500; j++) {
    for (i = 0; i < PTR_NUM; i++) {
      allocated[i] = (char *)alloc2(reqsize);
      if (allocated[i] == 0) {
        kase->put_msg(kase, "memory allocation failed. loop: %d, i = %d, requested size = %d, ALLOCSIZE: %d\n", j, i, reqsize, ALLOCSIZE);
        goto failed;
      }
    }

    // afree2
    for (i = PTR_NUM-1; i >= 0; i--) {
      afree2(allocated[i]);
    }
  }

succeeded:
  kase->status_code = STATUS_SUCCEEDED;
  return;
failed:
  for (--i; i >= 0; i--) {
    afree2(allocated[i]);
  }
  kase->status_code = STATUS_FAILED;
  return;
}

/**
 * LIFO順でない割り付け・解放を多数回繰り返しても失敗しない。
 **/
void alloc2_and_afree2_can_handle_multiple_requests_which_are_not_in_lifo_order (TEST_CASE* kase) {
  char *allocated[PTR_NUM];
  int reqsize = sizeof(char)*ALLOCSIZE/(PTR_NUM*2);
  int i, j, allocated_index;

  for (j = 0; j < 500; j++) {
    // だいたい ALLOCSIZE の半分が埋まるように alloc2
    for (i = 0; i < PTR_NUM; i++) {
      allocated[i] = (char *)alloc2(reqsize);
      if (allocated[i] == 0) {
        kase->put_msg(kase, "memory allocation failed. loop: %d, i = %d, requested size = %d, ALLOCSIZE: %d\n", j, i, reqsize, ALLOCSIZE);
        goto failed;
      }
    }

    // ランダムに開放
    for (allocated_index = rand()%PTR_NUM, i = 0; i < PTR_NUM; i++) {
      while (allocated[allocated_index] == 0) {
        allocated_index = rand()%PTR_NUM;
      }
      afree2(allocated[allocated_index]);
      allocated[allocated_index] = 0;
    }

    // もう一度割付
    for (i = 0; i < PTR_NUM; i++) {
      allocated[i] = (char *)alloc2(reqsize);
      if (allocated[i] == 0) {
        kase->put_msg(kase, "memory allocation failed. loop: %d, i = %d, requested size = %d, ALLOCSIZE: %d\n", j, i, reqsize, ALLOCSIZE);
        goto failed;
      }
    }

    // 割付とおなじ順で開放
    for (i = 0; i < PTR_NUM; i++) {
      afree2(allocated[i]);
    }
  }

succeeded:
  kase->status_code = STATUS_SUCCEEDED;
  return;
failed:
  for (--i; i >= 0; i--) {
    afree2(allocated[i]);
  }
  kase->status_code = STATUS_FAILED;
  return;
}

/**
 * 上限を越えた割り付けを行なうと、失敗する。
 **/
void alloc2_fails_on_overlimit_memory_request (TEST_CASE* kase) {
  if (alloc2(ALLOCSIZE+1) != 0) goto failed;
  
succeeded:
  kase->status_code = STATUS_SUCCEEDED;
  return;
failed:
  kase->status_code = STATUS_FAILED;
  return;
}

/**
 * 割り付けを受けて、まだ解放されていない領域は、互いに重なっていない。
 **/
void allocated_memory_spaces_are_not_overlapped (TEST_CASE* kase) {
  char *allocated[PTR_NUM];
  int reqsize = sizeof(char)*ALLOCSIZE/(PTR_NUM*2);
  int i, j, allocated_index;

  // だいたい ALLOCSIZE の半分が埋まるように alloc2
  for (i = 0; i < PTR_NUM; i++) {
    allocated[i] = (char *)alloc2(reqsize);
    if (allocated[i] == 0) {
      kase->put_msg(kase, "memory allocation failed. loop: %d, i = %d, requested size = %d, ALLOCSIZE: %d\n", j, i, reqsize, ALLOCSIZE);
      goto failed;
    }
  }

  // fill the each allocated memory spaces with its index number
  for (i = 0; i < PTR_NUM; i++) {
    for (j = 0; j < reqsize; j++) {
      allocated[i][j] = (char)i;
    }
  }

  // check all numbers in the allocated memory spaces
  for (i = 0; i < PTR_NUM; i++) {
    for (j = 0; j < reqsize; j++) {
      if (allocated[i][j] != (char)i){
        kase->put_msg(kase, "overlapping check failed. expected: %d, found %d. i = %d, j: %d\n", i, allocated[i][j], i, j);
        goto failed;
      }
    }
  }

  // 割付とおなじ順で開放
  for (i = 0; i < PTR_NUM; i++) {
    afree2(allocated[i]);
  }

succeeded:
  kase->status_code = STATUS_SUCCEEDED;
  return;
failed:
  kase->status_code = STATUS_FAILED;
  return;
}

