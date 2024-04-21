#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    int read_end;
    int write_end;
} Pipe;

void mkpipe(int *read_end, int *write_end)
{
    int pipe_fds[2];
    int status;
    status = pipe(pipe_fds);
    if (status == -1)
        perror("pipe"), exit(EXIT_FAILURE);

    *read_end = pipe_fds[0];
    *write_end = pipe_fds[1];
}

int main(int argc, char *argv[])
{
    if (argc != 3)
        fprintf(stderr, "Usage: %s n-children m-tasks\n", argv[0]), exit(EXIT_FAILURE);

    int status;

    int n = atoi(argv[1]); // child cnt
    int m = atoi(argv[2]); // task cnt

    Pipe *pipes = (Pipe *)malloc(sizeof(Pipe) * (m - 1));
    if (pipes == NULL)
        perror("malloc"), exit(EXIT_FAILURE);

    for (int i = 0; i < m - 1; i++)
    {
        mkpipe(&pipes[i].read_end, &pipes[i].write_end);
    }

    pid_t child_pid;
    for (int i = 0; i < n; i++)
    {
        child_pid = fork();
        switch (child_pid)
        {
        case -1:
            perror("fork"), exit(EXIT_FAILURE);
        case 0:
            for (int j = 0; j < m; j++)
            {
                status = close(pipes[j].read_end);
                if (status == -1)
                    perror("child close"), _exit(EXIT_FAILURE);

                printf("Child %d finished Task %d\n", i, j);

                if (j < m - 1)
                {
                    status = close(pipes[j].write_end);
                    if (status == -1)
                        perror("child close write end"), _exit(EXIT_FAILURE);
                }
                sleep(1);
            }

            _exit(EXIT_SUCCESS);
        default:
            break;
        }
    }

    // parent pipe
    for (int i = 0; i < m - 1; i++)
    {
        status = close(pipes[i].write_end);
        if (status == -1)
            perror("parent close write end"), exit(EXIT_FAILURE);

        char buf[1];
        int nr_read;
        nr_read = read(pipes[i].read_end, &buf, sizeof(char));
        if (nr_read != 0)
        {
            fprintf(stderr, "parent failed to get EOF from child\n");
            exit(EXIT_FAILURE);
        }

        printf("All children has completed Task %d\n\n", i);

        status = close(pipes[i].read_end);
        if (status == -1)
            perror("parent close read end"), exit(EXIT_FAILURE);
    }

    // parent wait
    // serves as the ipc for the last task
    pid_t terminated;
    int wait_status;
    for (int i = 0; i < n; i++)
    {
        terminated = wait(&wait_status);
        if (terminated == -1)
            perror("wait"), exit(EXIT_FAILURE);
    }
    printf("All children has completed Task %d\n", m - 1);

    free(pipes);
}