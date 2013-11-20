#include <stdio.h>
#include "banking.h"

int main (void) {
  int value;
  int succeeded;
  int i;

  for (i = 0, value = 100; i < 10; i++) {
    deposit(value);
    printf("depositted: %d\n", value);
    printf("balance: %d\n", get_balance());
  }

  for (i = 0, value = 200, succeeded = 0; i < 10; i++) {
    succeeded = withdraw(value);
    if (succeeded) {
      printf("withdrawed: %d\n", value);
    }
    else {
      printf("withdrawing failed. insufficient fund.\n");
    }
    printf("balance: %d\n", get_balance());
  }

  return 0;
}
