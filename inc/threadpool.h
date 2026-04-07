#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <pthread.h>
#include <stdatomic.h>
#include <semaphore.h>

#define THREADS 16
#define QUEUE_SIZE 100

// Single task to execute by thread pool
typedef struct {
  void (*fn)(void* arg);
  void* arg;
} task_t;

// Thread pool
typedef struct {
  pthread_mutex_t lock;		// Lock the pool
  pthread_cond_t notify;	// Notify the queue to run a task
  pthread_t threads[THREADS];
  task_t task_queue[QUEUE_SIZE];
  sem_t	available;
  int queued;
  int queue_front;
  int queue_back;
  int stop;					// Stop the thread pool
} threadpool_t;

// Function declarations
void threadpool_init(threadpool_t* pool);
void threadpool_destroy(threadpool_t* pool);
void threadpool_add_task(threadpool_t* pool, void (*function)(void*), void* arg);
void* thread_function(void* arg);

#endif