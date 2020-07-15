#pragma once

#define FAKE_TIME 1

#if (FAKE_TIME)
typedef struct timeval_x {
    int tv_sec;
} timeval_x;
extern int gettimeofday(timeval_x * tp);
#else
#include <windows.h>
typedef timeval timeval_x;
extern int gettimeofday(struct timeval_x * tp);
#endif
