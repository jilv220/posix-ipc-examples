#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

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

void remove_response_fifo()
{
    int status;
    char client_fifo_name[MAX_LENGTH];
    snprintf(client_fifo_name, MAX_LENGTH, CLIENT_FIFO_TEMPL, getpid());

    status = unlink(client_fifo_name);
    if (status == -1)
        perror("unlink"), exit(EXIT_FAILURE);
}

int main()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srandom(tv.tv_sec ^ tv.tv_usec);

    int status;
    status = atexit(remove_response_fifo);
    if (status == -1)
        perror("atexit"), exit(EXIT_FAILURE);

    char client_fifo_name[MAX_LENGTH];
    snprintf(client_fifo_name, MAX_LENGTH, CLIENT_FIFO_TEMPL, getpid());
    status = mkfifo(client_fifo_name, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if (status == -1)
        perror("mkfifo"), exit(EXIT_FAILURE);

    Request request;
    for (int i = 0; i < 10; i++)
    {
        request.client_id = getpid();
        request.nums[i] = (double)random() / (double)RAND_MAX;
    }

    int request_fd;
    request_fd = open(SERVER_FIFO, O_WRONLY);
    if (request_fd == -1)
        perror("open server fifo"), exit(EXIT_FAILURE);

    int nr_write;
    nr_write = write(request_fd, &request, sizeof(Request));
    if (nr_write < sizeof(Request))
    {
        fprintf(stderr, "partial write error\n");
        exit(EXIT_FAILURE);
    }

    status = close(request_fd);
    if (status == -1)
        perror("close request"), exit(EXIT_FAILURE);

    int response_fd;
    response_fd = open(client_fifo_name, O_RDONLY);
    if (response_fd == -1)
        perror("open client fifo"), exit(EXIT_FAILURE);

    Response response;
    int nr_read;
    nr_read = read(response_fd, &response, sizeof(Response));
    if (nr_read == -1)
        perror("read client fifo"), exit(EXIT_FAILURE);

    printf("[%d] The result is %f\n", getpid(), response.result);

    status = close(response_fd);
    if (status == -1)
        perror("close response"), exit(EXIT_FAILURE);
}