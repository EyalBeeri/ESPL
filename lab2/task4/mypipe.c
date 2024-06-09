#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

int main() {
    int pipefds[2];
    pid_t pid;
    const char *message = "hello";
    char readbuffer[10]; // Adjust buffer size if necessary

    // Initialize the buffer to zero
    memset(readbuffer, 0, sizeof(readbuffer));

    // Create a pipe
    if (pipe(pipefds) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Create a child process
    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Child process
        // Close the read end of the pipe
        close(pipefds[0]);

        // Write the message to the pipe
        write(pipefds[1], message, strlen(message) + 1); // +1 for null terminator

        // Close the write end of the pipe
        close(pipefds[1]);

        exit(EXIT_SUCCESS);
    } else {
        // Parent process
        // Close the write end of the pipe
        close(pipefds[1]);

        // Read the message from the pipe
        read(pipefds[0], readbuffer, sizeof(readbuffer) - 1); // -1 to leave space for null terminator

        // Print the message
        printf("Received message: %s\n", readbuffer);

        // Close the read end of the pipe
        close(pipefds[0]);

        // Wait for the child process to terminate
        wait(NULL);
    }

    return 0;
}
