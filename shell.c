#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

// max number of characters shell can read from input
int BUFFER_SIZE = 80;

// pid of the current foreground process
int fg_pid = -1;

// tracing background processes
const int MAX_BG = 15;
int bg_pids[MAX_BG];
int bg_count = 0;

// function to print the shell prompt
void print_prompt() {
  printf("mini-shell> ");
  fflush(stdout);
}

// signal handler for SIGINT (Ctrl+C) and SIGTSTP (Ctrl+Z)
void sig_handler(int sig) {
  if (sig == SIGINT) {
    printf("\nmini-shell terminated");
    if (fg_pid > 0) kill(fg_pid, SIGINT);
    for (int i = 0; i < bg_count; i++) {
      kill(bg_pids[i], SIGINT);
    }
    exit(0); 
  } else if (sig == SIGTSTP) {
    if (bg_count < MAX_BG && fg_pid > 0) bg_pids[bg_count++] = fg_pid;
    if (fg_pid > 0) {
      kill(fg_pid, SIGTSTP); 
      fg_pid = -1;
    } 
  }
}

int main(int argc, char** argv){
  alarm(120); // set a timer for 120 seconds to prevent accidental infinite loops or fork bombs
  signal(SIGINT, sig_handler);
  signal(SIGTSTP, sig_handler);
  
  // buffer to store user input
  char input[BUFFER_SIZE];

  while (1) {
    print_prompt();

    // read input from user
    if (fgets(input, BUFFER_SIZE, stdin) == NULL) {
      printf("\n"); 
      break;
    }

    input[strcspn(input, "\n")] = '\0';
    if (strcmp(input, "exit") == 0) break;
    if (strlen(input) == 0) continue;

    // check if command should run in background
    int run_in_bg = 0;
    int len = strlen(input);
    if (input[len-1] == '&') {
        run_in_bg = 1;
        input[len-1] = '\0';  
    }

    char *args[BUFFER_SIZE];
    int i = 0;
    char *token = strtok(input, " ");

    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    // handle built-in cd command
    if (strcmp(args[0], "cd") == 0) {
      char *target_dir = args[1];

      if (target_dir == NULL) {
        target_dir = getenv("HOME");
        if (target_dir == NULL) {
            fprintf(stderr, "couldn't find a home directory\n");
            continue;
        }
      }
      if (chdir(target_dir) != 0) fprintf(stderr, "cd failed\n");

      continue;
    }

    // create a new process to execute the command
    pid_t pid = fork();
    if (pid == 0) {
      execvp(args[0], args);
      fprintf(stderr, "exec failed\n");
      exit(1);
    } else if (pid > 0) {
      if (run_in_bg) {
        if (bg_count < MAX_BG) {
            bg_pids[bg_count++] = pid;
        }
      } else {
          fg_pid = pid;
          int status;
          waitpid(pid, &status, WUNTRACED);
          fg_pid = -1;
      } 
    } else {
      fprintf(stderr, "fork failed\n");
    }
  }
 
  return 0;
}