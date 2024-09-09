// Elay Ben Yehoshua 214795668
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <time.h>


// create lock
void create_lock(const char *lockfile) {
    while (open(lockfile, O_CREAT | O_EXCL, 0) == -1) {
        if (errno != EEXIST) {
            perror("Error creating lock file");
            exit(EXIT_FAILURE);
        }
        usleep(100000); 
    }
}

// write a message with random delays
void write_message(const char *message, int count) {
    for (int i = 0; i < count; i++) {
        printf("%s\n", message);
        usleep((rand() % 100) * 1000); 
    }
}

// release the lock
void release_lock(const char *lockfile) {
    if (unlink(lockfile) == -1) {
        perror("Error deleting lock file");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 5) {
        fprintf(stderr, "Usage: %s <message1> <message2> <message3> ... <count>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // get the number of times each messagw should be written
    int count = atoi(argv[argc - 1]);
    if (count <= 0) {
        fprintf(stderr, "Count must be a positive integer\n");
        return EXIT_FAILURE;
    }


    srand(time(NULL));

    // open the output file
    FILE *file = fopen("output2.txt", "w");
    if (file == NULL) {
        perror("Error opening output file");
        return EXIT_FAILURE;
    }

    // fork for each message
    for (int i = 1; i < argc - 1; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("Error forking process");
            return EXIT_FAILURE;
        } else if (pid == 0) {
            const char *lockfile = "lockfile.lock";

            for (int j = 0; j < count; j++) {
                create_lock(lockfile);
                if (freopen("output2.txt", "a", stdout) == NULL) {
                    perror("Error redirecting stdout to file");
                    exit(EXIT_FAILURE);
                }
                write_message(argv[i], 1);
                fflush(stdout); 
                fclose(stdout); 
                release_lock(lockfile);
            }

            exit(EXIT_SUCCESS);
        }
    }

    // parent wait
    while (wait(NULL) > 0);

    fclose(file);
    return EXIT_SUCCESS;
}