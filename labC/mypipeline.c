#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char* argv[]) {
    int pipefd[2];
    pid_t child1, child2;

    // Create a pipe
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Fork the first child (child1)
    fprintf(stderr, "(parent_process>forking…)\n");
    child1 = fork();
    if (child1 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (child1 == 0) { // Child1 process
        fprintf(stderr, "(child1>redirecting stdout to the write end of the pipe…)\n");
        close(1); // Close stdout
        dup(pipefd[1]); // Duplicate the write-end of the pipe using
        close(pipefd[1]); // Close duplicated fd

        // Execute "ls -l"
        fprintf(stderr, "(child1>going to execute cmd: ls -l)\n");
        char* args[] = {"ls", "-l", NULL};
        execvp("ls", args);
        perror("execvp ls");
        exit(EXIT_FAILURE);
    } else {
        fprintf(stderr, "(parent_process>created process with id: ) %d\n", child1);
    }

    // Parent process
    fprintf(stderr, "(parent_process>closing the write end of the pipe…)\n");
    close(pipefd[1]); // Close write end

    // Fork the second child (child2)
    fprintf(stderr, "(parent_process>forking…)\n");
    child2 = fork();
    if (child2 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (child2 == 0) { // Child2 process
        fprintf(stderr, "(child2>redirecting stdin to the read end of the pipe…)\n");
        close(0); // Close stdin
        dup(pipefd[0]); // Duplicate the read-end of the pipe
        close(pipefd[0]); // Close duplicated fd

        // Debug message
        fprintf(stderr, "(child2>redirecting stdin to the read end of the pipe...)\n");

        // Execute "tail -n 2"
        fprintf(stderr, "(child2>going to execute cmd: tail -n 2)\n");
        char* args[] = {"tail", "-n", "2", NULL};
        execvp("tail", args);
        perror("execvp tail");
        exit(EXIT_FAILURE);
    } else {
        fprintf(stderr, "(parent_process>created process with id: ) %d\n", child2);
    }

    // Parent process
    fprintf(stderr, "(parent_process>closing the read end of the pipe…)\n");
    close(pipefd[0]); // Close read end

    // Wait for child processes to terminate
    fprintf(stderr, "(parent_process>waiting for child processes to terminate…)\n");
    waitpid(child1, NULL, 0);
    waitpid(child2, NULL, 0);

    // Debug messages
    fprintf(stderr, "(parent_process>exiting...)\n");

    return 0;
}
