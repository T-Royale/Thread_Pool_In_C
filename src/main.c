#include "../inc/threadpool.h"
#include <stdatomic.h>
#include <stdio.h>

#define COUNT_TARGET 500

#if __STDC_NO_ATOMICS__ 
	#error "NO ATOMICS"
#endif

void task_printmsg(void* arg){
	char *str = (char*)arg;
	printf("%s\n", str);
}

void task_addone(void* arg){
	atomic_int* count = (atomic_int*)arg;
	(*count)++;
}

int main(int argc, char *argv[]) {
	threadpool_t pool;
	char *messages[] = {"Hello, world", "Ey, world", "Yo, world!", "Bye, world!", NULL};
	threadpool_init(&pool);
	for(int i = 0; messages[i] != NULL; i++){
		while(threadpool_add_task(&pool, task_printmsg, messages[i]) == 1);		// Queue could overflow and return 1
	}
	atomic_int count = 0;
	for(int i = 0; i < COUNT_TARGET; i++){
		while(threadpool_add_task(&pool, task_addone, &count) == 1);	
	}
	threadpool_destroy(&pool);
	if(count == COUNT_TARGET){
		printf("Count target reached\n");
	} else printf("Count target failed, was %d\n", count);
	return 0; 
}