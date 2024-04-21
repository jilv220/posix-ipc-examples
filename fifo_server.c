#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct
{
    pid_t client_id;
    double nums[10];
} Request;

typedef struct
{
    double result;
} Response;

#define SERVER_FIFO "request_fifo"
#define CLIENT_FIFO_TEMPL "response_fifo.%d"
#define MAX_LENGTH 100

void remove_server_fifo()
{
    int status;
    status = unlink(SERVER_FIFO);
    if (status == -1)
        perror("unlink"), exit(EXIT_FAILURE);
}

void interrupt_handler()
{
    exit(EXIT_SUCCESS);
}

void *request_handler(void *input)
{
    int status;
    Request request;
    request = *(Request *)input;

    printf("Server received a request from client %d\n", request.client_id);

    status = pthread_mutex_lock(&mutex);
    if (status != 0)
        perror("mutex lock"), exit(EXIT_FAILURE);

    double sum;
    for (int i = 0; i < 10; i++)
    {
        sum += request.nums[i];
    }

    status = pthread_mutex_unlock(&mutex);
    if (status != 0)
        perror("mutex unlock"), exit(EXIT_FAILURE);

    Response response;
    response.result = sum / 10;

    char client_fifo_name[MAX_LENGTH];
    snprintf(client_fifo_name, MAX_LENGTH, CLIENT_FIFO_TEMPL, request.client_id);

    int response_fd;
    int nr_write;
    response_fd = open(client_fifo_name, O_WRONLY);
    if (response_fd == -1)
        perror("open response"), exit(EXIT_FAILURE);

    nr_write = write(response_fd, &response, sizeof(Response));
    if (nr_write < sizeof(Response))
    {
        fprintf(stderr, "partial write error");
        exit(EXIT_FAILURE);
    }

    status = close(response_fd);
    if (status == -1)
        perror("close response"), exit(EXIT_FAILURE);

    free(input);
    pthread_exit(NULL);
}

int main()
{
    int status;
    status = atexit(remove_server_fifo);
    if (status == -1)
        perror("atexit"), exit(EXIT_FAILURE);

    if (signal(SIGINT, interrupt_handler) == SIG_ERR)
        perror("register interrupt handler"), exit(EXIT_FAILURE);

    status = mkfifo(SERVER_FIFO, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if (status == -1)
        perror("create request fifo"), exit(EXIT_FAILURE);

    int request_fd, dummy_fd;
    request_fd = open(SERVER_FIFO, O_RDONLY);
    if (request_fd == -1)
        perror("open request fd"), exit(EXIT_FAILURE);

    dummy_fd = open(SERVER_FIFO, O_WRONLY);
    if (dummy_fd == -1)
        perror("open dummy fd"), exit(EXIT_FAILURE);

    Request request;
    pthread_t thread;
    while (1)
    {
        int nr_read;
        nr_read = read(request_fd, &request, sizeof(Request));
        if (nr_read == -1)
            perror("read request"), exit(EXIT_FAILURE);

        Request *request_handler_input = malloc(sizeof(Request));
        if (request_handler_input == NULL)
            perror("malloc"), exit(EXIT_FAILURE);
        memcpy(request_handler_input, &request, sizeof(Request));

        status = pthread_create(&thread, NULL, request_handler, (void *)request_handler_input);
        if (status != 0)
            perror("pthread create"), exit(EXIT_FAILURE);

        status = pthread_detach(thread);
        if (status != 0)
            perror("pthread detach"), exit(EXIT_FAILURE);
    }
}