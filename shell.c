#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

// struct for jobs 
struct job {
  int pid;
  int state;
  char *cmd;
};

// tracking jobs 
const int MAX_JOBS = 5;
int job_count = 0;
struct job jobs[MAX_JOBS];

// track history of commands
const int MAX_HISTORY = 100;
char *history[MAX_HISTORY];
int history_count = 0;

// max number of characters shell can read from input
const int BUFFER_SIZE = 80;

// pid of the current foreground process
int fg_pid = -1;

// track current command
char current_cmd[BUFFER_SIZE];

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
    for (int i = 0; i < job_count; i++) {
      kill(jobs[i].pid, SIGINT);
      waitpid(jobs[i].pid, NULL, 0);
    }
    exit(0); 
  } else if (sig == SIGTSTP) {

    if (fg_pid > 0) {
      kill(fg_pid, SIGTSTP);

      // track jobs in the background
      if (job_count < MAX_JOBS) {
        jobs[job_count].pid = fg_pid;
        jobs[job_count].state = 1; 
        jobs[job_count].cmd = strdup(current_cmd); 
        printf("\nsuspended   %s   (%d)", jobs[job_count].cmd, fg_pid);
        job_count++;
      } else {
        printf("\nsuspended   %d   but job table full", fg_pid);
      }
    }
    fg_pid = -1;
    printf("\n");

  }
}

// function that runs when input has a pipe
void execute_piped_input (char *input) {
  // splits input by each pipe
  char *commands[BUFFER_SIZE];
  int num_cmds = 0;
  char *cmd = strtok(input, "|");
  while (cmd != NULL) {
      commands[num_cmds++] = cmd;
      cmd = strtok(NULL, "|");
  }

  // file descriptor
  int fd[2];
  int fd_in = 0;

  for (int i = 0; i < num_cmds; i++) {
    pipe(fd);
    pid_t pid = fork();

    // child process
    if (pid == 0) {
      dup2(fd_in, STDIN_FILENO); // read from fd_in
      if (i < num_cmds - 1) {
        dup2(fd[1], STDOUT_FILENO); // write to the next pipe process
      }
      
      close(fd[0]);
      close(fd[1]);

      // parse and execute the command
      char *args[BUFFER_SIZE];
      int j = 0;
      char *p = commands[i];

      while (*p != '\0') {
        while (*p == ' ') p++; 
        if (*p == '\0') break;

        char *start;
        if (*p == '"' || *p == '\'') {
            char quote = *p++;
            start = p;
            while (*p && *p != quote) p++;
            if (*p) *p++ = '\0';  
        } else {
            start = p;
            while (*p && *p != ' ') p++;
            if (*p) *p++ = '\0';
        }
        args[j++] = start;
      }
      args[j] = NULL;

      execvp(args[0], args);
      fprintf(stderr, "exec failed\n");
      exit(1);
    } else if (pid > 0) {
      waitpid(pid, NULL, 0);
      close(fd[1]);
      fd_in = fd[0];
    } else {
      fprintf(stderr, "fork failed\n");
    }
  }
}

// function that runs when there is conditional input
void execute_conditional_input(char *input) {
  char *commands[BUFFER_SIZE];  // store individual commands
  char *operators[BUFFER_SIZE]; // store operators between commands
  int num_cmds = 0;

  char *token = strtok(input, " ");
  char cur_command[BUFFER_SIZE];
  cur_command[0] = '\0';

  // separate input into commands and operators
  while (token != NULL) {
    if (strcmp(token, "&&") == 0 || strcmp(token, "||") == 0 || strcmp(token, ";") == 0) {
      commands[num_cmds] = strdup(cur_command); // save the command
      operators[num_cmds] = token; // save the operator
      num_cmds++;
      cur_command[0] = '\0';
    } else {
      strcat(cur_command, token);
      strcat(cur_command, " ");
    }
    token = strtok(NULL, " ");
  }

  // save the last command if there is any
  if (strlen(cur_command) > 0) {
    commands[num_cmds] = strdup(cur_command);
    operators[num_cmds] = NULL;
    num_cmds++;
  }

  int run_next = 1;

  for (int i = 0; i < num_cmds; i++) {
    // skip this command if previous condition says not to run
    if (run_next == 0) {
      if (operators[i-1] && strcmp(operators[i-1], "&&") == 0) continue;
      if (operators[i-1] && strcmp(operators[i-1], "||") == 0) continue;
    }

    // parse the command into arguments
    char *args[BUFFER_SIZE];
    int j = 0;
    char *p = commands[i];

    while (*p != '\0') {
      while (*p == ' ') p++; 
      if (*p == '\0') break;

      char *start;
      if (*p == '"' || *p == '\'') {
          char quote = *p++;
          start = p;
          while (*p && *p != quote) p++;
          if (*p) *p++ = '\0';  
      } else {
          start = p;
          while (*p && *p != ' ') p++;
          if (*p) *p++ = '\0';
      }
      args[j++] = start;
    }
    args[j] = NULL;

    int exit_value = 0;
    pid_t pid = fork(); 
    if (pid == 0) {
      execvp(args[0], args);
      fprintf(stderr, "exec failed\n");
      exit(1);
    } else {
      int status;
      waitpid(pid, &status, 0);
      exit_value = WEXITSTATUS(status);
    }

    // determine if next command should run based on operator
    if (operators[i] != NULL) {
      if (strcmp(operators[i], "&&") == 0) run_next = (exit_value == 0);
      else if (strcmp(operators[i], "||") == 0) run_next = (exit_value != 0);
      else run_next = 1;
    }
  }
}

// function to check if background jobs have completed
void check_background_jobs() {
  for (int i = 0; i < job_count; i++) {
    int status;
    pid_t result = waitpid(jobs[i].pid, &status, WNOHANG);

    if (result > 0 && (WIFEXITED(status) || WIFSIGNALED(status))) { 
      printf("[%d]   done   %s   (%d)\n", i+1, jobs[i].cmd, jobs[i].pid);
      free(jobs[i].cmd);

      for (int j = i; j < job_count - 1; j++) {
          jobs[j] = jobs[j+1];
      }
      job_count--;
      i--; 
    }
  }
}

// adds a command to history of commands
void add_to_history(const char *cmd) {
  if (history_count < MAX_HISTORY)
      history[history_count++] = strdup(cmd);
}

// built in cd function
void cd_func(char **args) {
  char *target_dir = args[1];
  if (!target_dir) target_dir = getenv("HOME");
  if (chdir(target_dir) != 0) fprintf(stderr, "cd failed\n");
}

// built in exit function
void exit_func(char **args) {
  exit(0);
}

// built in help function
void help_func(char **args) {
  printf("Built-in commands:\ncd\nexit\nhelp\nfg\njobs\nbg\n");
}

// built in fg function
void fg_func(char **args) {
  int index = job_count - 1; 
  if (args[1]) index = atoi(args[1]) - 1;

  if (index >= 0 && index < job_count) {
    int pid = jobs[index].pid;
    printf("[%d]   running   %s   (%d)\n", index+1, jobs[index].cmd, pid);

    fg_pid = pid;
    kill(pid, SIGCONT);
    waitpid(pid, NULL, WUNTRACED);
    jobs[index].state = 0;

    for (int i = index; i < job_count-1; i++) jobs[i] = jobs[i+1];
    job_count--;
    fg_pid = -1;
  }
}

// built in jobs function
void jobs_func(char **args) {
  check_background_jobs();

  // print out the jobs
  for (int i = 0; i < job_count; i++) {
    if (jobs[i].state == 0) printf("[%d]   running   %s   (%d)\n", i+1, jobs[i].cmd, jobs[i].pid);
    else if (jobs[i].state == 1) printf("[%d]   suspended   %s   (%d)\n", i+1, jobs[i].cmd, jobs[i].pid);
  }
}

// built in bg function
void bg_func(char **args) {
  int index = job_count - 1;
  if (args[1]) index = atoi(args[1]) - 1;

  if (index >= 0 && index < job_count) {
    int pid = jobs[index].pid;
    printf("[%d]   continued   %s   (%d)\n", index+1, jobs[index].cmd, pid);
    kill(pid, SIGCONT);
    jobs[index].state = 0;
  }
}

// built in function that prints the past commands
void history_func(char **args) {
  for (int i = 0; i < history_count; i++) {
      printf("[%d] %s\n", i + 1, history[i]);
  }
}

// keep track of built ins
char *builtins[] = {"cd", "exit", "help", "fg", "jobs", "bg", "history"};
void (*builtin_funcs[])(char **) = {&cd_func, &exit_func, &help_func, &fg_func, &jobs_func, &bg_func, &history_func};
int num_builtins = sizeof(builtins) / sizeof(char *);

int main(int argc, char** argv){
  alarm(120); // set a timer for 120 seconds to prevent accidental infinite loops or fork bombs
  signal(SIGINT, sig_handler);
  signal(SIGTSTP, sig_handler);
  
  // buffer to store user input
  char input[BUFFER_SIZE];

  while (1) {
    check_background_jobs();
    print_prompt();

    // read input from user
    if (fgets(input, BUFFER_SIZE, stdin) == NULL) {
      printf("\n"); 
      break;
    }

    input[strcspn(input, "\n")] = '\0';
    if (strlen(input) == 0) continue;

    // get the current command
    strncpy(current_cmd, input, sizeof(current_cmd)-1);
    current_cmd[sizeof(current_cmd)-1] = '\0';

    add_to_history(input);

    // check if command should run in background
    int run_in_bg = 0;
    int len = strlen(input);

    // removes the & from a background command
    while (len > 0 && (input[len-1] == '&' || input[len-1] == ' ')) {
      if (input[len-1] == '&') run_in_bg = 1;
      input[--len] = '\0'; 
    }
    strncpy(current_cmd, input, sizeof(current_cmd)-1);
    current_cmd[sizeof(current_cmd)-1] = '\0';

    // checks if the input has a pipe in it
    int has_pipe = strchr(input, '|') != NULL;
    if (has_pipe) {
      execute_piped_input(input);
      continue;
    }

    // check if there is conditional input
    if (strstr(input, "&&") || strstr(input, "||") || strchr(input, ';')) {
      execute_conditional_input(input);
      continue;
    }

    // reads input
    char *args[BUFFER_SIZE];
    int j = 0;
    char *p = input;

    while (*p != '\0') {
        while (*p == ' ') p++; 
        if (*p == '\0') break;

        char *start;
        if (*p == '"' || *p == '\'') {
            char quote = *p++;
            start = p;
            while (*p && *p != quote) p++;
            if (*p) *p++ = '\0';  
        } else {
            start = p;
            while (*p && *p != ' ') p++;
            if (*p) *p++ = '\0';
        }
        args[j++] = start;
    }
    args[j] = NULL;

    // checks if a command is built in
    int built_in = 0;
    for (int i = 0; i < num_builtins; i++) {
      if (strcmp(args[0], builtins[i]) == 0) {
        builtin_funcs[i](args); 
        built_in = 1;
        break;
      }
    }
    if (built_in) continue;

    // create a new process to execute the command
    pid_t pid = fork();
    if (pid == 0) {
      execvp(args[0], args);
      fprintf(stderr, "exec failed\n");
      exit(1);
    } else if (pid > 0) {
      if (run_in_bg) {
        if (job_count < MAX_JOBS) {
          jobs[job_count].pid = pid;
          jobs[job_count].state = 0;
          jobs[job_count].cmd = strdup(current_cmd);
          job_count++;
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