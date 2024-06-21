#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "LineParser.h"
#include <linux/limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

// Process status constants
#define TERMINATED -1
#define RUNNING 1
#define SUSPENDED 0

// Process structure
typedef struct process
{
	cmdLine *cmd;
	pid_t pid;
	int status;
	struct process *next;
} process;

// Add a process to the process list
void addProcess(process **process_list, cmdLine *cmd, pid_t pid)
{
	process *new_process = (process *)malloc(sizeof(process));
	new_process->cmd = malloc(sizeof(cmdLine));
	new_process->cmd = cmd;
	new_process->pid = pid;
	new_process->status = RUNNING; // Assume it's running initially
	new_process->next = *process_list;
	*process_list = new_process;
}

// Print the process list
void printProcessList(process **process_list)
{
	printf("PID\tCommand\t\tSTATUS\n");
	process *curr = *process_list;
	while (curr)
	{
		printf("%d\t%s\t\t", curr->pid, curr->cmd->arguments[0]);
		if (curr->status == TERMINATED)
			printf("Terminated\n");
		else if (curr->status == RUNNING)
			printf("Running\n");
		else if (curr->status == SUSPENDED)
			printf("Suspended\n");
		curr = curr->next;
	}
}

void execute(cmdLine *pCmdLine, int debug_mode)
{
	char *programname = pCmdLine->arguments[0];
	char *const *argv = pCmdLine->arguments;

	if (debug_mode)
	{
		fprintf(stderr, "PID: %d\nExecuting command: %s", getpid(), programname);
		for (int i = 1; argv[i] != NULL; i++)
		{
			fprintf(stderr, " %s", argv[i]);
		}
		fprintf(stderr, "\n");
	}
	execvp(programname, argv);
	perror("Program execution has failed");
	_exit(1);
}

void run_pipeline(cmdLine *pipeline, int debug_mode)
{
	int num_cmds = 0;
	cmdLine *currCmdLine = pipeline;
	while (currCmdLine)
	{
		num_cmds++;
		currCmdLine = currCmdLine->next;
	}

	int pipe_fds[2];
	pid_t child_pid;
	int status;

	for (int i = 0; i < num_cmds; i++)
	{
		if (i < num_cmds - 1)
		{
			// Create a pipe for communication between processes
			if (pipe(pipe_fds) == -1)
			{
				perror("Error creating pipe");
				exit(1);
			}
		}

		child_pid = fork();
		if (child_pid == 0)
		{
			// Child process
			if (i > 0)
			{
				// Redirect input from the previous pipe
				dup2(pipe_fds[0], STDIN_FILENO);
				close(pipe_fds[0]);
			}
			if (i < num_cmds - 1)
			{
				// Redirect output to the next pipe
				dup2(pipe_fds[1], STDOUT_FILENO);
				close(pipe_fds[1]);
			}

			execute(pipeline, debug_mode);
		}
		else
		{
			// Parent process
			if (i > 0)
				close(pipe_fds[0]);
			if (i < num_cmds - 1)
				close(pipe_fds[1]);

			if (pipeline->blocking)
				waitpid(child_pid, &status, 0);

			pipeline = pipeline->next;
		}
	}
}

int main(int argc, char **argv)
{
	int debug_mode = 0;
	for (int i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-' && argv[i][1] == 'd')
		{
			debug_mode = 1;
		}
	}

	// Initialize process list (start with NULL)
	process *process_list = NULL;

	while (1)
	{
		char cwd_path[PATH_MAX];
		getcwd(cwd_path, PATH_MAX);
		printf("%s\n", cwd_path);

		char line[2048];
		fgets(line, 2048, stdin);
		if (strcmp(line, "quit") == 0 || strcmp(line, "quit\n") == 0)
		{
			exit(0);
		}
		else if (strncmp(line, "cd ", 3) == 0)
		{
			char path[PATH_MAX];
			sscanf(line, "cd %s\n", path);
			if (chdir(path) == -1)
			{
				perror(path);
			}
		}
		else if (strncmp(line, "alarm ", 6) == 0)
		{
			int pid;
			sscanf(line, "alarm %d\n", &pid);
			if (kill(pid, SIGCONT) == -1)
			{
				perror("Error sending SIGCONT");
			}
			else
			{
				printf("Sent SIGCONT to process %d\n", pid);
			}
		}
		else if (strncmp(line, "blast ", 6) == 0)
		{
			int pid;
			sscanf(line, "blast %d\n", &pid);
			if (kill(pid, SIGTERM) == -1)
			{
				perror("Error sending SIGTERM");
			}
			else
			{
				printf("Sent SIGTERM to process %d\n", pid);
			}
		}
		else if (strcmp(line, "procs\n") == 0)
		{
			// Print the process list
			printProcessList(&process_list);
		}
		else
		{
			cmdLine *pipeline = parseCmdLines(line);
			pid_t child_pid = fork();
			if (child_pid == 0)
			{
				// Child process
				execute(pipeline, debug_mode);
				_exit(0); // Ensure child process exits
			}
			else
			{
				// Parent process
				addProcess(&process_list, pipeline, child_pid);
				if (pipeline->blocking)
				{
					int status;
					waitpid(child_pid, &status, 0);
					process_list->status = TERMINATED;
				}
			}
			freeCmdLines(pipeline);
		}
	}
}
