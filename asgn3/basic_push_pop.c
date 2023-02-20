#include "queue.h"

#include <unistd.h>

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  queue_t *q = queue_new(10);
  if (q == NULL) {
    return 1;
  }

  // push a 1
  queue_push(q, (void *)1);

  // expect to pop a 1
  uintptr_t r;
  queue_pop(q, (void **)&r);
  if (r != 1) {
    // if not, then we failed
    return 1;
  }

  queue_delete(&q);
  return 0;
}
