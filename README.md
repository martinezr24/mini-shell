# Mini Shell

## Overview
This project is a simple command-line shell implemented in C. It supports command execution, piping, logical operators, job control, and built-in commands. The goal of the project was to gain hands-on experience with process management, signal handling, and command parsing in C.

## Features
- **Built-in Commands**: `cd`, `exit`, `help`, `fg`, `bg`, `jobs`, `history`
- **Job Control**: Run jobs in the background using `&`, and manage them with `jobs`, `fg`, and `bg`
- **Command Execution**: Executes both built-in and external commands
- **Piping**: Supports multiple piped commands (e.g. `ls | grep .c | wc -l`)
- **Logical Operators**: Handles `;`, `&&`, and `||` for sequencing and conditional execution
- **Signal Handling**: Gracefully responds to `Ctrl+C` (`SIGINT`) and `Ctrl+Z` (`SIGTSTP`)
- **Command History**: Stores and displays previously executed commands

## How to Compile and Run
To compile the program:

```bash
gcc mini_shell.c -o mini-shell
```

To run the shell:

```bash
./mini-shell
```

## Usage
- Type commands and press Enter to execute them.
- Use `&` at the end of a command to run it in the background.
- Use `fg` and `bg` to move jobs to the foreground or background.
- Use `history` to display previously entered commands.
- Press `Ctrl+C` to exit the shell.

### Example Commands

```bash
ls -l | grep .c
cd ..
sleep 10 &
jobs
fg 1
```

## Limitations
- Maximum of **5 background jobs** stored in the job table
- Command buffer size limited to **80 characters**
- Command history limited to **100 entries**