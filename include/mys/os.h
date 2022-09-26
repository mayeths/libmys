#pragma once

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "macro.h"

#if defined(POSIX_COMPLIANCE)
#include <unistd.h>
#elif defined(OS_WINDOWS)
#include <windows.h>
#endif

static inline int busysleep(double sec)
{
#if defined(POSIX_COMPLIANCE)
    /* https://stackoverflow.com/a/8158862 */
    struct timespec req;
    req.tv_sec = (uint64_t)sec;
    req.tv_nsec = (sec - req.tv_sec) * 1000000000;
    do {
        if(nanosleep(&req, &req) == 0)
            break;
        else if(errno != EINTR)
            return -1;
    } while (req.tv_sec > 0 || req.tv_nsec > 0);
    return 0;
#elif defined(OS_WINDOWS)
    /* https://stackoverflow.com/a/5801863 */
    /* https://stackoverflow.com/a/26945754 */
    LARGE_INTEGER fr,t1,t2;
    if (!QueryPerformanceFrequency(&fr))
        return -1;
    if (!QueryPerformanceCounter(&t1))
        return -1;
    do {
        if (!QueryPerformanceCounter(&t2))
            return -1;
    } while(((double)t2.QuadPart-(double)t1.QuadPart)/(double)fr.QuadPart < sec);
    return 0;
#else
    return -2;
#endif
}

static inline char* execshell(const char* command) {
    FILE* fp;
    char* line = NULL;
    char* result = (char*) calloc(1, 1);
    size_t len = 0;

    fflush(NULL);
    fp = popen(command, "r");
    if (fp == NULL) {
        printf("Cannot execute command:\n%s\n", command);
        return NULL;
    }

    while(getline(&line, &len, fp) != -1) {
        size_t lenline = strlen(line);
        size_t lenres = strlen(result);
        result = (char*) realloc(result, lenline + lenres + 1);
        strncpy(result + lenres, line, lenline + 1);
        free(line);
        line = NULL;
    }

    fflush(fp);
    if (pclose(fp) != 0) {
        perror("Cannot close stream.\n");
    }
    return result;
}
