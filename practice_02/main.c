#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "alloc2.h"

#define STATUS_FAILED -1
#define STATUS_SUCCEEDED 0

#define PTR_NUM 64
#define MSG_LENGTH 1024
#define BUFFER_LENGTH 256

// テスト結果
typedef struct {
  int status_code;
  char func_name[MSG_LENGTH];
  char msg[MSG_LENGTH];
}test_result;

// テスト関数の関数ポインタ
typedef test_result (*TEST_FUNC)(void);

// テスト実行
void check(TEST_FUNC);

// テスト
test_result alloc2_and_afree2_can_handle_multiple_requests_within_the_size_limit (void);
test_result alloc2_and_afree2_can_handle_multiple_requests_which_are_not_in_lifo_order (void);
test_result alloc2_fails_on_overlimit_memory_request (void);
test_result allocated_memory_spaces_are_not_overlapped (void);

int main(void) {
  // テスト項目
  TEST_FUNC tests[] = {
    alloc2_and_afree2_can_handle_multiple_requests_within_the_size_limit,
    alloc2_and_afree2_can_handle_multiple_requests_which_are_not_in_lifo_order,
    alloc2_fails_on_overlimit_memory_request,
    // allocated_memory_spaces_are_not_overlapped,
  };
  int i;

  // 全てのテストコードを実行
  for (i = 0; i < sizeof(tests)/sizeof(tests[0]); i++) {
    check(tests[i]);
  }

  return 0;
}

/**
 * テストを実行する
 **/
void check(TEST_FUNC test) {
  test_result result = test();

  // テスト中で使用する乱数発生器にシードを設定
  srand((unsigned int)time(NULL));

  switch (result.status_code) {
    case STATUS_FAILED:
      printf("\x1b[31m");
      printf("[FAILED]    %s\n", result.func_name);
      printf("            %s\n", result.msg);
      break;
    case STATUS_SUCCEEDED:
      printf("\x1b[32m");
      printf("[SUCCEEDED] %s\n", result.func_name);
      break;
    default:
      printf("\x1b[33m");
      printf("[UNKNOWN]   %s\n", result.func_name);
      printf("            %s\n", result.msg);
      break;
  }
  printf("\x1b[39m");
}

/**
 * 上限を越えない範囲で割り付け・解放を多数回繰り返しても失敗しない。
 **/
test_result alloc2_and_afree2_can_handle_multiple_requests_within_the_size_limit (void) {
  test_result result = {
    .status_code = (int)NULL,
    .msg = "\0",
  };
  strncpy(result.func_name, __func__, MSG_LENGTH);
  char buffer[BUFFER_LENGTH];

  char *allocated[PTR_NUM];
  int reqsize = sizeof(char)*ALLOCSIZE/(PTR_NUM*2);
  int i, j;

  // だいたい ALLOCSIZE の半分が埋まるように alloc2
  for (j = 0; j < 500; j++) {
    for (i = 0; i < PTR_NUM; i++) {
      allocated[i] = (char *)alloc2(reqsize);
      if (allocated[i] == 0) {
        snprintf(buffer, BUFFER_LENGTH, "memory allocation failed. loop: %d, i = %d, requested size = %d, ALLOCSIZE: %d\n", j, i, reqsize, ALLOCSIZE);
        strncpy(result.msg, buffer, MSG_LENGTH-strlen(result.msg));
        goto failed;
      }
    }

    // afree2
    for (i = PTR_NUM-1; i >= 0; i--) {
      afree2(allocated[i]);
    }
  }

succeeded:
  result.status_code = STATUS_SUCCEEDED;
  return result;
failed:
  for (--i; i >= 0; i--) {
    afree2(allocated[i]);
  }
  result.status_code = STATUS_FAILED;
  return result;
}

/**
 * LIFO順でない割り付け・解放を多数回繰り返しても失敗しない。
 **/
test_result alloc2_and_afree2_can_handle_multiple_requests_which_are_not_in_lifo_order (void) {
  test_result result = {
    .status_code = (int)NULL,
    .msg = "\0",
  };
  strncpy(result.func_name, __func__, MSG_LENGTH);
  char buffer[BUFFER_LENGTH];

  char *allocated[PTR_NUM];
  int reqsize = sizeof(char)*ALLOCSIZE/(PTR_NUM*2);
  int i, j, allocated_index;

  for (j = 0; j < 500; j++) {
    // だいたい ALLOCSIZE の半分が埋まるように alloc2
    for (i = 0; i < PTR_NUM; i++) {
      allocated[i] = (char *)alloc2(reqsize);
      if (allocated[i] == 0) {
        snprintf(buffer, BUFFER_LENGTH, "memory allocation failed. loop: %d, i = %d, requested size = %d, ALLOCSIZE: %d\n", j, i, reqsize, ALLOCSIZE);
        strncpy(result.msg, buffer, MSG_LENGTH-strlen(result.msg));
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
        snprintf(buffer, BUFFER_LENGTH, "memory allocation failed. loop: %d, i = %d, requested size = %d, ALLOCSIZE: %d\n", j, i, reqsize, ALLOCSIZE);
        strncpy(result.msg, buffer, MSG_LENGTH-strlen(result.msg));
        goto failed;
      }
    }

    // 割付とおなじ順で開放
    for (i = 0; i < PTR_NUM; i++) {
      afree2(allocated[i]);
    }
  }

succeeded:
  result.status_code = STATUS_SUCCEEDED;
  return result;
failed:
  result.status_code = STATUS_FAILED;
  return result;
}

/**
 * 上限を越えた割り付けを行なうと、失敗する。
 **/
test_result alloc2_fails_on_overlimit_memory_request (void) {
  test_result result = {
    .status_code = (int)NULL,
    .msg = "\0",
  };
  strncpy(result.func_name, __func__, MSG_LENGTH);
  char buffer[BUFFER_LENGTH];

  if (alloc2(ALLOCSIZE+1) != 0) goto failed;
  
succeeded:
  result.status_code = STATUS_SUCCEEDED;
  return result;
failed:
  result.status_code = STATUS_FAILED;
  return result;
}

/**
 * 割り付けを受けて、まだ解放されていない領域は、互いに重なっていない。
 **/
test_result allocated_memory_spaces_are_not_overlapped (void) {
  test_result result = {
    .status_code = (int)NULL,
    .msg = "\0",
  };
  strncpy(result.func_name, __func__, MSG_LENGTH);
  char buffer[BUFFER_LENGTH];

  goto failed;

succeeded:
  result.status_code = STATUS_SUCCEEDED;
  return result;
failed:
  result.status_code = STATUS_FAILED;
  return result;
}

