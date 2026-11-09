#include "shell.h"
#include <string.h>

void parse_input(char *input, char *args[], char **mode) {
    int i = 0;
    char *token = strtok(input, " ");
    while (token) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    *mode = NULL;
    if (i > 0 && (strcmp(args[i-1], "ECHO") == 0 || strcmp(args[i-1], "IO") == 0 || strcmp(args[i-1], "PIPE") == 0)) {
        *mode = args[i-1];
        args[i-1] = NULL;
    }
}
