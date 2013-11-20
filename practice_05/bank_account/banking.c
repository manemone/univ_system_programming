#include <pthread.h>
#include "banking.h"

// 口座残高
static int balance = 0;

// 残高の mutex
pthread_mutex_t mutex_balance = PTHREAD_MUTEX_INITIALIZER;

/**
 * 口座へ入金する。
 * 処理が成功すると 1 を返す。
 **/
void deposit (int n) {
  pthread_mutex_lock(&mutex_balance);
  balance += n;
  pthread_mutex_unlock(&mutex_balance);
}

/**
 * 預金を引き出す。
 * 引き出し処理が成功すると 1 を返す。
 * 残高不足により処理が行えなければ 0 を返す。
 **/
int withdraw (int n) {
  int ret;

  pthread_mutex_lock(&mutex_balance);
  if (balance-n < 0) {
    ret = 0;
  }
  else {
    balance -= n;
    ret = 1;
  }
  pthread_mutex_unlock(&mutex_balance);

  return ret;
}

/**
 * 残高を返す。
 **/
int get_balance (void) {
  int ret;

  pthread_mutex_lock(&mutex_balance);
  ret = balance; 
  pthread_mutex_unlock(&mutex_balance);

  return ret;
}

