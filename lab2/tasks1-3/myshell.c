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
		else
		{
			cmdLine *currCmdLine = parseCmdLines(line);

			int pid = fork();
			if (pid == 0)
			{
				if (currCmdLine->inputRedirect != NULL)
				{
					int input_fd = open(currCmdLine->inputRedirect, O_RDONLY);
					if (input_fd == -1)
					{
						perror("Error opening input file");
						_exit(1);
					}
					if (dup2(input_fd, STDIN_FILENO) == -1)
					{
						perror("Error duplicating input file descriptor");
						close(input_fd);
						_exit(1);
					}
					close(input_fd);
				}
				if (currCmdLine->outputRedirect != NULL)
				{
					int output_fd = open(currCmdLine->outputRedirect, O_WRONLY | O_APPEND);
					if (output_fd == -1)
					{
						perror("Error opening output file");
						_exit(1);
					}
					if (dup2(output_fd, STDOUT_FILENO) == -1)
					{
						perror("Error duplicating output file descriptor");
						close(output_fd);
						_exit(1);
					}
					close(output_fd);
				}
				execute(currCmdLine, debug_mode);
			}
			else if (currCmdLine->blocking)
			{
				int status;
				waitpid(pid, &status, 0);
			}
			freeCmdLines(currCmdLine);
		}
	}
}