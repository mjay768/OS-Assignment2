#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<sys/ipc.h>
#include<sys/shm.h>

#include "time.h"

int main(int* argc, char * argv[])
{
    uint dur = atoi(argv[1]);
    struct time *shm;
    /* Get shared memory*/
    key_t key;
    int shmid = shmget(key, sizeof(struct time), 0666 | IPC_CREAT);
    if(shmid < 0)
        perror("\n Error creating shared memory");
    shm = shmat(shmid,NULL,0);
    
    struct time *terminate = shm;
    time_increment(shm,dur);

    while(1)
    {
        if(time_check(shm, terminate->msec, terminate->nsec)){
            printf("[%6d %9d] Child %d terminating\n", shm->msec, shm->nsec, getpid());
        }
    }

return 0;
}