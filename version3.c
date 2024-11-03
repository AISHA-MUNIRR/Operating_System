#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_LINE 1024
#define MAX_ARGS 100

// Handler to reap child processes and avoid zombie processes
void handle_sigchld(int signo) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main() {
    char input[MAX_LINE];
    char *args[MAX_ARGS];
    struct sigaction sa;

    // Setup signal handling for SIGCHLD to handle background processes
    sa.sa_handler = handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);

    int job_count = 0;  // Counter for background jobs

    while (1) {
        // Display shell prompt
        printf("PUCITshell:- ");
        if (fgets(input, MAX_LINE, stdin) == NULL) { // Read input
            break; // Exit on Ctrl+D
        }

        // Remove trailing newline
        input[strcspn(input, "\n")] = 0;

        // Check for background execution (command ending with &)
        int is_background = 0;
        if (input[strlen(input) - 1] == '&') {
            is_background = 1;
            input[strlen(input) - 1] = 0; // Remove '&' from command
        }

        // Tokenize input into arguments
        int i = 0;
        char *token = strtok(input, " ");
        while (token != NULL) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL; // Null-terminate argument list

        // If there are arguments, proceed with forking
        if (i > 0) {
            pid_t pid = fork(); // Create child process
            if (pid < 0) {
                perror("Fork failed");
                exit(1);
            }

            if (pid == 0) { // Child process
                if (execvp(args[0], args) < 0) {
                    perror("Execution failed");
                    exit(1);
                }
            } else { // Parent process
                if (is_background) {
                    job_count++; // Increment background job count
                    printf("[%d] %d\n", job_count, pid); // Print job number and PID
                } else {
                    waitpid(pid, NULL, 0); // Wait for foreground job
                }
            }
        }
    }
    return 0;
}
