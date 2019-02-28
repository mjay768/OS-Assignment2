#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>

#include "time.h"

void time_increment(struct time *time, uint nsec)
{
    time->nsec += nsec;
    if(time->nsec > 1000000)
    {
        time->msec += time->nsec / 1000000;
        time->nsec += time->nsec % 1000000;
    }
}

int time_check(struct time *t, uint msec, uint nsec)
{
    if(t->msec > msec)
     return 1;
    if(t->msec == msec)
     return  t->nsec > nsec;
    return 0;
}