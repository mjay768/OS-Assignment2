#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/shm.h>
#include<sys/ipc.h>
#include<sys/wait.h>

#include "time.h"

#define KEY 0x7744842

typedef unsigned long ulong;
typedef unsigned int uint;

int flag = 1;

int max_processes =4;
int parallel_proc_limit =2;

/* Process states */

int total_processes;
int proc_parallel;


/* File pointers */
char* inputfile = "input.txt";
char* outputfile = "output.txt";
FILE* foptr;
FILE* fcptr;

pid_t *pids;
struct time *shm;

/*
long int* ms; // Pointer for milliseconds
long int* ns; // Pointer for nanoseconds */

uint millis,nanos,duration;

struct time *shm;

long int increment = 20000;

void checkArgs(int argc, char* argv[]);
void createSharedMem();
int timer(int* count);
void readNextLine(FILE* ptr);
void createChild();
void terminate();
int main(int argc, char* argv[])
{

    int i;
    int* count;
    pids = calloc(max_processes, sizeof(pid_t));
    checkArgs(argc,argv); // Parsing command-line arguments

    /* File pointers initialization */

    foptr = fopen(inputfile, "r");
    if(foptr == NULL)
     perror("\n File Open Error\n");
    fcptr = fopen(outputfile, "a+");

    fscanf(foptr, "%ld", &increment); // Read default incrementing value
    printf("\ninc : %ld", increment);
    createSharedMem();           

    printf("\nfrom main : %s", inputfile);
    printf("\nfrom main : %s",outputfile);
    int tempcount =0;
    
    readNextLine(foptr);

    while(1)
    {
        time_increment(shm, nanos); // To increment the virtual timer

        /* Check to see if it is time to create a child */
        if(time_check(shm, millis, nanos) && total_processes < max_processes && proc_parallel < parallel_proc_limit)
        {
            createChild();
            readNextLine(foptr);
        }

        pid_t pid = waitpid(-1, NULL, WNOHANG);
		if (pid > 0)
        {
            fprintf(fcptr, "[%6d %9d] Terminated: %d\n", shm->msec, shm->nsec, pid);
	        for (int i = 0; i < total_processes; i++) {
		    if (pids[i] == pid) {
			pids[i] = 0;
		}
	}
	proc_parallel--;
		}
        if (!flag && !proc_parallel) {
			terminate();
			return;
		}
		if (!proc_parallel && total_processes == max_processes) {
			terminate();
			return;
		}
           

    }

    //printf("\n ms = %ld ns = %ld", *ms,*ns);
   
    return 0;
}

void checkArgs( int argc, char* argv[])
{
    char opt;
    while((opt= getopt(argc,argv, "hi:o:n:s:")) != -1)
    {
        switch(opt)
        {
            case 'h' : printf("\nUse -h for help");
                       printf("\n Syntax: -i [input file name] -o [output file name] -n [number of processes] -s [Max processes to be running]\n");break;
            case 'n' : parallel_proc_limit = atoi(optarg);break;
            case 's' : max_processes = atoi(optarg);break;
            case 'i' : inputfile = optarg; break;
            case 'o' : outputfile = optarg; break;
            case '?' : printf("\n Invalid arguments\n");
                       printf("\nPlease use -h for help");
                       printf("\n Syntax: -i [input file name] -o [output file name] -n [number of processes] -s [Max processes to be running]\n");break;
        }
    }
}


void createSharedMem()
{
    key_t key;
    int shmid = shmget(KEY, sizeof(struct time), 0666 | IPC_CREAT);
    if(shmid < 0)
        perror("\n Error creating shared memory");
    shm = shmat(shmid,NULL,0);
    shm->msec = 0;
    shm->nsec = 0;

    printf("\n %d %d \n", shm->msec,shm->nsec);
}

void readNextLine(FILE* ptr)
{
    if (!flag)
		return;
	if (EOF == fscanf(foptr, "%d %d %d", &millis, &nanos, &duration))
		flag = 0;

    printf("\nmillis : %d nanos : %d dura : %d", millis,nanos,duration);

    printf("\n%s", inputfile);
    printf("\n%s", outputfile);
}

void createChild()
{
    int i;
    char durstr[40];
    sprintf(durstr,"%d", duration);
    printf("\n%s",durstr);
    pid_t pid;
    pid = fork();
    if(pid < 0)
        perror("\nFork error");
    else if(pid == 0)
    {
        execlp("./child", "./child", durstr,NULL);
        printf("\nFrom child - Parent: %d Child : %d\n", getppid(),getpid());
    }

    else 
    {
        pids[total_processes] = pid;
        printf("\nFrom parent - Child : %d",pids[total_processes]);
    }
    total_processes++;
    proc_parallel++;

    printf("\n total procs = %d\nproc_parallel = %d", total_processes,proc_parallel);

 
}

void terminate() {
    fprintf(fcptr, "[%6d %9d] Terminating\n", shm->msec, shm->nsec);
	/* Terminate children */
	for (int i = 0; i < total_processes; i++) {
		if (pids[i] != 0) {
			kill(pids[i], 15);
		}
	}

	fclose(foptr);
	fclose(fcptr);
	exit(0);
}
