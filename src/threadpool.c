#include "../inc/threadpool.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

void threadpool_init(threadpool_t* pool){
	pool->queued = 0;
	pool->queue_back = 0;
	pool->queue_front = 0;
	pool->stop = 0;
	pthread_mutex_init(&pool->lock, NULL);
	pthread_cond_init(&pool->notify, NULL);
	for(int i = 0; i < THREADS; i++){
		pthread_create(&pool->threads[i], NULL, thread_function, pool);
	}
}

void threadpool_destroy(threadpool_t* pool){
	pthread_mutex_lock(&pool->lock);
	pool->stop = true;
	pthread_cond_broadcast(&pool->notify);
	pthread_mutex_unlock(&pool->lock);
	for(int i = 0; i < THREADS; i++){
		pthread_join(pool->threads[i], NULL);
	}
	pthread_mutex_destroy(&pool->lock);
	pthread_cond_destroy(&pool->notify);
}

void threadpool_add_task(threadpool_t* pool, void (*function)(void*), void* arg){
	pthread_mutex_lock(&pool->lock);
	pool->queued++;
	if(pool->queued >= QUEUE_SIZE){
		fprintf(stderr, "ERROR: QUEUE LIMIT REACHED\n");
		pool->queued--;
		return;
	}
	pool->task_queue[pool->queue_back].fn = function;
	pool->task_queue[pool->queue_back].arg = arg;
	pool->queue_back = (pool->queue_back+1) % QUEUE_SIZE;
	pthread_cond_signal(&pool->notify);
	pthread_mutex_unlock(&pool->lock);
}

void example_task(void* arg){
	char *str = (char*)arg;
	printf("ARG: %s\n", str);
}

void* thread_function(void* arg){
	threadpool_t* pool = (threadpool_t*)arg;
	pthread_mutex_lock(&pool->lock);
	while(true){
		while(pool->queued == 0 && !pool->stop){
			pthread_cond_wait(&pool->notify, &pool->lock);
		}
		if(pool->queued == 0 && pool->stop){
			pthread_mutex_unlock(&pool->lock);
			break;
		}
		(pool->task_queue[pool->queue_front].fn)(pool->task_queue[pool->queue_front].arg);
		pool->queue_front = (pool->queue_front+1) % QUEUE_SIZE;
		pool->queued--;
	}
	return NULL;
}