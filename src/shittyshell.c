#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    char *line = NULL;
    char **argz;
    pid_t pid, wpid;
    size_t bufsize = 0;

    while(1) {
        // 1. Print a prompt
        printf("$ ");
        fflush(stdout);

        // 2. Read a line
        if (getline(&line, &bufsize, stdin) == -1) {
            perror("getline");
            continue;
        }

        // 3. Trim newline
        if (line[strlen(line) - 1] == '\n')
            line[strlen(line) - 1] = '\0';

        // 4. Fork
        pid = fork();
        if (pid == 0) {
            // Child

            // 5. Build arg list
            argz = malloc(2 * sizeof(char*));
            argz[0] = line;
            argz[1] = NULL;

            // 6. Exec
            if (execv(line, argz) == -1) {
                perror("nsh");
            }
            exit(1);
        }
        else if (pid > 0) {
            // Parent
            wait(NULL);
        }
        else {
            perror("fork");
        }

        // 7. Free memory each loop
        free(line);
        line = NULL;
        bufsize = 0;
    }
}
