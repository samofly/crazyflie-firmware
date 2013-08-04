#include <stdio.h>

uint32_t x;

int main(void) {
  x = 1;
  // CHECK: x=1
  printf("x=%lu\n", x);

  __sync_fetch_and_add(&x, 1);
  // CHECK: x=2
  printf("x=%lu\n", x);

  // CHECK: y=2
  uint32_t y;
  __atomic_load(&x, &y, __ATOMIC_SEQ_CST);
  printf("y=%lu\n", y);
}
