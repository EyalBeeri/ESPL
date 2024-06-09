#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char *map(char *array, int array_length, char (*f)(char))
{
    char *mapped_array = (char *)(malloc(array_length * sizeof(char)));
    /* TODO: Complete during task 2.a */
    for (int i = 0; i < array_length; i++)
    {
        mapped_array[i] = f(array[i]);
    }

    return mapped_array;
}

char my_get(char c)
{
    static int isFirst = 1;
    char input = fgetc(stdin);
    if (isFirst && input == '\n')
    {
        isFirst = 0;
        input = fgetc(stdin);
    }
    return input;
}

/* Ignores c, reads and returns a character from stdin using fgetc. */

char cprt(char c)
{
    if (c >= 0x20 && c <= 0x7E)
    {
        printf("%c\n", c);
    }
    else
    {
        printf(".\n");
    }
    return c;
}

/* If c is a number between 0x20 and 0x7E, cprt prints the character of ASCII value c followed by a new line. Otherwise, cprt prints the dot ('.') character. After printing, cprt returns the value of c unchanged. */

char encrypt(char c)
{
    if (c >= 0x20 && c <= 0x4E)
    {
        return c + 0x20;
    }
    else
    {
        return c;
    }
}
/* Gets a char c. If c is between 0x20 and 0x4E add 0x20 to its value and return it. Otherwise return c unchanged */

char decrypt(char c)
{
    if (c >= 0x40 && c <= 0x7E)
    {
        return c - 0x20;
    }
    else
    {
        return c;
    }
}
/* Gets a char c and returns its decrypted form subtractng 0x20 from its value. But if c was not between 0x40 and 0x7E it is returned unchanged */

char xoprt(char c)
{
    if (c >= 0x20 && c <= 0x7E)
    {
        printf("%x ", c);
        printf("%o\n", c);
    }
    else
    {
        printf(".\n");
    }

    return c;
}
/* xoprt prints the value of c in a hexadecimal representation, then in octal representation, followed by a new line, and returns c unchanged. */

struct fun_desc
{
    char *name;
    char (*fun)(char);
};

int main(int argc, char **argv)
{
    char *carray = calloc(5, sizeof(char));
    struct fun_desc menu[] = {
        {"Get String", my_get},
        {"Print String", cprt},
        {"Encrypt", encrypt},
        {"Decrypt", decrypt},
        {"Print Hex and Octal", xoprt},
        {NULL, NULL}};
    int choice;
    // struct fun_desc main_fun_desc[5] = {{"Get String", &my_get}, {"Print String", &cprt}, {"Print Hex", &xoprt}, {"Encrypt", &encrypt}, {"Decrypt", &decrypt}, {NULL, NULL}};
    while (feof(stdin) == 0)
    {
        printf("Select operation from the following menu (ctrl^D for exit):\n");
        for (int i = 0; menu[i].name != NULL; i++)
        {
            printf("%d) %s\n", i, menu[i].name);
        }
        printf("Option : ");
        if (scanf("%d", &choice) == EOF)
        {
            return 0;
        }
        if (choice < 0 || choice >= sizeof(menu) / sizeof(struct fun_desc) - 1)
        {
            printf("Not within bounds\n");
            return 0;
        }
        printf("\n");
        printf("Within bounds\n");
        carray = map(carray, 5, menu[choice].fun);
        printf("DONE.\n\n");
    }
    free(carray);
    return 0;
}