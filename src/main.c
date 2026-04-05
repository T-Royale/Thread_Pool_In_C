#include "../inc/threadpool.h"

int main(int argc, char *argv[]) {
	threadpool_t pool;
	threadpool_init(&pool);
	threadpool_add_task(&pool, example_task, "Hello, world!");
	threadpool_add_task(&pool, example_task, "Ey, world!");
	threadpool_add_task(&pool, example_task, "Yo, world!");
	threadpool_add_task(&pool, example_task, "Bye, world!");
	threadpool_destroy(&pool);
	return 0; 
}