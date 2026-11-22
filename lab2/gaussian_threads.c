#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <errno.h>

typedef struct {
    int thread_id;
    int num_threads;
    int n;
    double **a;
    pthread_barrier_t *barrier;
} thread_data_t;

static void* worker(void *arg) {
    thread_data_t *data = (thread_data_t*) arg;
    int n            = data->n;
    int id           = data->thread_id;
    int num_threads  = data->num_threads;
    double **a       = data->a;

    for (int k = 0; k < n - 1; k++) {
        int total_rows = n - (k + 1);

        if (total_rows > 0) {
            int base = total_rows / num_threads;
            int rem  = total_rows % num_threads;

            int start = (k + 1) + id * base + (id < rem ? id : rem);
            int end   = start + base + (id < rem ? 1 : 0);

            for (int i = start; i < end; i++) {
                double factor = a[i][k] / a[k][k];
                for (int j = k; j <= n; j++) {
                    a[i][j] -= factor * a[k][j];
                }
            }
        }

        pthread_barrier_wait(data->barrier);
    }
    return NULL;
}

static double** allocate_matrix(int n) {
    double **a = malloc((size_t)n * sizeof(double*));
    if (!a) { perror("malloc"); exit(EXIT_FAILURE); }
    for (int i = 0; i < n; i++) {
        a[i] = malloc((size_t)(n + 1) * sizeof(double));
        if (!a[i]) { perror("malloc"); exit(EXIT_FAILURE); }
    }
    return a;
}

static void free_matrix(double **a, int n) {
    for (int i = 0; i < n; i++) free(a[i]);
    free(a);
}

static void copy_matrix(double **dst, double **src, int n) {
    for (int i = 0; i < n; i++)
        memcpy(dst[i], src[i], (size_t)(n + 1) * sizeof(double));
}

static void generate_matrix(double **a, int n) {
    unsigned short xsubi[3] = {42, 42, 42};
    for (int i = 0; i < n; i++)
        for (int j = 0; j <= n; j++)
            a[i][j] = erand48(xsubi) * 10.0 + 1.0;
}

static void back_substitution(double **a, int n, double *x) {
    for (int i = n - 1; i >= 0; i--) {
        x[i] = a[i][n];
        for (int j = i + 1; j < n; j++)
            x[i] -= a[i][j] * x[j];
        x[i] /= a[i][i];
    }
}

static void sequential_gaussian(double **a, int n, double *x) {
    for (int k = 0; k < n - 1; k++) {
        for (int i = k + 1; i < n; i++) {
            double factor = a[i][k] / a[k][k];
            for (int j = k; j <= n; j++)
                a[i][j] -= factor * a[k][j];
        }
    }
    back_substitution(a, n, x);
}

static void parallel_gaussian(double **a, int n, int num_threads, double *x) {
    if (num_threads < 1) num_threads = 1;

    pthread_t         *threads = malloc((size_t)num_threads * sizeof(pthread_t));
    thread_data_t     *td      = malloc((size_t)num_threads * sizeof(thread_data_t));
    pthread_barrier_t barrier;

    if (!threads || !td) { perror("malloc"); exit(EXIT_FAILURE); }

    if (pthread_barrier_init(&barrier, NULL, (unsigned)num_threads) != 0) {
        perror("pthread_barrier_init");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_threads; i++) {
        td[i].thread_id   = i;
        td[i].num_threads = num_threads;
        td[i].n           = n;
        td[i].a           = a;
        td[i].barrier     = &barrier;

        if (pthread_create(&threads[i], NULL, worker, &td[i]) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_threads; i++)
        pthread_join(threads[i], NULL);

    pthread_barrier_destroy(&barrier);
    free(threads);
    free(td);

    back_substitution(a, n, x);
}

static double now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1e6;
}

static double residual(double **a_orig, int n, const double *x) {
    double max_r = 0.0;
    for (int i = 0; i < n; i++) {
        double s = 0.0;
        for (int j = 0; j < n; j++)
            s += a_orig[i][j] * x[j];
        double r = s - a_orig[i][n];
        if (r < 0) r = -r;
        if (r > max_r) max_r = r;
    }
    return max_r;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <N> [threads1 threads2 ...]\n", argv[0]);
        fprintf(stderr, "Пример: %s 512 1 2 4 8 16 128 1024\n", argv[0]);
        return 1;
    }

    char *end = NULL;
    errno = 0;
    long n_l = strtol(argv[1], &end, 10);
    if (errno || *end != '\0' || n_l <= 1) {
        fprintf(stderr, "Некорректный N\n");
        return 1;
    }
    int n = (int)n_l;

    int  n_threads_list = argc - 2;
    int *threads_list   = NULL;

    if (n_threads_list > 0) {
        threads_list = malloc((size_t)n_threads_list * sizeof(int));
        for (int i = 0; i < n_threads_list; i++) {
            errno = 0;
            long t = strtol(argv[2 + i], &end, 10);
            if (errno || *end != '\0' || t <= 0 || t > 100000) {
                fprintf(stderr, "Некорректное число потоков: %s\n", argv[2 + i]);
                free(threads_list);
                return 1;
            }
            threads_list[i] = (int)t;
        }
    } else {
        n_threads_list  = 1;
        threads_list    = malloc(sizeof(int));
        threads_list[0] = 4;
    }

    double **a_orig = allocate_matrix(n);
    double **a_seq  = allocate_matrix(n);
    double **a_par  = allocate_matrix(n);
    double  *x_seq  = malloc((size_t)n * sizeof(double));
    double  *x_par  = malloc((size_t)n * sizeof(double));

    generate_matrix(a_orig, n);
    copy_matrix(a_seq, a_orig, n);

    double t0 = now_ms();
    sequential_gaussian(a_seq, n, x_seq);
    double t_seq = now_ms() - t0;

    printf("N = %d\n", n);
    printf("Seq time: %.3f ms   (residual = %.3e)\n\n", t_seq, residual(a_orig, n, x_seq));

    printf("%10s | %14s | %10s | %12s | %12s\n",
           "Threads", "Time (ms)", "Speedup", "Efficiency", "Max residual");
    printf("-----------+----------------+------------+--------------+-------------\n");

    for (int t = 0; t < n_threads_list; t++) {
        int p = threads_list[t];

        copy_matrix(a_par, a_orig, n);

        double t1 = now_ms();
        parallel_gaussian(a_par, n, p, x_par);
        double t_par = now_ms() - t1;

        double speedup    = t_seq / t_par;
        double efficiency = speedup / p;
        double res        = residual(a_orig, n, x_par);

        printf("%10d | %14.3f | %10.3f | %12.3f | %12.3e\n",
               p, t_par, speedup, efficiency, res);
    }

    free_matrix(a_orig, n);
    free_matrix(a_seq,  n);
    free_matrix(a_par,  n);
    free(x_seq);
    free(x_par);
    free(threads_list);

    return 0;
}