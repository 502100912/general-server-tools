#pragma once
#include <stdio.h>
#include <ctime>

#define CFATAL_LOG(fmt, arg...) \
do { \
    time_t t = time(0); \
    char tmp[64];\
    strftime(tmp, sizeof(tmp), "[%m-%d %X]", localtime(&t));\
    printf("%s [FATAL] " "[%s:%d] " fmt "\n", tmp, __FILE__ , __LINE__, ##arg); \
} while (0)

#define CWARNING_LOG(fmt, arg...) \
do { \
    time_t t = time(0); \
    char tmp[64];\
    strftime(tmp, sizeof(tmp), "[%m-%d %X]", localtime(&t));\
    printf("%s [WARNING] " "[%s:%d] " fmt "\n", tmp, __FILE__ , __LINE__, ##arg); \
} while (0)

#define CDEBUG_LOG(fmt, arg...) \
do { \
    time_t t = time(0); \
    char tmp[64];\
    strftime(tmp, sizeof(tmp), "[%m-%d %X]", localtime(&t));\
    printf("%s [DEBUG] " "[%s:%d] " fmt "\n", tmp, __FILE__ , __LINE__, ##arg); \
} while (0)


