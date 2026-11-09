#include "shell.h"
#include <stdio.h>
#include <string.h>

char last_command[MAX_LINE] = "";

int main() {
    char input[MAX_LINE];
    char *args[MAX_ARGS];
    char *mode = NULL;

    while (1) {
        printf("mysh> ");
        if (fgets(input, MAX_LINE, stdin) == NULL) break;

        input[strcspn(input, "\n")] = 0;
        if (strlen(input) == 0) continue;

        strcpy(last_command, input);

        parse_input(input, args, &mode);

        if (strcmp(args[0], "!!") == 0) {
            if (strlen(last_command) == 0) {
                printf("No command history.\n");
                continue;
            }
            parse_input(last_command, args, &mode);
        }

        if (mode) {
            if (strcmp(mode, "ECHO") == 0) {
                for (int i = 0; args[i] != NULL; i++) {
                    if (strcmp(args[i], "<") == 0 || strcmp(args[i], ">") == 0 || strcmp(args[i], "|") == 0) continue;
                    printf("%s ", args[i]);
                }
                printf("\n");
                continue;
            } else if (strcmp(mode, "IO") == 0) {
                for (int i = 0; args[i] != NULL; i++) {
                    if (strcmp(args[i], "<") == 0) printf("LT %s\n", args[i+1]);
                    if (strcmp(args[i], ">") == 0) printf("GT %s\n", args[i+1]);
                }
                continue;
            } else if (strcmp(mode, "PIPE") == 0) {
                for (int i = 0; args[i] != NULL; i++) {
                    if (strcmp(args[i], "|") == 0 && args[i+1] != NULL) {
                        printf("PIPE %s\n", args[i+1]);
                    }
                }
                continue;
            }
        }

        if (strcmp(args[0], "exit") == 0) break;
        if (strcmp(args[0], "cd") == 0 || strcmp(args[0], "mkdir") == 0 || strcmp(args[0], "help") == 0) {
            handle_builtin(args);
            continue;
        }

        int has_pipe = 0;
        for (int i = 0; args[i] != NULL; i++) {
            if (strcmp(args[i], "|") == 0) {
                has_pipe = 1;
                char *cmd1[MAX_ARGS], *cmd2[MAX_ARGS];
                split_pipe_command(args, cmd1, cmd2);
                run_pipe_command(cmd1, cmd2);
                break;
            }
        }

        if (!has_pipe) {
            run_command(args);
        }
    }
    return 0;
}
