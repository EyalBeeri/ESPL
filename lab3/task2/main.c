#include "util.h"

#define SYS_WRITE 4
#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define SYS_OPEN 5
#define O_RDWR 2
#define SYS_SEEK 19
#define SEEK_SET 0
#define SHIRA_OFFSET 0x291

extern int system_call();
extern void infection();
extern void infector(char *);

int main(int argc, char *argv[], char *envp[])
{
    int i;
    for (i = 0; i < argc; i++)
    {
        if (argv[i][0] == '-') {
            if (argv[i][1] == 'a') {
                char* filename = argv[i] + 2;
                system_call(SYS_WRITE, STDOUT, filename, strlen(filename));
                system_call(SYS_WRITE, STDOUT, "\n", 1);
                infection();
                infector(filename);
            }
        }
    }

    return 0;
}
