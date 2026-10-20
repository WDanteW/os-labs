#include <unistd.h>

static int is_prime(long long n) {
    if (n <= 1) return 0;
    if (n <= 3) return 1;
    if (n % 2 == 0 || n % 3 == 0) return 0;
    for (long long i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return 0;
    }
    return 1;
}

static void process_line(char *line, int line_len) {
    if (line_len <= 0) return;
    line[line_len] = '\0';

    long long num = 0;
    int negative = 0;
    int start = 0;
    if (line[0] == '-') {
        negative = 1;
        start = 1;
    }

    int valid = 1;
    for (int j = start; j < line_len; ++j) {
        if (line[j] >= '0' && line[j] <= '9') {
            num = num * 10 + (line[j] - '0');
        } else {
            valid = 0;
            break;
        }
    }

    if (valid) {
        if (negative) {
            num = -num;
        }

        if (num < 0 || is_prime(num)) {
            _exit(0);
        } else {
            char out_buf[64];
            long long temp = num;
            char rev[64];
            int r_len = 0;
            int len = 0;

            if (temp == 0) {
                rev[r_len++] = '0';
            } else {
                while (temp > 0) {
                    rev[r_len++] = (char)('0' + (temp % 10));
                    temp /= 10;
                }
            }

            while (r_len > 0) {
                out_buf[len++] = rev[--r_len];
            }
            out_buf[len++] = '\n';
            write(STDOUT_FILENO, out_buf, len);
        }
    }
}

int main(void) {
    char buf[1024];
    char line[1024];
    int line_len = 0;
    ssize_t bytes;

    while ((bytes = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
        for (ssize_t i = 0; i < bytes; ++i) {
            char c = buf[i];
            if (c == '\n' || c == '\r') {
                process_line(line, line_len);
                line_len = 0;
            } else {
                if (line_len < (int)sizeof(line) - 1) {
                    line[line_len++] = c;
                }
            }
        }
    }

    if (line_len > 0) {
        process_line(line, line_len);
    }

    return 0;
}