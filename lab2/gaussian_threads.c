#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

typedef struct {
    int thread_id;
    int num_threads;
    int n;
    double **a;
    int k;
    pthread_barrier_t *barrier;
} thread_data_t;

static void* eliminate_rows(void *arg) {
    thread_data_t *data = (thread_data_t*) arg;
    int n = data->n;
    int k = data->k;
    double **a = data->a;
    int id = data->thread_id;
    int num_threads = data->num_threads;

    int total_rows = n - (k + 1);
    if (total_rows > 0) {
        int rows_per_thread = total_rows / num_threads;
        int remainder = total_rows % num_threads;

        int start = (k + 1) + id * rows_per_thread + (id < remainder ? id : remainder);
        int end = start + rows_per_thread + (id < remainder ? 1 : 0);

        for (int i = start; i < end; i++) {
            double factor = a[i][k] / a[k][k];
            for (int j = k; j <= n; j++) {
                a[i][j] -= factor * a[k][j];
            }
        }
    }

    pthread_barrier_wait(data->barrier);
    return NULL;
}

static double** allocate_matrix(int n) {
    double **a = malloc(n * sizeof(double*));
    for (int i = 0; i < n; i++) {
        a[i] = malloc((n + 1) * sizeof(double));
    }
    return a;
}

static void free_matrix(double **a, int n) {
    for (int i = 0; i < n; i++) free(a[i]);
    free(a);
}

static void generate_matrix(double **a, int n) {
    unsigned short xsubi[3] = {42, 42, 42};
    for (int i = 0; i < n; i++) {
        for (int j = 0; j <= n; j++) {
            a[i][j] = erand48(xsubi) * 10.0 + 1.0;
        }
    }
}

static void sequential_gaussian(double **a, int n, double *x) {
    for (int k = 0; k < n - 1; k++) {
        for (int i = k + 1; i < n; i++) {
            double factor = a[i][k] / a[k][k];
            for (int j = k; j <= n; j++) {
                a[i][j] -= factor * a[k][j];
            }
        }
    }

    for (int i = n - 1; i >= 0; i--) {
        x[i] = a[i][n];
        for (int j = i + 1; j < n; j++) {
            x[i] -= a[i][j] * x[j];
        }
        x[i] /= a[i][i];
    }
}

static void parallel_gaussian(double **a, int n, int num_threads, double *x) {
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    thread_data_t *thread_data = malloc(num_threads * sizeof(thread_data_t));
    pthread_barrier_t barrier;

    pthread_barrier_init(&barrier, NULL, num_threads);

    for (int k = 0; k < n - 1; k++) {
        for (int i = 0; i < num_threads; i++) {
            thread_data[i].thread_id = i;
            thread_data[i].num_threads = num_threads;
            thread_data[i].n = n;
            thread_data[i].a = a;
            thread_data[i].k = k;
            thread_data[i].barrier = &barrier;

            pthread_create(&threads[i], NULL, eliminate_rows, &thread_data[i]);
        }

        for (int i = 0; i < num_threads; i++) {
            pthread_join(threads[i], NULL);
        }
    }

    pthread_barrier_destroy(&barrier);
    free(threads);
    free(thread_data);

    for (int i = n - 1; i >= 0; i--) {
        x[i] = a[i][n];
        for (int j = i + 1; j < n; j++) {
            x[i] -= a[i][j] * x[j];
        }
        x[i] /= a[i][i];
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Использование: %s <размер матрицы N> <количество потоков>\n", argv[0]);
        return 1;
    }

    char *endptr1;
    char *endptr2;
    int n = (int)strtol(argv[1], &endptr1, 10);
    int num_threads = (int)strtol(argv[2], &endptr2, 10);

    if (*endptr1 != '\0' || *endptr2 != '\0' || n <= 0 || num_threads <= 0) {
        printf("Некорректные аргументы командной строки\n");
        return 1;
    }

    double **a_seq = allocate_matrix(n);
    double **a_par = allocate_matrix(n);
    double *x_seq = malloc(n * sizeof(double));
    double *x_par = malloc(n * sizeof(double));

    generate_matrix(a_seq, n);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j <= n; j++) {
            a_par[i][j] = a_seq[i][j];
        }
    }

    struct timespec start;
    struct timespec end;

    clock_gettime(CLOCK_MONOTONIC, &start);
    sequential_gaussian(a_seq, n, x_seq);
    clock_gettime(CLOCK_MONOTONIC, &end);
    double seq_time = (double)(end.tv_sec - start.tv_sec) * 1000.0 + (double)(end.tv_nsec - start.tv_nsec) / 1000000.0;

    clock_gettime(CLOCK_MONOTONIC, &start);
    parallel_gaussian(a_par, n, num_threads, x_par);
    clock_gettime(CLOCK_MONOTONIC, &end);
    double par_time = (double)(end.tv_sec - start.tv_sec) * 1000.0 + (double)(end.tv_nsec - start.tv_nsec) / 1000000.0;

    printf("N = %d, Threads = %d\n", n, num_threads);
    printf("Seq Time: %.2f ms\n", seq_time);
    printf("Par Time: %.2f ms\n", par_time);
    printf("Speedup:  %.2f\n", seq_time / par_time);
    printf("Efficiency: %.2f\n", (seq_time / par_time) / num_threads);

    free_matrix(a_seq, n);
    free_matrix(a_par, n);
    free(x_seq);
    free(x_par);

    return 0;
}