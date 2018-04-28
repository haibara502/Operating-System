#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>

pthread_mutex_t god;
pthread_cond_t can_cat;
pthread_cond_t can_dog;
pthread_cond_t can_bird;

volatile int num_cat = 0, num_dog = 0, num_bird = 0;
volatile int cannot;
volatile int current_cat = 0, current_dog = 0, current_bird = 0;

struct thread_data
{
    int number, count;
};

void play()
{
    for (int i = 0; i < 10; ++i)
    {
	assert(current_cat >= 0 && current_cat <= num_cat);
	assert(current_dog >= 0 && current_dog <= num_dog);
	assert(current_bird >= 0 && current_bird <= num_bird);
	assert(current_cat == 0 || current_dog == 0);
	assert(current_cat == 0 || current_bird == 0);
    }
}


void enter_cat(int number)
{
    pthread_mutex_lock(&god);
    while (current_dog > 0 || current_bird > 0)
	pthread_cond_wait(&can_cat, &god);
    ++current_cat;
    pthread_mutex_unlock(&god);
}

void exit_cat(int number)
{
    pthread_mutex_lock(&god);
    --current_cat;
    if (current_cat == 0)
    {
	pthread_cond_broadcast(&can_dog);
	pthread_cond_broadcast(&can_bird);
    }
    pthread_mutex_unlock(&god);
}
void enter_dog(int number)
{
    pthread_mutex_lock(&god);
    while (current_cat > 0)
	pthread_cond_wait(&can_dog, &god);
    ++current_dog;
    pthread_mutex_unlock(&god);
}

void exit_dog(int number)
{
    pthread_mutex_lock(&god);
    --current_dog;
    if (current_dog == 0 && current_bird == 0)
	pthread_cond_broadcast(&can_cat);
    pthread_mutex_unlock(&god);
}
void enter_bird(int number)
{
    pthread_mutex_lock(&god);
    while (current_cat > 0)
	pthread_cond_wait(&can_bird, &god);
    ++current_bird;
    pthread_mutex_unlock(&god);
}

void exit_bird(int number)
{
    pthread_mutex_lock(&god);
    --current_bird;
    if (current_bird == 0 && current_dog == 0)
    {
	pthread_cond_broadcast(&can_cat);
    }
    pthread_mutex_unlock(&god);
}

void *run_cat(void *arg)
{
    struct thread_data *data = (struct thread_data *) arg;
    int number = data -> number;
    int count = 0;
    while (cannot)
    {
	count = count + 1;
	enter_cat(number);
	play();
	exit_cat(number);
    }

    data -> count = count;
    pthread_exit(NULL);
}

void *run_dog(void *arg)
{
    struct thread_data *data = (struct thread_data *) arg;
    int number = data -> number;
    int count = 0;
    while (cannot)
    {
	count = count + 1;
	enter_dog(number);
	play();
	exit_dog(number);
    }

    data -> count = count;
    pthread_exit(NULL);
}
void *run_bird(void *arg)
{
    struct thread_data *data = (struct thread_data *) arg;
    int number = data -> number;
    int count = 0;
    while (cannot)
    {
	count = count + 1;
	enter_bird(number);
	play();
	exit_bird(number);
    }

    data -> count = count;
    pthread_exit(NULL);
}
int main(int argc, char *argv[])
{
    if (argc != 4)
    {
	fprintf(stderr, "Wrong input!\n");
	return EXIT_FAILURE;
    }

    int cats = atoi(argv[1]);
    int dogs = atoi(argv[2]);
    int birds = atoi(argv[3]);


    num_cat = cats;
    num_dog = dogs;
    num_bird = birds;

    if ((cats < 0) || (cats > 99) || (dogs < 0) || (dogs > 99) || (birds < 0) || (birds > 99))
    {
	fprintf(stderr, "The number of the animals is invalid!\n");
	return EXIT_FAILURE;
    }

    pthread_t thread_cat[cats];
    pthread_t thread_dog[dogs];
    pthread_t thread_bird[birds];

    struct thread_data data_cat[cats];
    struct thread_data data_dog[dogs];
    struct thread_data data_birds[birds];

    cannot = 1;

    if (pthread_mutex_init(&god, NULL) != 0)
    {
	perror("Error occurs when initing the mutex.\n");
	exit(1);
    }

    if (pthread_cond_init(&can_cat, NULL) != 0)
    {
	perror("Error occurs when initing the condition variable for cats.\n");
	exit(1);
    } else if (pthread_cond_init(&can_dog, NULL) != 0)
    {
	perror("Error occurs when initing the cond variable for dogs.\n");
	exit(1);
    } else if (pthread_cond_init(&can_bird, NULL) != 0)
    {
	perror("Error occurs when initing the cond variable for birds. \n");
	exit(1);
    }

    int i;
    int status;

    for (i = 0; i < cats; ++i)
    {
	data_cat[i].number = i;
	status = pthread_create(&thread_cat[i], NULL, run_cat, &data_cat[i]);
	if (status != 0)
	{
	    fprintf(stderr, "Error occurs when create the %dth thread of cats.\n", i);
	    break;
	}
    }

    for (i = 0; i < dogs; ++i)
    {
	data_dog[i].number = i;
	data_dog[i].count = 0;
	status = pthread_create(&thread_dog[i], NULL, run_dog, &data_dog[i]);
	if (status != 0)
	{
	    fprintf(stderr, "Error occurs when create the %dth thread of dogs.\n", i);
	    break;
	}
    }

    for (i = 0; i < birds; ++i)
    {
	data_birds[i].number = i;
	status = pthread_create(&thread_bird[i], NULL, run_bird, &data_birds[i]);
	if (status != 0)
	{
	    fprintf(stderr, "Error occurs when create the %dth thread of birds.\n", i);
	    break;
	}
    }

    sleep(1);

    cannot = 0;
    int ans_cat = 0;
    for (i = 0; i < cats; ++i)
    {
	pthread_join(thread_cat[i], NULL);
	ans_cat += data_cat[i].count;
    }

    int ans_dog = 0;
    for (i = 0; i < dogs; ++i)
    {
	pthread_join(thread_dog[i], NULL);
	ans_dog += data_dog[i].count;
    }

    int ans_bird = 0;
    for (i = 0; i < birds; ++i)
    {
	pthread_join(thread_bird[i], NULL);
	ans_bird += data_birds[i].count;
    }

    printf("Cats play for %d times.\n", ans_cat);
    printf("Dogs play for %d times. \n", ans_dog);
    printf("Birds play for %d times. \n", ans_bird);

    return EXIT_SUCCESS;
}
