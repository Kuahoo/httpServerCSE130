#ifndef MYQUEUE_H_
#define MYQUEUE_H_

struct node {
    struct node* next;
    int *p_new_socket;
};
typedef struct node node_t;

void enqueue(int *p_new_socket);
int* dequeue();

#endif
