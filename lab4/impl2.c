#include <math.h>

#include "contracts.h"

float cos_derivative(float a, float dx) {
    return (cosf(a + dx) - cosf(a - dx)) / (2.0f * dx);
}

int gcd(int a, int b) {
    int m = a < b ? a : b;
    for (int d = m; d > 1; d--) {
        if (a % d == 0 && b % d == 0) {
            return d;
        }
    }
    return 1;
}