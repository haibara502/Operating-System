#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>

volatile int total_circle = 0, total_inside_circle = 0;

volatile int thread_number = 0;
volatile int point_number = 0;

struct spin_lock
{
    volatile int enter, wait;
} god;

struct thread_data
{
    volatile int count;
};

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

void *run_thread()
{
    int inside_circle = 0;
    int total = point_number;

    //Generate the points
    while (total--)
    {
	double x = (rand() * 1.0 / RAND_MAX) * 2 - 1;
	double y = (rand() * 1.0 / RAND_MAX) * 2 - 1;
	if (x * x + y * y <= 1)
	    ++inside_circle;
    }

    //Use mutex to add the total number
    run_spin_lock(&god);
    total_inside_circle += inside_circle;
    run_spin_unlock(&god);

    pthread_exit(NULL);
}

void *main_function()
{
    pthread_t threads[thread_number];

    //Create the threads
    for (int i = 0; i < thread_number; ++i)
    {
	int status = pthread_create(&threads[i], NULL, run_thread, NULL);
	if (status != 0)
	{
	    fprintf(stderr, "Error when creating the %dth thread.\n", i);
	    break;
	}
    }
    
    //Collect the threads
    for (int i = 0; i < thread_number; ++i)
	pthread_join(threads[i], NULL);

    printf("There are %d points.\n", thread_number * point_number);
    printf("%d are inside the circle.\n", total_inside_circle);
    fprintf(stderr, "Pi is close to %f\n", 4.0 * total_inside_circle / thread_number / point_number);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) 
{
    //Check input
    if (argc != 3)
    {
	fprintf(stderr, "The input argument is not valid. You should first input the number of threads. And then the number of points generated.\n");
	return EXIT_FAILURE;
    }

    thread_number = atoi(argv[1]);
    point_number = atoi(argv[2]);

    if ((thread_number < 0) || (point_number < 0) || (thread_number > 99))
    {
	fprintf(stderr, "Input number is invalid. \n");
	return EXIT_FAILURE;
    }

    //Initialize
    god.enter = 0;
    god.wait = 0;

    pthread_t main_thread;

    int status = pthread_create(&main_thread, NULL, main_function, NULL);
    if (status != 0)
    {
	fprintf(stderr, "Error when create the main thread.\n");
	return EXIT_FAILURE;
    }

    //Main thread
    pthread_join(main_thread, NULL);

    return EXIT_SUCCESS;
}
