#include "queue.h"


#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c = PTHREAD_COND_INITIALIZER;
int ops = 0;
queue_t *q = NULL;

void pop_check(queue_t *q, void *elem) {
  bool rtn = queue_pop(q, elem);
  if (!rtn) {
    fprintf(stderr, "queue_pop failed!\n");
    exit (1);
  }
}

void push_check(queue_t *q, void *elem) {
  bool rtn = queue_push(q, elem);
  if (!rtn) {
    fprintf(stderr, "queue_push failed!\n");
    exit (1);
  }
}


int thread1() {
  push_check(q, (void *)1);

  pthread_mutex_lock(&mutex);
  ops = 1;
  pthread_cond_signal(&c);
  pthread_mutex_unlock(&mutex);

  return 0;
}

int thread2() {

  pthread_mutex_lock(&mutex);
  while (ops == 0) {
    pthread_cond_wait(&c, &mutex);
  }
  pthread_mutex_unlock(&mutex);

  push_check(q, (void *)2);

  return 0;
}

int main() {

  q = queue_new(10);
  if (q == NULL) {
    return 1;
  }
  pthread_t t1, t2;
  uintptr_t rc;
  bool rtn;

  pthread_create(&t1, NULL, (void *(*)(void*))thread1, NULL);
  pthread_create(&t2, NULL, (void *(*)(void*))thread2, NULL);

  pthread_join(t1, (void **)&rc);
  if (rc) {
    printf("pthread_join failed!\n");
    return 1;
  }
  pthread_join(t2, (void **)&rc);
  if (rc) {
    printf("pthread_join failed!\n");
    return 1;
  }

  pop_check(q, (void **)&rc);
  if (rc != 1) {
    printf("queue_pop produced wrong value!\n");
    return 1;
  }

  pop_check(q, (void **)&rc);
  if (rc != 2) {
    printf("queue_pop produced wrong value!\n");
    return 1;
  }

  queue_delete(&q);
  return 0;
}
