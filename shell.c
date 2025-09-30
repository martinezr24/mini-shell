#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

int BUFFER_SIZE = 80;

void print_prompt() {
  printf("mini-shell> ");
  fflush(stdout);
}

int main(int argc, char** argv){
  alarm(120); 

  print_prompt();
 
  return 0;
}