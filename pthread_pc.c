#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 5

volatile int num_of_units = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t unit_available = PTHREAD_COND_INITIALIZER;
pthread_cond_t space_available = PTHREAD_COND_INITIALIZER;

void *producer_handler(void *data)
{
    int status;
    while (1)
    {
        status = pthread_mutex_lock(&mutex);
        if (status != 0)
            perror("producer lock"), exit(EXIT_FAILURE);

        while (num_of_units >= MAX_BUFFER_SIZE)
        {
            printf("producer is about to block\n");
            status = pthread_cond_wait(&space_available, &mutex);
            if (status != 0)
                perror("producer cond wait"), exit(EXIT_FAILURE);
        }

        num_of_units++;
        printf("producer increased number of units to %d\n", num_of_units);

        if (num_of_units == 1)
        {
            status = pthread_cond_signal(&unit_available);
            if (status != 0)
                perror("producer cond signal"), exit(EXIT_FAILURE);
        }

        status = pthread_mutex_unlock(&mutex);
        if (status != 0)
            perror("producer unlock"), exit(EXIT_FAILURE);

        sleep(1);
    }

    return NULL;
}

void *consumer_handler(void *data)
{
    int status;

    while (1)
    {
        status = pthread_mutex_lock(&mutex);
        if (status != 0)
            perror("consumer lock"), exit(EXIT_FAILURE);

        while (num_of_units <= 0)
        {
            printf("consumer is about to block\n");
            status = pthread_cond_wait(&unit_available, &mutex);
            if (status != 0)
                perror("consumer wait"), exit(EXIT_FAILURE);
        }

        num_of_units--;
        printf("consumer reduced the number of units to %d\n", num_of_units);

        if (num_of_units == MAX_BUFFER_SIZE - 1)
        {
            status = pthread_cond_signal(&space_available);
            if (status != 0)
                perror("consumer signal"), exit(EXIT_FAILURE);
        }

        status = pthread_mutex_unlock(&mutex);
        if (status != 0)
            perror("consumer unlock"), exit(EXIT_FAILURE);

        sleep(2);
    }

    return NULL;
}

int main()
{
    int status;

    pthread_t thread1, thread2;
    status = pthread_create(&thread1, NULL, producer_handler, NULL);
    if (status != 0)
    {
        perror("thread1 create");
        exit(EXIT_FAILURE);
    }

    status = pthread_create(&thread2, NULL, consumer_handler, NULL);
    if (status != 0)
    {
        perror("thread2 create");
        exit(EXIT_FAILURE);
    }

    void *result;
    status = pthread_join(thread1, &result);
    if (status != 0)
    {
        perror("thread1 join");
        exit(EXIT_FAILURE);
    }

    status = pthread_join(thread2, &result);
    if (status != 0)
    {
        perror("thread2 join");
        exit(EXIT_FAILURE);
    }
}