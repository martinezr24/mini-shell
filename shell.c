#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

// max number of characters shell can read from input
int BUFFER_SIZE = 80;

// function to print the shell prompt
void print_prompt() {
  printf("mini-shell> ");
  fflush(stdout);
}

int main(int argc, char** argv){
  alarm(120); // set a timer for 120 seconds to prevent accidental infinite loops or fork bombs

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
        wait(NULL);
    } else {
      fprintf(stderr, "fork failed\n");
    }
  }
 
  return 0;
}