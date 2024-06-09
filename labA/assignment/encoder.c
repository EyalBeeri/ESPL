#include <stdio.h>
#include <string.h>

int isDigit(char c)
{
	return c >= '0' && c <= '9';
}

int isLower(char c)
{
	return c >= 'a' && c <= 'z';
}

int encoder(FILE *infile, FILE *outfile, char *enc_str)
{
	char enc_sign = enc_str[0];
	char *code = &enc_str[2];
	int code_len = 0;
	for (; code[code_len] != '\0'; code_len++);

	int i = 0;
	int curr_code_dig;
	while (feof(infile) == 0)
	{
		char out_char = fgetc(infile);
		if (isDigit(out_char) || isLower(out_char))
		{
			curr_code_dig = (code[i % code_len] - '0');
			if (enc_sign == '-')
			{
				curr_code_dig *= -1;
			}

			if (isLower(out_char))
			{
				out_char = (((((out_char + curr_code_dig) - 'a') % 26) + 26) % 26) + 'a';
			}
			else
			{
				out_char = (((((out_char + curr_code_dig) - '0') % 10) + 10) % 10) + '0';
			}
		}
		i++;
		if (out_char != EOF)
		{
			fputc(out_char, outfile);
		}
	}
	fclose(outfile);
	return 0;
}

int main(int argc, char **argv)
{
	int debug_mode = 1; // default, debug mode is on

	char *enc_str = "";

	FILE *infile = stdin;
	FILE *outfile = stdout;

	for (int i = 1; i < argc; i++)
	{
		if (argv[i][1] != '\0')
		{
			if (argv[i][1] == 'D')
			{
				if (argv[i][0] == '-')
				{
					debug_mode = 0;
				}
				else if (argv[i][0] == '+')
				{
					debug_mode = 1;
				}
			}
			else if (argv[i][1] == 'e' && (argv[i][0] == '+' || argv[i][0] == '-'))
			{
				enc_str = argv[i];
			}
			else if (argv[i][1] == 'I' && argv[i][0] == '-')
			{
				char *file_name = &argv[i][2];
				infile = fopen(file_name, "r");
				if (infile == NULL)
				{
					fprintf(stderr, "Failed to open %s\n", file_name);
					return 1;
				}
			}
			else if (argv[i][1] == 'O' && argv[i][0] == '-')
			{
				char *file_name = &argv[i][2];
				outfile = fopen(file_name, "w");
				if (infile == NULL)
				{
					fprintf(stderr, "Failed to open %s\n", file_name);
					return 1;
				}
			}
			if (debug_mode)
			{
				fprintf(stderr, "%s\n", argv[i]);
			}
		}
	}
	return encoder(infile, outfile, enc_str);
}
