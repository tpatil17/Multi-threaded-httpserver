#include "queue.h"
#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>

typedef struct Node
{
    void *data;
    struct Node *next;
    
} Node;


struct queue
{
    Node *head;
    Node *tail;
    int size;
    
}queue;

queue_t* queue_new(int size){

    queue_t *q = malloc(sizeof(struct queue));

    if(q == NULL){
        fprintf(stderr, "Problem in allocating memory for queue\n");
        exit(1);
    } 
    q->head = NULL;
    q->tail = NULL;
    q->size = size;

    return q;
}

void queue_delete(struct queue **q){

    while((*q)->head != NULL){
        Node *temp = (*q)->head;
        (*q)->head = (*q)->head->next;
        free(temp);
        if((*q)->head == NULL){
            (*q)->tail = NULL;
        }

    }
    free(*q);
    *q = NULL;
}

bool queue_push(queue_t *q, void *elem){

    if(q == NULL){
        return false;
    }

    Node *new = malloc(sizeof(Node));
    if(new == NULL){
        fprintf(stderr, "Failed to allocate memory to node\n");
        exit(1);
    }
    new->data = elem;
    new->next = NULL;

    if(q->tail == NULL){
        q->head = new;
        q->tail = new;
    }
    else{
        q->tail->next = new;
        q->tail = new;
    }

    return true;

}

bool queue_pop(queue_t *q, void **elem){
    if(q->head == NULL){
        fprintf(stderr, "queue is empty");
    }
    *elem = q->head->data;
    Node *temp = q->head;
    q->head = q->head->next;
    free(temp);
    if(q->head == NULL){
        q->tail = NULL;
    }

    return true;
}

