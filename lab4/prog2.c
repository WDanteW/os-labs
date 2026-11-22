#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef float (*cos_derivative_fn)(float, float);
typedef int (*gcd_fn)(int, int);

typedef struct {
    void *handle;
    cos_derivative_fn deriv;
    gcd_fn gcd;
} lib_t;

static int load_lib(const char *path, lib_t *lib) {
    lib->handle = dlopen(path, RTLD_LOCAL | RTLD_NOW);
    if (lib->handle == NULL) {
        fprintf(stderr, "Ошибка: не удалось загрузить библиотеку %s: %s\n",
                path, dlerror());
        return -1;
    }

    *(void **)(&lib->deriv) = dlsym(lib->handle, "cos_derivative");
    *(void **)(&lib->gcd) = dlsym(lib->handle, "gcd");
    if (lib->deriv == NULL || lib->gcd == NULL) {
        fprintf(stderr, "Ошибка: в библиотеке %s не найдены нужные функции\n", path);
        dlclose(lib->handle);
        return -1;
    }
    return 0;
}

int main(void) {
    char exe[1024];
    ssize_t len = readlink("/proc/self/exe", exe, sizeof(exe) - 1);
    if (len == -1) {
        fprintf(stderr, "Ошибка: не удалось определить путь к программе\n");
        return 1;
    }
    exe[len] = '\0';
    char *slash = strrchr(exe, '/');
    if (slash) *slash = '\0';

    const char *names[2] = {"libimpl1.so", "libimpl2.so"};
    lib_t libs[2];
    for (int i = 0; i < 2; i++) {
        char path[1100];
        snprintf(path, sizeof(path), "%s/%s", exe, names[i]);
        if (load_lib(path, &libs[i]) == -1) {
            return 1;
        }
    }

    int cur = 0;
    printf("Используется реализация 1\n");

    char line[256];
    while (fgets(line, sizeof(line), stdin)) {
        int cmd;
        if (sscanf(line, "%d", &cmd) != 1) {
            printf("Ошибка: некорректная команда\n");
            continue;
        }

        if (cmd == 0) {
            cur = 1 - cur;
            printf("Используется реализация %d\n", cur + 1);
        } else if (cmd == 1) {
            float a, dx;
            if (sscanf(line, "%d %f %f", &cmd, &a, &dx) != 3 || dx == 0.0f) {
                printf("Ошибка: формат команды: 1 a dx (dx не равно 0)\n");
                continue;
            }
            printf("Производная cos в точке %f: %f\n", a, libs[cur].deriv(a, dx));
        } else if (cmd == 2) {
            int a, b;
            if (sscanf(line, "%d %d %d", &cmd, &a, &b) != 3 || a <= 0 || b <= 0) {
                printf("Ошибка: формат команды: 2 a b (натуральные числа)\n");
                continue;
            }
            printf("НОД(%d, %d) = %d\n", a, b, libs[cur].gcd(a, b));
        } else {
            printf("Ошибка: неизвестная команда\n");
        }
    }

    dlclose(libs[0].handle);
    dlclose(libs[1].handle);
    return 0;
}