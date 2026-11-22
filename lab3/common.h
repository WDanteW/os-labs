#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

#define SHM_SIZE 4096
#define MSG_END  UINT32_MAX

typedef struct {
    uint32_t length;
    char     text[SHM_SIZE - sizeof(uint32_t)];
} shared_t;

#endif