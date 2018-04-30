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

struct spin_lock
{
    volatile int enter;
    volatile int wait;
} god;

int atomic_xadd(volatile int *ptr)
{
    register int val __asm__("eax") = 1;
    asm volatile("lock xaddl %0, %1" : "+r" (val) : "m" (*ptr) : "memory" );
    return val;
}

//Lock
void run_spin_lock(struct spin_lock *sl)
{
    int wait = atomic_xadd(&sl -> wait);
    while (wait != sl -> enter);
}

//Unlock
void run_spin_unlock(struct spin_lock *sl)
{
    atomic_xadd(&sl -> enter);
}

void *run_thread(void *arg)
{
    int *data = (int *)arg;

    int id = *data;
    int count = 1;

    //Use spin lock
    while (can)
    {
	++count;

	run_spin_lock(&god);
	assert(in_cs == 0);
	++in_cs;
	assert(in_cs == 1);
	++in_cs;
	assert(in_cs == 2);
	++in_cs;
	assert(in_cs == 3);
	in_cs = 0;
	run_spin_unlock(&god);
    }

    printf("Thread %d enter %d times.\n", id, count);
    pthread_exit(NULL);
}

void *main_function()
{
    pthread_t threads[thread_number];
    int thread_num[thread_number];

    //Prepare to store the data
    enter = malloc(thread_number * sizeof(*enter));
    num = malloc(thread_number * sizeof(*index));
    for (int i = 0; i < thread_number; ++i)
    {
	enter[i] = 0;
	num[i] = 0;
    }

    //Create the threads
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
    // Check input valid
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

    // Main thread
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
