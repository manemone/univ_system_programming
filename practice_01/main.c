#include <stdio.h>
#include <string.h>
#include "alloc.h"

#define STATUS_FAILED -1
#define STATUS_SUCCEEDED 0

// テスト結果
typedef struct {
  int status_code;
  char func_name[1024];
  char msg[1024];
}test_result;

// テスト関数の関数ポインタ
typedef test_result (*TEST_FUNC)(void);

// テスト実行
void check(TEST_FUNC);

// テスト
test_result ensure_alloc_to_fail_on_overlimit_request(void);
test_result ensure_alloc_not_to_fail_on_request_within_the_size_limit(void);
test_result ensure_alloc_not_to_allocate_duplicated_memory_space(void);

int main(void) {
  TEST_FUNC tests[] = {
    ensure_alloc_to_fail_on_overlimit_request,
    ensure_alloc_not_to_fail_on_request_within_the_size_limit,
    ensure_alloc_not_to_allocate_duplicated_memory_space
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
      printf("[FAILED]  %s\n", result.func_name);
      // printf("\tmsg: %s\n", result.msg);
      break;
    case STATUS_SUCCEEDED:
      printf("[SUCCEEDED] %s\n", result.func_name);
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

  char *allocated = (char *)alloc(ALLOCSIZE+1);

  if (allocated == 0) {
    result.status_code = STATUS_SUCCEEDED;
  }
  else {
    result.status_code = STATUS_FAILED;
  }

  return result;
}

/**
 * 制限を超えない範囲でのメモリ割り当て/開放は成功する
 **/
test_result ensure_alloc_not_to_fail_on_request_within_the_size_limit(void) {
  test_result result;
  strcpy(result.func_name, __func__);

  char *a;
  char *b;
  int i;

  // 割付
  a = (char *)alloc(ALLOCSIZE/2);
  b = (char *)alloc(ALLOCSIZE/2);
  if (a == 0 || b == 0) {
    result.status_code = STATUS_FAILED;
    return result;
  }

  // 開放
  afree(a);

  // 割付
  a = (char *)alloc(ALLOCSIZE/2);
  if (a == 0) {
    result.status_code = STATUS_FAILED;
    return result;
  }

  result.status_code = STATUS_SUCCEEDED;
  return result;
}

/**
 * 割り当てられたメモリ領域は重ならない
 **/
test_result ensure_alloc_not_to_allocate_duplicated_memory_space(void) {
  test_result result;
  strcpy(result.func_name, __func__);

  char *a;
  char *b;
  int i;

  a = (char *)alloc(3);
  a[0] = 0;
  a[1] = 1;
  a[2] = 2;

  b = (char *)alloc(3);
  b[0] = 3;
  b[1] = 4;
  b[2] = 5;
  for (i = 0; i < 3; i++) {
    if (a[i] == i) {
      result.status_code = STATUS_FAILED;
      return result;
    }
  }

  result.status_code = STATUS_SUCCEEDED;
  return result;
}
