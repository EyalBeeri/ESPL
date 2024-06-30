#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct
{
	char debug_mode;
	char file_name[128];
	int unit_size;
	unsigned char mem_buf[10000];
	size_t mem_count;
	/*
	 .
	 .
	 Any additional fields you deem necessary
	*/
} state;

struct fun_desc
{
	char *name;
	void (*fun)(state *);
} fun_desc;

void toggleDebugMode(state *state)
{
	if (state->debug_mode)
	{
		state->debug_mode = 0;
		printf("Debug flag now off\n");
	}
	else
	{
		state->debug_mode = 1;
		printf("Debug flag now on\n");
	}
}

void setFileName(state *state)
{
	printf("Enter file name: ");
	fgets(state->file_name, 100, stdin);
	state->file_name[strlen(state->file_name) - 1] = '\0';
	if (state->debug_mode)
		fprintf(stderr, "Debug: file name set to %s", state->file_name);
}

void setUnitSize(state *state)
{
	int intOption;
	char option[10];
	printf("Enter unit size: ");
	fgets(option, 10, stdin);
	int parse = sscanf(option, "%d", &intOption);
	if (parse != 0 && (intOption == 1 || intOption == 2 || intOption == 4))
	{
		state->unit_size = intOption;
		if (state->debug_mode)
			fprintf(stderr, "Debug: set size to %s", option);
	}
	else
		fprintf(stderr, "Error: not a valid size: %s", option);
}

void loadIntoMemory(state *state)
{
	printf("Not implemented yet");
}

void toggleDisplayMode(state *state)
{
	printf("Not implemented yet");
}

void memoryDisplay(state *state)
{
	printf("Not implemented yet");
}

void saveIntoFile(state *state)
{
	printf("Not implemented yet");
}

void memoryModify(state *state)
{
	printf("Not implemented yet");
}

void quit(state *state)
{
	if (state->debug_mode)
	{
		fprintf(stderr, "Quitting\n");
	}
	exit(0);
}

int main(int argc, char **argv)
{
	state *state = malloc(sizeof(state));
	state->debug_mode = 0;
	struct fun_desc menu[] = {
		{"Toggle Debug Mode", toggleDebugMode},
		{"Set File Name", setFileName},
		{"Set Unit Size", setUnitSize},
		{"Load Into Memory", loadIntoMemory},
		{"Toggle Display Mode", toggleDisplayMode},
		{"Memory Display", memoryDisplay},
		{"Save Into File", saveIntoFile},
		{"Memory Modify", memoryModify},
		{"Quit", quit},
		{NULL, NULL}};
	int choice;
	while (feof(stdin) == 0)
	{
		if (state->debug_mode)
		{
			fprintf(stderr, "Debug log:\n");
			fprintf(stderr, "Unit size: %d\n", state->unit_size);
			fprintf(stderr, "File name: %s\n", state->file_name);
			fprintf(stderr, "Memory count: %zu\n", state->mem_count);
			fprintf(stderr, "\n");
		}
		printf("Select operation from the following menu (ctrl^D for exit):\n");
		for (int i = 0; menu[i].name != NULL; i++)
		{
			printf("%d) %s\n", i, menu[i].name);
		}
		printf("Option: ");
		if (scanf("%d", &choice) == EOF)
		{
			return 0;
		}
		while ((getchar()) != '\n')
			;
		if (choice < 0 || choice >= sizeof(menu) / sizeof(struct fun_desc) - 1)
		{
			printf("Not within bounds\n");
			return 0;
		}
		printf("\n");
		menu[choice].fun(state);
	}
}