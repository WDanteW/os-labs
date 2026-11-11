#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

static char CHILD_PROGRAM_NAME[] = "child";

static void write_error(const char *msg, size_t len) {
    write(STDERR_FILENO, msg, len);
}

int main(void) {
    char progpath[1024];
    ssize_t len = readlink("/proc/self/exe", progpath, sizeof(progpath) - 1);
    if (len == -1) {
        const char err[] = "Error reading /proc/self/exe\n";
        write_error(err, sizeof(err) - 1);
        _exit(1);
    }
    
    progpath[len] = '\0';
    while (len > 0 && progpath[len] != '/') {
        --len;
    }
    progpath[len] = '\0';

    char filename[1024];
    int idx = 0;
    {
        const char prompt[] = "Enter filename: ";
        write(STDOUT_FILENO, prompt, sizeof(prompt) - 1);

        char c;
        while (read(STDIN_FILENO, &c, 1) > 0) {
            if (c == '\n' || c == '\r') {
                break;
            }
            if (idx < (int)sizeof(filename) - 1) {
                filename[idx++] = c;
            }
        }
        filename[idx] = '\0';
    }

    if (idx == 0) {
        const char err[] = "Filename is empty\n";
        write_error(err, sizeof(err) - 1);
        _exit(1);
    }

    int file_fd = open(filename, O_RDONLY);
    if (file_fd == -1) {
        const char err[] = "Cannot open specified file\n";
        write_error(err, sizeof(err) - 1);
        _exit(1);
    }

    int pipe1[2];
    if (pipe(pipe1) == -1) {
        const char err[] = "Pipe creation failed\n";
        write_error(err, sizeof(err) - 1);
        close(file_fd);
        _exit(1);
    }

    pid_t pid = fork();
    if (pid == -1) {
        const char err[] = "Fork failed\n";
        write_error(err, sizeof(err) - 1);
        close(file_fd);
        close(pipe1[0]);
        close(pipe1[1]);
        _exit(1);
    }

    if (pid == 0) {
        close(pipe1[0]);

        if (dup2(file_fd, STDIN_FILENO) == -1) {
            _exit(1);
        }
        close(file_fd);

        if (dup2(pipe1[1], STDOUT_FILENO) == -1) {
            _exit(1);
        }
        close(pipe1[1]);

        char path[1024];
        int p_len = 0;
        while (progpath[p_len] != '\0' && p_len < 512) {
            path[p_len] = progpath[p_len];
            p_len++;
        }
        path[p_len++] = '/';
        int c_len = 0;
        while (CHILD_PROGRAM_NAME[c_len] != '\0' && p_len < 1023) {
            path[p_len++] = CHILD_PROGRAM_NAME[c_len++];
        }
        path[p_len] = '\0';

        char *const args[] = {CHILD_PROGRAM_NAME, NULL};
        execv(path, args);
        
        const char err[] = "Execv failed\n";
        write_error(err, sizeof(err) - 1);
        _exit(1);
    } else {
        close(file_fd);
        close(pipe1[1]);

        char buf[1024];
        ssize_t bytes;

        while ((bytes = read(pipe1[0], buf, sizeof(buf))) > 0) {
            write(STDOUT_FILENO, buf, bytes);
        }
        close(pipe1[0]);

        waitpid(pid, NULL, 0);
    }

    return 0;
}
