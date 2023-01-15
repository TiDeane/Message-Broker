#include "producer-consumer.h"
#include "common.h"

int pcq_create(pc_queue_t *queue, size_t capacity) {
    queue->pcq_buffer = malloc(sizeof(pthread_t) * queue->pcq_capacity);
    queue->pcq_capacity = capacity;
    queue->pcq_current_size = 0;
    queue->pcq_head = 0;
    queue->pcq_tail = 0;

    if (pthread_mutex_init(&queue->pcq_current_size_lock, NULL) != 0) {
        perror("Failed to init Mutex");
        return -1;
    }
    if (pthread_mutex_init(&queue->pcq_head_lock, NULL) != 0) {
        perror("Failed to init Mutex");
        return -1;
    }
    if (pthread_mutex_init(&queue->pcq_tail_lock, NULL) != 0) {
        perror("Failed to init Mutex");
        return -1;
    }
    if (pthread_mutex_init(&queue->pcq_pusher_condvar_lock, NULL) != 0) {
        perror("Failed to init Mutex");
        return -1;
    }
    if (pthread_cond_init(&queue->pcq_pusher_condvar, NULL) != 0) {
        perror("Failed to init condition variable");
        return -1;
    }
    if (pthread_mutex_init(&queue->pcq_popper_condvar_lock, NULL) != 0) {
        perror("Failed to init Mutex");
        return -1;
    }
    if (pthread_cond_init(&queue->pcq_popper_condvar, NULL) != 0) {
        perror("Failed to init condition variable");
        return -1;
    }
    
    return 0;
}

int pcq_destroy(pc_queue_t *queue) {
    for(size_t i = 0; i < queue->pcq_capacity; i++)
        free(queue->pcq_buffer[i]);
    free(queue->pcq_buffer);

    if (pthread_mutex_destroy(&queue->pcq_current_size_lock) != 0) {
        perror("Failed to destroy Mutex");
        return -1;
    }
    if (pthread_mutex_destroy(&queue->pcq_head_lock) != 0) {
        perror("Failed to destroy Mutex");
        return -1;
    }
    if (pthread_mutex_destroy(&queue->pcq_tail_lock) != 0) {
        perror("Failed to destroy Mutex");
        return -1;
    }
    if (pthread_mutex_destroy(&queue->pcq_pusher_condvar_lock) != 0) {
        perror("Failed to destroy Mutex");
        return -1;
    }
    if (pthread_cond_destroy(&queue->pcq_pusher_condvar) != 0) {
        perror("Failed to destroy condition variable");
        return -1;
    }
    if (pthread_mutex_destroy(&queue->pcq_popper_condvar_lock) != 0) {
        perror("Failed to destroy Mutex");
        return -1;
    }
    if (pthread_cond_destroy(&queue->pcq_popper_condvar) != 0) {
        perror("Failed to destroy condition variable");
        return -1;
    }

    return 0;
}

int pcq_enqueue(pc_queue_t *queue, void *elem) {
    if (pthread_mutex_lock(&queue->pcq_pusher_condvar_lock) != 0)
        exit(EXIT_FAILURE);

    while (queue->pcq_current_size == queue->pcq_capacity)
        pthread_cond_wait(&queue->pcq_pusher_condvar,
                          &queue->pcq_pusher_condvar_lock);
 
    queue->pcq_current_size += 1;
    queue->pcq_tail += 1;

    if (queue->pcq_tail == queue->pcq_capacity)
        queue->pcq_tail = 0;

    queue->pcq_buffer[queue->pcq_tail] = elem;

    if (pthread_mutex_unlock(&queue->pcq_pusher_condvar_lock) != 0)
        exit(EXIT_FAILURE);
    
    return 0;
}

void *pcq_dequeue(pc_queue_t *queue) {
    if (pthread_mutex_lock(&queue->pcq_popper_condvar_lock) != 0)
        exit(EXIT_FAILURE);

    while (queue->pcq_current_size == 0)
        pthread_cond_wait(&queue->pcq_popper_condvar,
                          &queue->pcq_popper_condvar_lock);

    queue->pcq_current_size -= 1;
    queue->pcq_head += 1;

    if (queue->pcq_head == queue->pcq_capacity)
        queue->pcq_head = 0;
    
    if (pthread_mutex_unlock(&queue->pcq_popper_condvar_lock) != 0)
        exit(EXIT_FAILURE);
    
    return NULL;
}