#include <stdio.h>

#include "contracts.h"

int main(void) {
    char line[256];

    while (fgets(line, sizeof(line), stdin)) {
        int cmd;
        if (sscanf(line, "%d", &cmd) != 1) {
            printf("Ошибка: некорректная команда\n");
            continue;
        }

        if (cmd == 0) {
            printf("Ошибка: в этой программе переключение реализаций не поддерживается\n");
        } else if (cmd == 1) {
            float a, dx;
            if (sscanf(line, "%d %f %f", &cmd, &a, &dx) != 3 || dx == 0.0f) {
                printf("Ошибка: формат команды: 1 a dx (dx не равно 0)\n");
                continue;
            }
            printf("Производная cos в точке %f: %f\n", a, cos_derivative(a, dx));
        } else if (cmd == 2) {
            int a, b;
            if (sscanf(line, "%d %d %d", &cmd, &a, &b) != 3 || a <= 0 || b <= 0) {
                printf("Ошибка: формат команды: 2 a b (натуральные числа)\n");
                continue;
            }
            printf("НОД(%d, %d) = %d\n", a, b, gcd(a, b));
        } else {
            printf("Ошибка: неизвестная команда\n");
        }
    }
    return 0;
}