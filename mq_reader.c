#include <mqueue.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

typedef struct
{
    char firstName[20];
    char lastName[20];
} Person;

int main()
{
    int status;

    mqd_t mqd;
    mqd = mq_open("/mqueue", O_RDONLY);
    if (mqd == (mqd_t)-1)
        perror("mq open"), exit(EXIT_FAILURE);

    struct mq_attr attr;
    status = mq_getattr(mqd, &attr);

    int priority;
    int nr_read;
    long n_msgs = attr.mq_curmsgs;

    Person person;
    for (int i = 0; i < n_msgs; i++)
    {
        nr_read = mq_receive(mqd, (char *)&person, attr.mq_msgsize, &priority);
        if (nr_read == -1)
            perror("mq receive"), exit(EXIT_FAILURE);

        printf("firstName: %s, lastName: %s\n", person.firstName, person.lastName);
    }

    status = mq_close(mqd);
    if (status == -1)
        perror("mq close"), exit(EXIT_FAILURE);
}