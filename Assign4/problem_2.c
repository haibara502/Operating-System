#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>

volatile int in_cs = 0;
volatile int can = 0;
volatile int thread_number = 0;
volatile int wait_time = 0;

volatile int *enter;
volatile int *num;

int max()
{
    int ans = 0;
    for (int i = 0; i < thread_number; ++i)
	if (num[i] > ans)
	    ans = num[i];
    return ans;
}

void unlock(int number)
{
    num[number] = 0;
}

void lock(int number)
{
    enter[number] = 1;
    num[number] = 1 + max();
    enter[number] = 0;

    for (int j = 0; j < thread_number; ++j)
    {
	while (enter[j])
	    sched_yield();
	while (num[j] != 0 && ((num[j] < num[number] || (num[j] == num[number] && j < number))))
	    sched_yield();
    }
}


void *run_thread(void *arg)
{
    int *data = (int *)arg;

    int id = *data;
    int count = 1;
    while (can)
    {
	++count;

	lock(id);
	assert(in_cs == 0);
	++in_cs;
	assert(in_cs == 1);
	++in_cs;
	assert(in_cs == 2);
	++in_cs;
	assert(in_cs == 3);
	in_cs = 0;
	unlock(id);
    }

    printf("Thread %d enter %d times.\n", id, count);
    pthread_exit(NULL);
}

void *main_function()
{
    pthread_t threads[thread_number];
    int thread_num[thread_number];

    enter = malloc(thread_number * sizeof(*enter));
    num = malloc(thread_number * sizeof(*index));
    for (int i = 0; i < thread_number; ++i)
    {
	enter[i] = 0;
	num[i] = 0;
    }

    for (int i = 0; i < thread_number; ++i)
    {
	thread_num[i] = i;
	int status = pthread_create(&threads[i], NULL, run_thread, &thread_num[i]);
	if (status != 0)
	{
	    fprintf(stderr, "Error when creating the %dth thread.\n", i);
	    break;
	}
    }

    sleep(wait_time);
    can = 0;
    for (int i = 0; i < thread_number; ++i)
	pthread_join(threads[i], NULL);

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) 
{
    if (argc != 3)
    {
	fprintf(stderr, "The input argument is not valid. You should input the number of thread and the execution time.\n");
	return EXIT_FAILURE;
    }

    thread_number = atoi(argv[1]);
    wait_time = atoi(argv[2]);

    if ((thread_number < 0) || (wait_time < 0) || (thread_number > 99))
    {
	fprintf(stderr, "Input number is invalid. \n");
	return EXIT_FAILURE;
    }

    can = 1;

    pthread_t main_thread;

    int status = pthread_create(&main_thread, NULL, main_function, NULL);
    if (status != 0)
    {
	fprintf(stderr, "Error when create the main thread.\n");
	return EXIT_FAILURE;
    }

    pthread_join(main_thread, NULL);

    return EXIT_SUCCESS;
}
