#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "common.h"

static void print_err(const char *msg) {
    write(STDERR_FILENO, msg, strlen(msg));
}

static void die(const char *msg) {
    print_err(msg);
    _exit(EXIT_FAILURE);
}

int main(void) {
    const char ask[] = "Enter file name: ";
    write(STDOUT_FILENO, ask, sizeof(ask) - 1);

    char fname[256];
    ssize_t n = read(STDIN_FILENO, fname, sizeof(fname) - 1);
    if (n <= 0) die("error: failed to read file name\n");
    fname[n] = '\0';
    char *nl = strchr(fname, '\n');
    if (nl) *nl = '\0';

    int file = open(fname, O_RDONLY);
    if (file == -1) die("error: failed to open file\n");

    char shm_name[64], sem_empty_name[64], sem_full_name[64];
    pid_t pid = getpid();
    snprintf(shm_name,       sizeof(shm_name),       "/lab3-shm-%d", pid);
    snprintf(sem_empty_name, sizeof(sem_empty_name), "/lab3-empty-%d", pid);
    snprintf(sem_full_name,  sizeof(sem_full_name),  "/lab3-full-%d", pid);

    int shm = shm_open(shm_name, O_RDWR | O_CREAT | O_EXCL, 0600);
    if (shm == -1) die("error: failed to create SHM\n");

    if (ftruncate(shm, sizeof(shared_t)) == -1) {
        shm_unlink(shm_name);
        die("error: failed to resize SHM\n");
    }

    shared_t *data = mmap(NULL, sizeof(shared_t), PROT_READ | PROT_WRITE,
                          MAP_SHARED, shm, 0);
    if (data == MAP_FAILED) {
        shm_unlink(shm_name);
        die("error: failed to map SHM\n");
    }
    data->length = 0;

    sem_t *sem_empty = sem_open(sem_empty_name, O_CREAT | O_EXCL, 0600, 1);
    sem_t *sem_full  = sem_open(sem_full_name,  O_CREAT | O_EXCL, 0600, 0);
    if (sem_empty == SEM_FAILED || sem_full == SEM_FAILED) {
        shm_unlink(shm_name);
        die("error: failed to create semaphores\n");
    }

    pid_t child = fork();
    if (child == -1) die("error: failed to fork\n");

    if (child == 0) {
        if (dup2(file, STDIN_FILENO) == -1) die("error: dup2 failed\n");
        close(file);
        execl("./child", "child", shm_name, sem_empty_name, sem_full_name, NULL);
        die("error: failed to exec\n");
    }
    close(file);
    bool child_reaped = false;
    while (true) {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 1;

        if (sem_timedwait(sem_full, &ts) == -1) {
            if (errno == EINTR) continue;
            if (errno == ETIMEDOUT) {
                if (waitpid(child, NULL, WNOHANG) == child) {
                    child_reaped = true;
                    print_err("error: child terminated unexpectedly\n");
                    break;
                }
                continue;
            }
            print_err("error: sem_timedwait failed\n");
            break;
        }

        if (data->length == MSG_END) {
            sem_post(sem_empty);
            break;
        }

        write(STDOUT_FILENO, data->text, data->length);
        sem_post(sem_empty);
    }

    if (!child_reaped) waitpid(child, NULL, 0);

    sem_close(sem_empty);
    sem_close(sem_full);
    sem_unlink(sem_empty_name);
    sem_unlink(sem_full_name);
    munmap(data, sizeof(shared_t));
    close(shm);
    shm_unlink(shm_name);
    return 0;
}