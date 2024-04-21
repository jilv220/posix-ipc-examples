#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    int child_id;
    int task_id;
} Task;

int main(int argc, char *argv[])
{
    if (argc != 3)
        fprintf(stderr, "Usage: %s n-children m-tasks\n", argv[0]), exit(EXIT_FAILURE);

    int status;

    int child_cnt = atoi(argv[1]);
    int task_cnt = atoi(argv[2]);

    int pipe_fds[2];
    status = pipe(pipe_fds);
    if (status == -1)
        perror("pipe"), exit(EXIT_FAILURE);

    int read_end = pipe_fds[0];
    int write_end = pipe_fds[1];

    pid_t child_pid;
    for (int i = 0; i < child_cnt; i++)
    {
        child_pid = fork();
        switch (child_pid)
        {
        case -1:
            perror("fork"), exit(EXIT_FAILURE);
        case 0:
            status = close(read_end);
            if (status == -1)
                perror("child close read end"), _exit(EXIT_FAILURE);

            for (int j = 0; j < task_cnt; j++)
            {
                Task task;
                task.child_id = i;
                task.task_id = j;

                int nr_write;
                nr_write = write(write_end, &task, sizeof(Task));
                if (nr_write < sizeof(Task))
                {
                    fprintf(stderr, "child partial write error\n");
                    _exit(EXIT_FAILURE);
                }
                printf("Child %d has finished Task %d\n", i, j);

                sleep(1);
            }

            status = close(write_end);
            if (status == -1)
                perror("child close write end"), _exit(EXIT_FAILURE);

            _exit(EXIT_SUCCESS);
        default:
            break;
        }
    }

    // parent pipe
    status = close(write_end);
    if (status == -1)
        perror("parent close write end"), exit(EXIT_FAILURE);

    int *bucket = (int *)calloc(task_cnt, sizeof(int));
    for (int i = 0; i < child_cnt; i++)
    {
        Task task;
        for (int j = 0; j < task_cnt; j++)
        {
            int nr_read;
            nr_read = read(read_end, &task, sizeof(Task));
            if (nr_read == -1)
                perror("parent read"), exit(EXIT_FAILURE);

            bucket[task.task_id] += 1;
            // printf("task: %d, cnt: %d\n", task.task_id, bucket[task.task_id]);

            if (bucket[task.task_id] == child_cnt)
            {
                printf("All children have finished Task %d\n", task.task_id);
            }
        }
    }

    status = close(read_end);
    if (status == -1)
        perror("parent close read end"), exit(EXIT_FAILURE);

    // parent wait
    pid_t terminated;
    int wait_status;
    for (int i = 0; i < child_cnt; i++)
    {
        terminated = wait(&wait_status);
        if (terminated == -1)
            perror("wait"), exit(EXIT_FAILURE);
    }

    free(bucket);
}