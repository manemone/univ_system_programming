#include <stdio.h>
#include <string.h>
#include "alloc.h"

#define STATUS_FAILED -1
#define STATUS_SUCCEEDED 0

#define PTR_NUM 64
#define MSG_LENGTH 1024

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
test_result ensure_alloc_to_fail_on_overlimit_request(void);
test_result ensure_alloc_and_afree_to_succeed_on_request_within_the_size_limit(void);
test_result ensure_alloc_not_to_allocate_duplicated_memory_space(void);

int main(void) {
  TEST_FUNC tests[] = {
    ensure_alloc_to_fail_on_overlimit_request,
    ensure_alloc_and_afree_to_succeed_on_request_within_the_size_limit,
    ensure_alloc_not_to_allocate_duplicated_memory_space,
  };
  int i;

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

  switch (result.status_code) {
    case STATUS_FAILED:
      printf("\x1b[31m");
      printf("[FAILED]    %s\n", result.func_name);
      printf("\x1b[39m");
      printf("            %s\n", result.msg);
      break;
    case STATUS_SUCCEEDED:
      printf("\x1b[32m");
      printf("[SUCCEEDED] %s\n", result.func_name);
      printf("\x1b[39m");
      break;
    default:
      break;
  }
}

/**
 * 制限を超えたメモリ割り当ては失敗する
 **/
test_result ensure_alloc_to_fail_on_overlimit_request(void) {
  test_result result;
  strcpy(result.func_name, __func__);
  char buffer[MSG_LENGTH];

  int request = ALLOCSIZE+1;
  char *allocated = (char *)alloc(request);

  if (allocated == 0) {
    goto succeeded;
  }
  else {
    sprintf(buffer, "memory allocation expected to fail, but succeeded. requested = %d, ALLOCSIZE: %d\n", request, ALLOCSIZE);
    strcpy(result.msg, buffer);
    goto failed;
  }

succeeded:
  result.status_code = STATUS_SUCCEEDED;
  return result;
failed:
  afree(allocated);
  result.status_code = STATUS_FAILED;
  return result;
}

/**
 * 制限を超えない範囲でのメモリ割り当て/開放は成功する
 **/
test_result ensure_alloc_and_afree_to_succeed_on_request_within_the_size_limit(void) {
  test_result result;
  strcpy(result.func_name, __func__);
  char buffer[MSG_LENGTH];

  char *allocated[PTR_NUM];
  int i;

  // 割付
  for (i = 0; i < PTR_NUM; i++) {
    allocated[i] = alloc(ALLOCSIZE/PTR_NUM);
    if (allocated[i] == 0) {
      sprintf(buffer, "first memory allocation failed. i = %d\n", i);
      strcpy(result.msg, buffer);
      goto failed;
    }
  }

  // 開放
  for (i--; i >= PTR_NUM/2; i--) {
    afree(allocated[i]);
  }

  // 割付
  for (i++; i < PTR_NUM; i++) {
    allocated[i] = alloc(ALLOCSIZE/PTR_NUM);
    if (allocated[i] == 0) {
      sprintf(buffer, "second memory allocation failed. i = %d\n", i);
      strcpy(result.msg, buffer);
      goto failed;
    }
  }

  // 開放
  for (i--; i >= 0; i--) {
    afree(allocated[i]);
  }

succeeded:
  result.status_code = STATUS_SUCCEEDED;
  return result;
failed:
  // 開放
  for (i = PTR_NUM; i >= 0; i--) {
    afree(allocated[i]);
  }
  result.status_code = STATUS_FAILED;
  return result;
}

/**
 * 割り当てられたメモリ領域は重ならない
 **/
test_result ensure_alloc_not_to_allocate_duplicated_memory_space(void) {
  test_result result;
  strcpy(result.func_name, __func__);
  char buffer[MSG_LENGTH];

  char *allocated[PTR_NUM];
  int i, j;

  // 割付と値の書き込み
  for (i = 0; i < PTR_NUM; i++) {
    allocated[i] = alloc(ALLOCSIZE/PTR_NUM);
    if (allocated[i] == 0) {
      sprintf(buffer, "first memory allocation failed. i = %d\n", i);
      strcpy(result.msg, buffer);
      goto failed;
    }

    for (j = 0; j < ALLOCSIZE/PTR_NUM; j++) {
      allocated[i][j] = i;
    }
  }

  // 値のチェック
  for (i = 0; i < PTR_NUM; i++) {
    for (j = 0; j < ALLOCSIZE/PTR_NUM; j++) {
      if (allocated[i][j] != i) {
        sprintf(buffer, "duplication detected. expected: allocated[%d][%d] = %d, found %d.\n", i, j, i, allocated[i][j]);
        strcpy(result.msg, buffer);
        goto failed;
      }
    }
  }

  // 開放
  for (i--; i >= 0; i--) {
    afree(allocated[i]);
  }

succeeded:
  result.status_code = STATUS_SUCCEEDED;
  return result;
failed:
  // 開放
  for (i = PTR_NUM; i >= 0; i--) {
    afree(allocated[i]);
  }
  result.status_code = STATUS_FAILED;
  return result;
}

