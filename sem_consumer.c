#include <semaphore.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define SEM_MUTEX "/sem_mutex"
#define SEM_SLOT_EMPTY "/sem_slot_empty"
#define SEM_SLOT_FILLED "/sem_slot_filled"
#define SHM "/pc_shm"

int main()
{
    int status;
    sem_t *mutex, *slot_empty, *slot_filled;

    mutex = sem_open(SEM_MUTEX, 0);
    if (mutex == SEM_FAILED)
        perror("sem open mutex"), exit(EXIT_FAILURE);

    slot_empty = sem_open(SEM_SLOT_EMPTY, 0);
    if (slot_empty == SEM_FAILED)
        perror("sem open slot empty"), exit(EXIT_FAILURE);

    slot_filled = sem_open(SEM_SLOT_FILLED, 0);
    if (slot_filled == SEM_FAILED)
        perror("sem open slot filled"), exit(EXIT_FAILURE);

    int shm_fd;
    shm_fd = shm_open(SHM, O_RDWR, 0);
    if (shm_fd == -1)
        perror("shm open"), exit(EXIT_FAILURE);

    struct stat shm_meta;
    status = fstat(shm_fd, &shm_meta);
    if (status == -1)
        perror("fstat"), exit(EXIT_FAILURE);

    void *addr;
    addr = mmap(NULL, shm_meta.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (addr == MAP_FAILED)
        perror("mmap"), exit(EXIT_FAILURE);
    status = close(shm_fd);
    if (status == -1)
        perror("close shm fd"), exit(EXIT_FAILURE);

    int *n_msg_ptr = (int *)addr;
    while (1)
    {
        status = sem_wait(slot_filled);
        if (status != 0)
            perror("sem wait slot filled"), exit(EXIT_FAILURE);

        status = sem_wait(mutex);
        if (status != 0)
            perror("sem lock"), exit(EXIT_FAILURE);

        *n_msg_ptr -= 1;
        printf("consumer has decreased number of msgs to %d\n", *n_msg_ptr);

        status = sem_post(mutex);
        if (status != 0)
            perror("sem unlock"), exit(EXIT_FAILURE);

        status = sem_post(slot_empty);
        if (status != 0)
            perror("sem post slot empty"), exit(EXIT_FAILURE);

        sleep(2);
    }
}