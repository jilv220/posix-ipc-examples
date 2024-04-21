#include <semaphore.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define SLOT_EMPTY 1024
#define SLOT_FILLED 0
#define SEM_MUTEX "/sem_mutex"
#define SEM_SLOT_EMPTY "/sem_slot_empty"
#define SEM_SLOT_FILLED "/sem_slot_filled"
#define SHM "/pc_shm"

int main()
{
    int status;
    sem_t *mutex, *slot_empty, *slot_filled;

    mutex = sem_open(SEM_MUTEX, O_CREAT, S_IRUSR | S_IWUSR, 1);
    if (mutex == SEM_FAILED)
        perror("sem create mutex"), exit(EXIT_FAILURE);

    slot_empty = sem_open(SEM_SLOT_EMPTY, O_CREAT, S_IRUSR | S_IWUSR, SLOT_EMPTY);
    if (slot_empty == SEM_FAILED)
        perror("sem create slot empty"), exit(EXIT_FAILURE);

    slot_filled = sem_open(SEM_SLOT_FILLED, O_CREAT, S_IRUSR | S_IWUSR, SLOT_FILLED);
    if (slot_filled == SEM_FAILED)
        perror("sem create mutex"), exit(EXIT_FAILURE);

    int shm_fd;
    shm_fd = shm_open(SHM, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (shm_fd == -1)
        perror("shm create"), exit(EXIT_FAILURE);
    status = ftruncate(shm_fd, sizeof(int));
    if (status == -1)
        perror("ftruncate"), exit(EXIT_FAILURE);

    void *addr;
    addr = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (addr == MAP_FAILED)
        perror("mmap"), exit(EXIT_FAILURE);
    status = close(shm_fd);
    if (status == -1)
        perror("close shm fd"), exit(EXIT_FAILURE);

    int *n_msgs_ptr = (int *)addr;
    while (1)
    {
        status = sem_wait(slot_empty);
        if (status != 0)
            perror("sem wait slot empty"), exit(EXIT_FAILURE);

        status = sem_wait(mutex);
        if (status != 0)
            perror("sem lock"), exit(EXIT_FAILURE);
        //
        *n_msgs_ptr += 1;
        printf("producer has increased msgs to %d\n", *n_msgs_ptr);

        status = sem_post(mutex);
        if (status != 0)
            perror("sem unlock"), exit(EXIT_FAILURE);

        status = sem_post(slot_filled);
        if (status != 0)
            perror("sem post slot filled"), exit(EXIT_FAILURE);

        sleep(1);
    }
}