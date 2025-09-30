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

  char input[BUFFER_SIZE];

  // infinite loop to read input and print back what was typed
  while (1) {
    print_prompt();

    if (fgets(input, BUFFER_SIZE, stdin) == NULL) {
      printf("\n");
      break;
    }

    input[strcspn(input, "\n")] = '\0';

    if (strcmp(input, "exit") == 0) break;
  
    if (strlen(input) == 0) continue;

    printf("You typed: %s\n", input);
  }
 
  return 0;
}