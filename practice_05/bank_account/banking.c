#include <pthread.h>
#include "banking.h"

static int balance = 0;

/**
 * 口座へ入金する。
 * 処理が成功すると 1 を返す。
 **/
void deposit (int n) {
}

/**
 * 預金を引き出す。
 * 引き出し処理が成功すると 1 を返す。
 * 残高不足により処理が行えなければ 0 を返す。
 **/
int withdraw (int n) {
}

/**
 * 残高を返す。
 **/
int get_balance (void) {
}

