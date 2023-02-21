#include "queue.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>

typedef struct Node {
    void *data;
    struct Node *next;

} Node;

struct queue {
    Node *head;
    Node *tail;
    int size;
    sem_t *full;
    sem_t *empty;
    sem_t *mutex;

} queue;

queue_t *queue_new(int size) {

    queue_t *q = malloc(sizeof(struct queue));

    if (q == NULL) {
        fprintf(stderr, "Problem in allocating memory for queue\n");
        exit(1);
    }
    q->head = NULL;
    q->tail = NULL;
    q->size = size;
    q->empty = (sem_t *) malloc(sizeof(sem_t));
    q->full = (sem_t *) malloc(sizeof(sem_t));
    q->mutex = (sem_t *) malloc(sizeof(sem_t));
    sem_init(q->empty, 0, 0);
    sem_init(q->full, 0, size);
    sem_init(q->mutex, 0, 1);

    return q;
}

void queue_delete(struct queue **q) {

    while ((*q)->head != NULL) {
        Node *temp = (*q)->head;
        (*q)->head = (*q)->head->next;
        free(temp);
        if ((*q)->head == NULL) {
            (*q)->tail = NULL;
        }
    }
    sem_destroy((*q)->mutex);
    sem_destroy((*q)->full);
    sem_destroy((*q)->empty);
    free((*q)->empty);
    free((*q)->full);
    free((*q)->mutex);
    free(*q);
    *q = NULL;
}

bool queue_push(queue_t *q, void *elem) {

    sem_wait(q->full); //claim a spot
    sem_wait(q->mutex);

    Node *new = malloc(sizeof(Node));
    if (new == NULL) {
        fprintf(stderr, "Failed to allocate memory to node\n");
        exit(1);
    }
    new->data = elem;
    new->next = NULL;

    if (q->tail == NULL) {
        q->head = new;
        q->tail = new;
    } else {
        q->tail->next = new;
        q->tail = new;
    }
    sem_post(q->mutex);
    sem_post(q->empty);

    return true;
}

bool queue_pop(queue_t *q, void **elem) {
    sem_wait(q->empty);
    sem_wait(q->mutex);

    *elem = q->head->data;
    Node *temp = q->head;
    q->head = q->head->next;
    free(temp);
    if (q->head == NULL) {
        q->tail = NULL;
    }
    sem_post(q->mutex);
    sem_post(q->full);

    return true;
}
