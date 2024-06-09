#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "LineParser.h"
#include <linux/limits.h>

void execute(cmdLine *pCmdLine) {
    char *pathname = pCmdLine->arguments[0];
    char* const* argv = pCmdLine->arguments;
    execvp(pathname, argv);
    perror("Program execution has failed");
    exit(1);
}

int main(int argc, char **argv)
{
    while (1)
    {
        char cwd_path[PATH_MAX];
        getcwd(cwd_path, PATH_MAX);
        printf("%s\n", cwd_path);

        char line[2048];
        fgets(line, 2048, stdin);
        if (strcmp(line, "quit") == 0) {
            exit(0);
        }

        cmdLine* currCmdLine = parseCmdLines(line);
        execute(currCmdLine);
        freeCmdLines(currCmdLine);

    }
}