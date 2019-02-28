#ifndef TIME_H
#define TIME_H


struct time{
    uint msec;
    uint nsec;
};

void time_increment(struct time *time, uint nsec);
int time_check(struct time *time, uint msec, uint nsec);

#endif