#ifndef SCHEDULE_UTILS_H
#define SCHEDULE_UTILS_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

//Errors
#define FILE_NOT_FOUND NULL

typedef struct schedule{
    int i;
    int j;
    int shape;
    int *mask;
}Schedule;

Schedule *parseSchedule(char *scheduleFile);

int getFileLenght(char *scheduleFile);

int64_t *compress_schedule_toROM(Schedule *s, int schedule_len,
        int lanes,int schedule_rom_size);

#endif
