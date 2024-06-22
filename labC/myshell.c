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

#define HISTLEN 20	// Maximum number of history entries
#define MAX_BUF 200 // Maximum length of a command line

// History structure
typedef struct history_entry
{
	char command[MAX_BUF];
} history_entry;

// Circular queue for history
history_entry history[HISTLEN];
int newest = 0; // Index for the newest entry
int oldest = 0; // Index for the oldest entry

// Add a command to the history
void addToHistory(const char *command)
{
	strncpy(history[newest].command, command, MAX_BUF);
	newest = (newest + 1) % HISTLEN;
	if (newest == oldest)
		oldest = (oldest + 1) % HISTLEN;
}

// Print the history
void printHistory()
{
	int i = oldest;
	int count = 1;
	while (i != newest)
	{
		printf("%d: %s\n", count, history[i].command);
		i = (i + 1) % HISTLEN;
		count++;
	}
}

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
	process *new_process = malloc(sizeof(process));
	new_process->cmd = malloc(sizeof(cmdLine));
	new_process->cmd = cmd;
	new_process->pid = pid;
	new_process->status = RUNNING; // Assume it's running initially
	new_process->next = *process_list;
	*process_list = new_process;
}

void updateProcessList(process **process_list)
{
	process *curr = *process_list;
	while (curr && curr->cmd != 0)
	{
		int status;
		pid_t result;
		if (curr->cmd->blocking == 0)
		{
			result = waitpid(curr->pid, &status, WNOHANG);
		}
		else
		{
			result = waitpid(curr->pid, &status, 0);
		}
		if (result == curr->pid)
		{
			// Process has terminated
			if (WIFEXITED(status) || WIFSIGNALED(status))
			{
				curr->status = TERMINATED;
				// Remove the terminated process from the list
				// (you can implement this part)
			}
		}
		curr = curr->next;
	}
}

// Print the process list
void printProcessList(process **process_list)
{
	// Update process statuses
	updateProcessList(process_list);

	int is_head = 0;

	printf("PID\tCommand\t\tSTATUS\n");
	process *curr = *process_list;
	while (curr && curr->cmd != 0)
	{
		is_head = 0;
		printf("%d\t%s\t\t", curr->pid, curr->cmd->arguments[0]);
		if (curr->status == TERMINATED)
		{
			printf("Terminated\n");
			// Remove the terminated process from the list
			process *temp = curr;
			curr = curr->next;
			if (*process_list == temp)
			{
				*process_list = curr;
				is_head = 1;
			}
			freeCmdLines(temp->cmd);
			free(temp);
		}
		else if (curr->status == RUNNING)
			printf("Running\n");
		else if (curr->status == SUSPENDED)
			printf("Suspended\n");
		else
			printf("Unknown\n"); // Handle any other status (if needed)
		if (curr != NULL && !is_head)
		{
			curr = curr->next;
		}
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

void freeProcessList(process *process_list)
{
	while (process_list)
	{
		process *temp = process_list;
		process_list = process_list->next;
		freeCmdLines(temp->cmd);
	}
}

void updateProcessStatus(process *process_list, int pid, int status)
{
	process *curr = process_list;
	while (curr)
	{
		if (curr->pid == pid)
		{
			curr->status = status;
			break;
		}
		curr = curr->next;
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

	// Initialize process list
	process *process_list = malloc(sizeof(process));

	while (1)
	{
		char cwd_path[PATH_MAX];
		getcwd(cwd_path, PATH_MAX);
		// printf("%s\n", cwd_path);

		char line[2048];
		fgets(line, 2048, stdin);
		if (strcmp(line, "quit") == 0 || strcmp(line, "quit\n") == 0)
		{
			freeProcessList(process_list);
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
				updateProcessStatus(process_list, pid, RUNNING);
			}
		}
		else if (strncmp(line, "blast ", 6) == 0)
		{
			int pid;
			sscanf(line, "blast %d\n", &pid);
			if (kill(pid, SIGINT) == -1)
			{
				perror("Error sending SIGINT");
			}
			else
			{
				printf("Sent SIGINT to process %d\n", pid);
				updateProcessStatus(process_list, pid, TERMINATED);
			}
		}
		else if (strncmp(line, "sleep ", 6) == 0)
		{
			int pid;
			sscanf(line, "sleep %d\n", &pid);
			if (kill(pid, SIGTSTP) == -1)
			{
				perror("Error sending SIGTSTP");
			}
			else
			{
				printf("Sent SIGTSTP to process %d\n", pid);
				updateProcessStatus(process_list, pid, SUSPENDED);
			}
		}
		else if (strcmp(line, "procs\n") == 0)
		{
			// Print the process list
			printProcessList(&process_list);
		}
		else if (strcmp(line, "history\n") == 0)
		{
			printHistory();
		}
		else if (strcmp(line, "!!\n") == 0)
		{
			if (newest != oldest)
			{
				printf("Executing: %s\n", history[(newest - 1) % HISTLEN].command);
				// Execute the command (parse it again if needed)
				cmdLine *pipeline = parseCmdLines(history[(newest - 1) % HISTLEN].command);
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
					addToHistory(history[(newest - 1) % HISTLEN].command);
					updateProcessList(&process_list);
				}
			}
			else
			{
				printf("No previous command in history.\n");
			}
		}
		else if (line[0] == '!' && line[1] >= '1' && line[1] <= '9')
		{
			int n = line[1] - '0';
			if (n <= (newest - oldest) && n > 0)
			{
				printf("Executing: %s\n", history[(newest - n) % HISTLEN].command);
				// Execute the command (parse it again if needed)
				cmdLine *pipeline = parseCmdLines(history[(newest - n) % HISTLEN].command);
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
					addToHistory(history[(newest - n) % HISTLEN].command);
					updateProcessList(&process_list);
				}
			}
			else
			{
				printf("Invalid history index.\n");
			}
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
				addToHistory(line);
				updateProcessList(&process_list);
			}
		}
	}
}
