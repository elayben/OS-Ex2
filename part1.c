// Elay Ben Yehoshua 214795668
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

void write_to_file(const char *message, int count, const char *filename) {
    FILE *file = fopen(filename, "a");
    if (file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < count; ++i) {
        fprintf(file, "%s\n", message);
    }
    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <parent_message> <child1_message> <child2_message> <count>\n", argv[0]);
        return 1;
    }

    const char *parent_message = argv[1];
    const char *child1_message = argv[2];
    const char *child2_message = argv[3];
    int count = atoi(argv[4]);

    pid_t pid1, pid2;

    // fork first child process
    if ((pid1 = fork()) < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid1 == 0) {
        // first child process
        write_to_file(child1_message, count, "output.txt");
        exit(EXIT_SUCCESS);
    }

    // fork second child process
    if ((pid2 = fork()) < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid2 == 0) {
        // In child2 process
        write_to_file(child2_message, count, "output.txt");
        exit(EXIT_SUCCESS);
    }

    // in parent process, wait for the two childrwn to  to finish
    int status;
    if (waitpid(pid1, &status, 0) < 0) {
        perror("waitpid");
        exit(EXIT_FAILURE);
    }

    if (waitpid(pid2, &status, 0) < 0) {
        perror("waitpid");
        exit(EXIT_FAILURE);
    }

    // parent can write to the file
    write_to_file(parent_message, count, "output.txt");

    return 0;
}