#include "../inc/threadpool.h"
#include <stdbool.h>
#include <string.h>

void threadpool_init(threadpool_t* pool){
	pool->queued = 0;
	pool->queue_back = 0;
	pool->queue_front = 0;
	pool->stop = 0;
	pthread_mutex_init(&pool->lock, NULL);
	pthread_cond_init(&pool->notify, NULL);
	sem_init(&pool->available, 0, QUEUE_SIZE);
	for(int i = 0; i < THREADS; i++){
		pthread_create(&pool->threads[i], NULL, thread_function, pool);
	}
}

void threadpool_destroy(threadpool_t* pool){
	pthread_mutex_lock(&pool->lock);
	pool->stop = true;
	pthread_cond_broadcast(&pool->notify);
	pthread_mutex_unlock(&pool->lock);
	sem_destroy(&pool->available);
	for(int i = 0; i < THREADS; i++){
		pthread_join(pool->threads[i], NULL);
	}
	pthread_mutex_destroy(&pool->lock);
	pthread_cond_destroy(&pool->notify);
}

void threadpool_add_task(threadpool_t* pool, void (*function)(void*), void* arg){
	sem_wait(&pool->available);		// Can go outside the mutex (is atomic)
	pthread_mutex_lock(&pool->lock);
	pool->queued++;
	pool->task_queue[pool->queue_back].fn = function;
	pool->task_queue[pool->queue_back].arg = arg;
	pool->queue_back = (pool->queue_back+1) % QUEUE_SIZE;
	pthread_cond_signal(&pool->notify);
	pthread_mutex_unlock(&pool->lock);
}

void* thread_function(void* arg){
	threadpool_t* pool = (threadpool_t*)arg;
	while(true){
		pthread_mutex_lock(&pool->lock);
		while(pool->queued == 0 && !pool->stop){
			pthread_cond_wait(&pool->notify, &pool->lock);
		}
		if(pool->queued == 0 && pool->stop){
			pthread_mutex_unlock(&pool->lock);
			break;
		}
		void (*task_func)(void*) = pool->task_queue[pool->queue_front].fn;
		void* task_arg = (pool->task_queue[pool->queue_front].arg);
		pool->queue_front = (pool->queue_front+1) % QUEUE_SIZE;
		pool->queued--;
		sem_post(&pool->available);
		pthread_mutex_unlock(&pool->lock);		// Unlock mutex before running task
		task_func(task_arg);
	}
	return NULL;
}