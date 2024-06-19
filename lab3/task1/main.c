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
extern int read_input();

int INFILE = STDIN;
int OUTFILE = STDOUT;


int main(int argc, char *argv[], char *envp[])
{
	int i;
	for (i = 0; i < argc; i++)
	{
		system_call(SYS_WRITE, STDERR, argv[i], strlen(argv[i]));
		system_call(SYS_WRITE, STDERR, "\n", 1);

		if (argv[i][0] == '-') {
			if (argv[i][1] == 'i') {
				INFILE = system_call(SYS_OPEN, argv[i]+2, O_RDWR);
			} else if (argv[i][1] == 'o') {
				OUTFILE = system_call(SYS_OPEN, argv[i]+2, O_RDWR);
			}
		}
	}
	read_input(INFILE, OUTFILE);


	return 0;
}