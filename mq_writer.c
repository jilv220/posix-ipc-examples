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
    struct mq_attr attr;
    attr.mq_maxmsg = 5;
    attr.mq_msgsize = sizeof(Person);

    Person persons[5] = {
        {"Mary", "Mari"}, {"Hifumi", "lol"}, {"kasane", "Teto"}, {"wfef", "lmao"}, {"hatsune", "miku"}};

    mqd = mq_open("/mqueue", O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR, &attr);
    if (mqd == (mqd_t)-1)
        perror("mq create"), exit(EXIT_FAILURE);

    for (int i = 0; i < 5; i++)
    {
        status = mq_send(mqd, (char *)&persons[i], sizeof(Person), i);
        if (status == -1)
            perror("mq send"), exit(EXIT_FAILURE);
    }

    status = mq_close(mqd);
    if (status == -1)
        perror("mq close"), exit(EXIT_FAILURE);
}