# Unix Shell Implementation in C

## Overview

This project is a simple Unix shell implemented in C. It provides a command-line interface that allows users to execute commands and interact with the operating system.

The project demonstrates fundamental Unix system programming concepts, including process creation, command execution, input/output redirection, and inter-process communication using pipes.

## Features

- **Command Execution:** Executes external programs using `fork()` and `execvp()`.
- **Built-in Commands:** Supports built-in commands such as `cd`, `mkdir`, `help`, and `exit`.
- **Input Redirection:** Supports redirecting standard input using `<`.
- **Output Redirection:** Supports redirecting standard output using `>`.
- **Pipelines:** Supports connecting commands using pipes (`|`).
- **Command History:** Includes a `!!` command to repeat the previous command.

## Project Structure

```text
unix-shell/
├── main.c
├── builtins.c
├── executor.c
├── parser.c
├── shell.h
├── Makefile
└── README.md
```

## Requirements

- GCC compiler
- Unix-based operating system (Linux or macOS)
- Terminal

## Compilation

Compile the project using GCC:

```bash
gcc main.c builtins.c executor.c parser.c -o mysh
```

## Running the Shell

After compiling, run the shell with:

```bash
./mysh
```

You should see the shell prompt:

```text
mysh>
```

You can now enter commands.

## Example Commands

```bash
mysh> help
mysh> ls
mysh> pwd
mysh> mkdir test
mysh> cd test
mysh> ls
mysh> exit
```

### Input and Output Redirection

```bash
mysh> cat < input.txt
mysh> ls > output.txt
```

### Pipes

```bash
mysh> ls | grep .c
```

### Command History

```bash
mysh> !!
```

Repeats the previous command.

## Concepts Demonstrated

This project explores several important operating system concepts:

- Process creation using `fork()`
- Program execution using `execvp()`
- Process synchronization using `wait()`
- File descriptor manipulation using `dup2()`
- File operations using `open()` and `close()`
- Inter-process communication using `pipe()`
- Built-in shell command implementation

## Future Improvements

- Add support for multiple pipes.
- Improve error handling.
- Add support for environment variables.
- Implement more built-in commands.
- Improve command parsing and argument handling.

## Author

Robin Dunne

## License

This project is available for educational and personal use.
