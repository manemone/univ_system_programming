#include <stdio.h>
#include <string.h>
#include "alloc.h"

#define STATUS_FAILED -1
#define STATUS_SUCCEEDED 0

// テスト結果
typedef struct {
  int status_code;
  char func_name[256];
  char msg[256];
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
      printf("\tmsg: %s\n", result.msg);
      break;
    case STATUS_SUCCEEDED:
      printf("[SUCCEEDED] %s\n", result.func_name);
      printf("\tmsg: %s\n", result.msg);
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

  result.status_code = STATUS_FAILED;
  return result;
}

/**
 * 制限を超えない範囲でのメモリ割り当ては成功する
 **/
test_result ensure_alloc_not_to_fail_on_request_within_the_size_limit(void) {
  test_result result;
  strcpy(result.func_name, __func__);

  result.status_code = STATUS_FAILED;
  return result;
}

/**
 * 割り当てられたメモリ領域は重ならない
 **/
test_result ensure_alloc_not_to_allocate_duplicated_memory_space(void) {
  test_result result;
  strcpy(result.func_name, __func__);

  result.status_code = STATUS_FAILED;
  return result;
}
