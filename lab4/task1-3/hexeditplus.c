#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NAME_LEN 128
#define BUF_SZ 10000

typedef struct
{
	char debug_mode;
	char file_name[NAME_LEN];
	int unit_size;
	unsigned char mem_buf[BUF_SZ];
	size_t mem_count;
	char display_mode; // 0 = dec, 1 = hex
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

char *unit_to_hex(int unit_size)
{
	static char *formats[] = {"%#hhx\n", "%#hx\n", "No such unit", "%#x\n"};
	return formats[unit_size - 1];
}

char *unit_to_dec(int unit_size)
{
	static char *formats[] = {"%#hhd\n", "%#hd\n", "No such unit", "%#d\n"};
	return formats[unit_size - 1];
}

/* Prints the buffer to screen by converting it to text with printf */
void print_units(FILE *output, unsigned char* buffer, int length, int unit_size, char display_mode)
{
	unsigned char *beginBuf = buffer;
	unsigned char *end = buffer + unit_size * length;

	if (display_mode)
	{
		printf("Hex\n================\n");
		while (buffer < end)
		{
			int var = *((int *)(buffer));
			fprintf(output, unit_to_hex(unit_size), var);
			buffer += unit_size;
		}
		end = beginBuf + unit_size * length;
	}
	else
	{
		printf("Dec\n================\n");
		while (beginBuf < end)
		{
			// print ints
			int var = *((int *)(beginBuf));
			fprintf(output, unit_to_dec(unit_size), var);
			beginBuf += unit_size;
		}
	}
}

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
	printf("\n");
	state->file_name[strlen(state->file_name) - 1] = '\0';
	if (state->debug_mode)
		fprintf(stderr, "Debug: file name set to %s\n", state->file_name);
}

void setUnitSize(state *state)
{
	int intOption;
	char option[10];
	printf("Enter unit size: ");
	fgets(option, 10, stdin);
	printf("\n");
	int parse = sscanf(option, "%d", &intOption);
	if (parse != 0 && (intOption == 1 || intOption == 2 || intOption == 4))
	{
		state->unit_size = intOption;
		if (state->debug_mode)
			fprintf(stderr, "Debug: set size to %s\n", option);
	}
	else
		printf("Error: not a valid size: %s\n", option);
}

void loadIntoMemory(state *state)
{
	if (state->file_name[0] == '\0')
	{
		fprintf(stderr, "There is no file name\n");
		return;
	}

	free(state->mem_buf);

	FILE *file = fopen(state->file_name, "rb");
	if (file == NULL)
	{
		fprintf(stderr, "Failed to open file: %s\n", state->file_name);
		return;
	}

	printf("Please enter <location> <length>\n");
	char line[NAME_LEN];
	fgets(line, NAME_LEN, stdin);
	int location, length;
	sscanf(line, "%x %d", &location, &length);

	if (state->debug_mode)
	{
		fprintf(stderr, "File name: %s\nLocation: %x\nLength: %d\n", state->file_name, location, length);
	}
	if (fseek(file, location, SEEK_SET) == -1)
	{
		fprintf(stderr, "Failed to get current location in the file\n");
		fclose(file);
		return;
	}
	fread(state->mem_buf, state->unit_size, length, file);
	fclose(file);
	state->mem_count = length;
	printf("Loaded %d units into memory\n\n", state->mem_count);
	return;
}

void toggleDisplayMode(state *state)
{
	if (state->display_mode)
	{
		state->display_mode = 0;
		printf("Display flag now off, decimal representation\n");
	}
	else
	{
		state->display_mode = 1;
		printf("Display flag now on, hexadecimal representation\n");
	}
}

void memoryDisplay(state *state)
{
	printf("Enter address and length\n");
	char line[NAME_LEN];
	fgets(line, NAME_LEN, stdin);
	int address, length;
	sscanf(line, "%x %d", &address, &length);
	print_units(stdout, state->mem_buf + address, length, state->unit_size, state->display_mode);
	printf("\n");
}

void saveIntoFile(state *state)
{
	char line[NAME_LEN];
    printf("Please enter <source-address> <target-location> <length>\n");
    fgets(line, NAME_LEN, stdin);
    int target, sourceAddr, length;
    if (sscanf(line, "%x %x %d\n", &sourceAddr, &target, &length) == 0)
    {
        fprintf(stderr, "Invalid input\n");
        return;
    }
    FILE *fp = fopen(state->file_name, "r+");
    if (fp == NULL)
    {
        fprintf(stderr, "Failed to open file: %s\n", state->file_name);
        return;
    }
    fseek(fp, 0L, SEEK_END);
    int res = ftell(fp);
    if (res < target)
        fprintf(stderr, "File is shorter than target\n");
    else
    {
        fseek(fp, target, SEEK_SET);
        fwrite(state->mem_buf + sourceAddr, state->unit_size, length, fp);
    }
    fclose(fp);
}

void memoryModify(state *state)
{
	char line[NAME_LEN];
    printf("Please enter <location> <val>\n");
    fgets(line, NAME_LEN, stdin);
    int location, val;
    if (sscanf(line, "%x %x\n", &location, &val) == 0)
    {
        fprintf(stderr, "Invalid input\n");
        return;
    }
    FILE *fp = fopen(state->file_name, "r+");
    if (fp == NULL)
    {
        fprintf(stderr, "Failed to open file: %s\n", state->file_name);
        return;
    }
    fseek(fp, 0L, SEEK_END);
    int res = ftell(fp);
    if (res < location)
        fprintf(stderr, "File is shorter than target\n");
    else
    {
        fseek(fp, location, SEEK_SET);
        fwrite(&val, state->unit_size, 1, fp);
    }
    fclose(fp);
}

void quit(state *state)
{
	if (state->debug_mode)
	{
		fprintf(stderr, "Quitting\n");
	}
	free(state->mem_buf);
	free(state->file_name);
	exit(0);
}

int main(int argc, char **argv)
{
	state *state = malloc(sizeof(state));
	state->debug_mode = 0;
	state->display_mode = 0;
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