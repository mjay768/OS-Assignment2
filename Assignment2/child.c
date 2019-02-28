#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<sys/ipc.h>
#include<sys/shm.h>

#include "time.h"

#define KEY 0x7744842

int main(int* argc, char * argv[])
{
    uint dur = atoi(argv[1]);
    struct time *shm;
    /* Get shared memory*/
    key_t key;
    int shmid = shmget(KEY, sizeof(struct time), 0666 | IPC_CREAT);
    if(shmid < 0)
        perror("\n Error creating shared memory");
    shm = shmat(shmid,NULL,0);
    
    struct time *terminate = shm;
    time_increment(shm,dur);

    while(1)
    {
        if(time_check(shm, terminate->msec, terminate->nsec)){
            printf("Child %d terminating at [%4d %6d]\n",  getpid(),shm->msec, shm->nsec);break;
        }
    }

return 0;
}