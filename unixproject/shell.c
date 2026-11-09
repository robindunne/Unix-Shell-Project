#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define ARGS_SIZE   64
const char* LOCAL_CMDS[] = {"cd", "mkdir", "exit", "help"};
typedef enum REDIRECT { INPUT, OUTPUT, PIPE } REDIRECT;

// prints args and file descriptors
void strings_print(char* args[], int fin, int fout) { 
    char** q = args;
    int i = 0;
    while (*q != NULL) { 
        printf("args[%d] is %s\n", i++, *q++); 
    }
    printf("fin is %d, fout is %d\n", fin, fout); 
    printf("============================\n");
}

// gets command from user
int prompt_get_command(char* buf) { 
    memset(buf, 0, BUFSIZ);
    printf("> ");
    char* p = fgets(buf, BUFSIZ, stdin);
    unsigned long len = strlen(p);
    buf[len - 1] = '\0';
    return len;
}

// checks if cmd is local (cd, mkdir, etc.)
bool is_local_command(const char* cmd, const char* local_cmds[], int size) { 
    for (int i = 0; i < size; ++i) { 
        if (strcmp(cmd, local_cmds[i]) == 0) { 
            printf("%s is a local command\n", cmd);
            printf("============================\n");
            return true;
        }
    }
    return false;
}

// splits command line into argv
int parse(char* line, char** argv) { 
    int argc = 0;
    while (*line != '\0') { 
        while (isspace(*line)) {
            *line++ = '\0';
        }
        *argv++ = line;
        ++argc;
        while (*line != '\0' && !isspace(*line)) {
            line++;
        }
    }
    *argv = NULL;
    return argc;
}

// handles ECHO special case
bool parse_echo(int argc, char** argv) { 
    if (argc == 0) { return false; }
    char *last = argv[argc - 1];
    if (strncmp(last, "ECHO", strlen("ECHO")) != 0) {
        return false;
    }
    char** q = argv;
    bool found = false;
    while (*q != NULL) { 
        if (strncmp(*q, "<", 1) == 0) {
            q++;
            fprintf(stdout, "LT %s\n", *q++);
            found = true;
        } else if (strncmp(*q, ">", 1) == 0) {
            q++;
            fprintf(stdout, "GT %s\n", *q++);
            found = true;
        } else if (strncmp(*q, "|", 1) == 0) { 
            q++;
            char** r = argv;
            while (*r != NULL) { 
                if (strncmp(*r, "|", 1) == 0) { 
                    printf("PIPE ");
                } else if (strncmp(*r, "ECHO", 4) != 0) { 
                    printf("%s ", *r); 
                }
                r++;
            }
            printf("\n");
            found = true;
        } else { ++q; }
    }
    return found;
}

// parses redirection (<, >) and creates pipe if needed
void parse_redirect(char** argv, int* fin, int* fout, int* pipefd) { 
    char** q = argv;
    while (*q != NULL) { 
        if (strncmp(*q, "<", 1) == 0) {
            *q++ = NULL;
            *fin = open(*q, O_RDONLY, 0644);
            *q++ = NULL;      
        } else if (strncmp(*q, ">", 1) == 0) {
            *q++ = NULL;
            *fout = open(*q, O_CREAT | O_TRUNC | O_RDWR, 0644);
            *q++ = NULL;
        } else if (strncmp(*q, "|", 1) == 0) {
            *q++ = NULL;
            pipe(pipefd);  
        } else { ++q; }
    }
}

// handles built-in commands (cd, mkdir, etc.)
int execute_local_command(char** args) { 
    int rc = 0;
    const char* cmd = args[0];
    if (strncmp(cmd, "exit", 4) == 0) { 
        fprintf(stdout, "exiting shell...\n");
        exit(0);
    } else if (strncmp(cmd, "help", 4) == 0) { 
        fprintf(stdout, "Available commands: cd, mkdir, exit, help\n");
    } else if (strncmp(cmd, "cd", 2) == 0) { 
        rc = chdir(args[1]);
        if (rc == 0) { printf("changing directory to %s\n", args[1]); }
        else { fprintf(stderr, "failed to change directory to '%s'\n", args[1]); }
    } else if (strncmp(cmd, "mkdir", 5) == 0) { 
        rc = mkdir(args[1], 0755); 
        if (rc == 0) { printf("making new directory %s\n", args[1]); }
        else { fprintf(stderr, "failed to make directory '%s'\n", args[1]); }
    }
    return rc;
}

// runs a single command
int execute(char** args, int fin, int fout, int* pipefd) { 
    int ex_rc = 0;
    int stdin_orig = dup(STDIN_FILENO);
    int stdout_orig = dup(STDOUT_FILENO);

    if (fin != -1) { dup2(fin, STDIN_FILENO); }
    if (fout != -1) { dup2(fout, STDOUT_FILENO); }

    int rc = fork();
    if (rc < 0) { perror("Fork failed in execute()\n"); }
    else if (rc == 0) { 
        if (pipefd[0] != -1) {
            close(pipefd[0]); 
            dup2(pipefd[1], STDOUT_FILENO);  
            close(pipefd[1]);
        }
        ex_rc = execvp(args[0], args);
        if (ex_rc != 0) { 
            perror("execvp failure in execute()\n");
        }
    } else { 
        if (pipefd[1] != -1) {
            close(pipefd[1]); 
            dup2(pipefd[0], STDIN_FILENO); 
            close(pipefd[0]);
        }
        wait(&ex_rc);
        if (ex_rc != 0) { 
            fprintf(stderr, "execvp returned %d\n", ex_rc);
        }
    }

    dup2(stdin_orig, STDIN_FILENO);
    dup2(stdout_orig, STDOUT_FILENO);
    close(stdin_orig);
    close(stdout_orig);
    return ex_rc;
}

// handles two commands connected by pipe
void execute_pipe(char** args1, char** args2, int fin, int fout) {
    int pipefd[2];
    pipe(pipefd);

    int rc1 = fork();
    if (rc1 < 0) {
        perror("Fork failed in execute_pipe()\n");
    } else if (rc1 == 0) {
        close(pipefd[0]);  
        dup2(pipefd[1], STDOUT_FILENO);  
        close(pipefd[1]);
        execute(args1, fin, -1, NULL);
        exit(0);
    }

    int rc2 = fork();
    if (rc2 < 0) {
        perror("Fork failed in execute_pipe()\n");
    } else if (rc2 == 0) {
        close(pipefd[1]);  
        dup2(pipefd[0], STDIN_FILENO);  
        close(pipefd[0]);
        execute(args2, -1, fout, NULL);
        exit(0);
    }

    close(pipefd[0]);
    close(pipefd[1]);
    wait(NULL);
    wait(NULL);
}

// main shell loop
int run_shell(int argc, const char** argv) { 
    char cmd[BUFSIZ]; 
    char* args[ARGS_SIZE];
    int fin, fout, pipefd[2];

    while (true) { 
        fin = fout = pipefd[0] = pipefd[1] = -1;
        prompt_get_command(cmd);
        int argc = parse(cmd, args);     
        if (parse_echo(argc, args)) { continue; }

        parse_redirect(args, &fin, &fout, pipefd);
        strings_print(args, fin, fout);

        if (is_local_command(args[0], LOCAL_CMDS, sizeof(LOCAL_CMDS)/sizeof(char*))) { 
            execute_local_command(args);
            continue;
        }         

        char** args1 = args;
        char** args2 = NULL;
        for (int i = 0; args[i]; i++) if (!strcmp(args[i], "|")) args[i] = NULL, args2 = &args[i+1];

        args2 ? execute_pipe(args1, args2, fin, fout) : execute(args, fin, fout, pipefd);
    }
}
