#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "common.h"

static void die(const char *msg) {
    write(STDERR_FILENO, msg, strlen(msg));
    _exit(EXIT_FAILURE);
}

static bool is_prime(unsigned long long n) {
    if (n < 2) return false;
    for (unsigned long long i = 2; i * i <= n; i++)
        if (n % i == 0) return false;
    return true;
}

static void send_msg(shared_t *data, sem_t *empty, sem_t *full,
                     const char *text, uint32_t len) {
    while (sem_wait(empty) == -1) {
        if (errno != EINTR) die("error: sem_wait failed\n");
    }
    if (text) {
        memcpy(data->text, text, len);
        data->length = len;
    } else {
        data->length = MSG_END;
    }
    sem_post(full);
}

int main(int argc, char *argv[]) {
    if (argc != 4) die("usage: child <shm> <sem_empty> <sem_full>\n");

    int shm = shm_open(argv[1], O_RDWR, 0);
    if (shm == -1) die("error: failed to open SHM\n");

    shared_t *data = mmap(NULL, sizeof(shared_t), PROT_READ | PROT_WRITE,
                          MAP_SHARED, shm, 0);
    if (data == MAP_FAILED) die("error: failed to map SHM\n");

    sem_t *empty = sem_open(argv[2], 0);
    sem_t *full  = sem_open(argv[3], 0);
    if (empty == SEM_FAILED || full == SEM_FAILED)
        die("error: failed to open semaphores\n");

    char line[128];
    while (fgets(line, sizeof(line), stdin)) {
        char *end;
        errno = 0;
        long long num = strtoll(line, &end, 10);
        if (end == line) continue;
        if (errno == ERANGE) die("error: number out of range\n");

        if (num < 0 || is_prime((unsigned long long)num)) {
            break;
        }
        if (num < 2) continue;

        char out[32];
        int len = snprintf(out, sizeof(out), "%lld\n", num);
        send_msg(data, empty, full, out, (uint32_t)len);
    }

    send_msg(data, empty, full, NULL, 0);

    sem_close(empty);
    sem_close(full);
    munmap(data, sizeof(shared_t));
    close(shm);
    return 0;
}