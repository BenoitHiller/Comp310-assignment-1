#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

#define NO_PIPE 0
#define PIPE_IN -1
#define PIPE_OUT 1
#define BUFFSIZE 512
#define DEBUG 0
char **envp;
int exit_num;

char ** split_spaces_cmd(const char * name,const char * params){
  int i = 1;
  char delims[] = " ";
  char * next = NULL;
  char * i_am_starting_to_get_frustrated_with_the_elements_tangential_to_course_content = strdup(params);
  char **it_is_just_going_to_concat_this_anyway = (char**)malloc(sizeof(char**)*(strlen(params) + 2));
  it_is_just_going_to_concat_this_anyway[0] = strdup(name);
  next = strtok(i_am_starting_to_get_frustrated_with_the_elements_tangential_to_course_content,delims);
  if(next == NULL){
    it_is_just_going_to_concat_this_anyway[i] = strdup(params);
    i++;
  }
  else {
    while(next != NULL){
      it_is_just_going_to_concat_this_anyway[i] = strdup(next);
      i++;
      next = strtok(NULL,delims);
    }
  }
  it_is_just_going_to_concat_this_anyway[i] = (char *)0;
  return it_is_just_going_to_concat_this_anyway;
}

char * sub_q(char * input){
  char * buffer = strdup(input);
  char * buffer2;
  char code[4];
  sprintf(code,"%d",exit_num);
  for(int i = 0; buffer[i] != '\0';i++){
    if(buffer[i] == '$' && buffer[i+1] == '?'){
      buffer2 = (char *)malloc(strlen(input) + 2);
      strncpy(buffer2,input,i);
      strncpy(&buffer2[i],code,3);
      strcat(buffer2,&input[i+2]);
      i += strlen(code) - 2;
      buffer = strdup(buffer2);
    }
  }
  return buffer;
}

typedef struct StackNode
{
    char* item;
    struct StackNode * next;
} StackNode;

StackNode * stack_push(StackNode * head, const char * noot){
  StackNode * new_node = (StackNode *)malloc(sizeof(*new_node));
  if(new_node == NULL){return NULL;}
  new_node->item = strdup(noot);
  new_node->next = head;
  return new_node;
}

char * stack_pop(StackNode * head){
  char * item = head->item;
  head = head->next;
  return item;
}

void print_stack(StackNode * node){
  if(node != NULL){
    printf("%s",node->item);
    if(node->next != NULL){
      printf(" ");
      print_stack(node->next);
    }
  }
}

void print_stack_lines(StackNode * node){
  if(node->next != NULL){
    print_stack_lines(node->next);
    printf("%s\n",node->item);
  }
  else if(node == NULL){}
  else {
    printf("%s\n",node->item);
  }
}

StackNode * root;
StackNode * history;

// Command functions
int cd(const char * dir){
  if(dir == NULL){return 0;}
  else{
    return chdir(dir);
  }
}
int pwd(){
  char * pwd;
  if((pwd = getcwd(NULL,0)) == NULL){return 1;}
  else {
    printf("%s\n",pwd);
  }
  free(pwd);
  return 0;
}
int echo(const char * string){
  if(string == NULL){return 0;}
  if((strlen(string) >= 2) && (string[0] == '-' && string[1] == 'e')){
    printf("%s", &string[2]);
  }
  else{
    printf("%s\n",string);
  }
  return 0;
}
int pushd(const char * dir){
  int ret = 0;
  char * pwd;
  if((pwd = getcwd(NULL,0)) == NULL){return 1;}
  if(dir == NULL){
    if(root == NULL){return 1;}
    else{
      char * top = stack_pop(root);
      root = stack_push(root,pwd);
      printf("%s ", top);
      print_stack(root);
      printf("\n");
      return cd(top);
    }
  }
  else {
    ret = cd(dir);
    char * new_dir;
    if((new_dir = getcwd(NULL,0)) == NULL){return 1;}
    root = stack_push(root,pwd);
    printf("%s ", new_dir);
    print_stack(root);
    printf("\n");
    return ret;
  }
}

int popd(){
  print_stack(root);
  printf("\n");
  char * new_dir = stack_pop(root);
  root = root->next;
  return cd(new_dir);
}

void exitc(const char * code){
  if(code == NULL){
    exit(0);
  }
  else {
    int ret = atoi(code);
    if(0 <= ret && ret <= 255){
      exit(ret);
    }
    else {
      exit(1);
    }
  }
}

int env(){
  char **iter = envp;
  for(;iter != NULL;iter++){
    printf("%s\n", *iter);
  }
  return 1;
}
//big ass switchy thingy
int find_and_exec(const char * command_name, const char * parameters){
  int ret;
  if(command_name == NULL){
    return 0;
  }
  else if(strcmp(command_name,"pwd") == 0){
    return pwd();
  }
  else if(strcmp(command_name,"cd") == 0){
    return cd(parameters);
  }
  else if(strcmp(command_name,"echo") == 0){
    return echo(parameters);
  }
  else if(strcmp(command_name,"pushd") == 0){
    return pushd(parameters);
  }
  else if(strcmp(command_name,"popd") == 0){
    return popd();
  }
  else if(strcmp(command_name,"exit") == 0){
    exitc(parameters);
  }
  else if(strcmp(command_name,"set") == 0){
    printf("Did you mean export? If you are looking for windows try your walls\n");
    return 0;
  }
  else if(strcmp(command_name,"history") == 0){
    print_stack_lines(history);
    return 0;
  }
  else if(strcmp(command_name,"unsetenv") == 0){
    return unsetenv(parameters);
  }
  else {
    int pid = fork();
    if(pid == -1){return 1;}
    else if(pid == 0){
      if(parameters == NULL) {
        execlp(command_name,command_name,NULL);
      }
      else {
        execvp(command_name,split_spaces_cmd(command_name,parameters));
      }
    }
    else {
      wait(&ret);
      return ret;
    }
  }
  return 0;
}

int parse_exec(const char* input){
  if(input[0] == '\0'){return 1;}
  int ret = 0;
  int head = 0;
  int pipev = NO_PIPE;
  int pipeline = 0;
  char * parsable = (char *)strdup(input);
  if (parsable == NULL){
    return 1;
  }
  char * command = NULL;
  char * params = NULL;
  char * file = NULL;

  for(;parsable[head] == ' ' || parsable[head] == '\t';head++){}
  int tail = head;
  for(;parsable[head] != '\0';head++){
    if((parsable[head] == ' ' || parsable[head] == '\t') && command == NULL && pipev == NO_PIPE){//first word terminated by space character
      command = (char *)malloc(sizeof(char) *(head - tail + 1));
      if (command == NULL){return 1;}
      if (strncpy(command,parsable + tail,head-tail) == NULL){return 1;}
      command[head-tail] = '\0';
      for(;parsable[head] == ' ' || parsable[head] == '\t';head++){}//discard any extra spaces after the first one
      tail = head;
      head--;//the head is at the next character but is going to get incremented so we must decrement first
    }
    else if((parsable[head] == '<' ||  parsable[head] == '>') && pipev == NO_PIPE){
      if (parsable[head] == '<'){// this has a bug that I don't need to fix because the spec changed
        pipev = PIPE_IN;
      }
      else {
        pipev = PIPE_OUT;
      }
      if (tail != head){
        if(command == NULL){//the first word is terminated with a pipe rather than a space
          command = (char *)malloc(head - tail + 1);
          if (strncpy(command,parsable + tail,head-tail) == NULL){return 1;}
          command[head-tail] = '\0';
          tail = head + 1;//We set the tail ahead of the head so that we don't include the current character
        }
        else {
          int back = head;
          for(;parsable[back] == ' ' || parsable[back] == '\t' || parsable[back] == '>'|| parsable[back] == '<';back--){}
          back++;
          params = (char *)malloc(back - tail + 1);
          if (strncpy(params,parsable + tail,back-tail) == NULL){return 1;}
          params[back-tail] = '\0';
          tail = head + 1;
        }
      }
    }
    else if(parsable[head] == '|'){
      if (pipev != NO_PIPE){//redirect was found first
        for(;parsable[tail] == ' ' || parsable[tail] == '\t';tail++);//no reason to have spaces at head of the redirect file
        if (head != tail){//redirect was not the previous character
          file = (char *)malloc(head - tail + 1);
          if (strncpy(file,parsable + tail,head-tail) == NULL){return 1;}
          file[head-tail] = '\0';
        }
      }
      else if(command == NULL){
        if(tail != head){//the first word is terminated with a pipe rather than a space
          command = (char *)malloc(head - tail + 1);
          if (strncpy(command,parsable + tail,head-tail) == NULL){return 1;}
          command[head-tail] = '\0';
        }
        //this "branch" denotes that the pipe is the first character
      }
      else {// input with params and no redirect or a few other things
        int back = head;
        for(;parsable[back] == ' ' || parsable[back] == '\t' || parsable[back] == '|';back--){}
        back++;
        if(tail < back){
        params = (char *)malloc(back - tail + 1);
        if (strncpy(params,parsable + tail,back-tail) == NULL){return 1;}
        params[back-tail] = '\0';
        }
      }
      pipeline = 1;
      head++;
      tail = head;//Now both head and tail should be set ahead of the pipe
      break;
    }
  }
  if(tail != head){
    if(parsable[tail] == '<' || parsable[tail] == '>' || parsable[tail] == '|'){tail++;}
    for(;tail < strlen(parsable) && (parsable[tail] == ' ' || parsable[tail] == '\t');tail++);
  }
  if(tail != head && pipeline == 0){
    if(pipev == PIPE_IN || pipev == PIPE_OUT){
      file = (char *)malloc(head - tail + 1);
      if (strncpy(file,parsable + tail,head-tail) == NULL){return 1;}
      file[head-tail] = '\0';
      tail = head;
    }
    else if(command == NULL){
      command = (char *)malloc(head - tail + 1);
      if (strncpy(command,parsable + tail,head-tail) == NULL){return 1;}
      command[head-tail] = '\0';
      tail = head;
    }
    else {
      int num = head-tail;
      params = (char *)malloc(sizeof(char)*(head - tail + 1));
      if (strncpy(params,parsable + tail,num) == NULL){return 1;}
      params[head-tail] = '\0';
      tail = head;
    }
  }
  if(DEBUG == 1){
    if(command != NULL){
      printf("Command is: %s\n", command);
    }
    if(params != NULL){
      printf("Params are: %s\n", params);
    }
    if(file != NULL){
      printf("File is: %s\n", file);
    }
    printf("pipev: %d\n",pipev);
    printf("pipeline: %d\n",pipeline);
  }
  int oldredir;
  int pipefile;
  if(pipev == PIPE_IN){
    oldredir = dup(STDIN_FILENO);
    if(file != NULL){
      pipefile = open(file,O_RDONLY);//add file check
      dup2(pipefile,STDIN_FILENO);
      close(pipefile);
    }
  }
  else if(pipev == PIPE_OUT){
    oldredir = dup(STDOUT_FILENO);
    fflush(stdout);
    if(file != NULL){
      pipefile = open(file,O_WRONLY|O_CREAT,0777);
      dup2(pipefile,STDOUT_FILENO);
      close(pipefile);
    }
    else {
      dup2(STDIN_FILENO,STDOUT_FILENO);
    }
  }
  if(pipeline == 1){
    int oldredir2;
    int tubing[2];
    pid_t pid;
    if (pipe(tubing) < 0) {return 1;}
    if ((pid = fork()) < 0) {return 1;}
    else if (pid == 0){
      close(tubing[1]);
      oldredir2 = dup(STDIN_FILENO);
      dup2(tubing[0], STDIN_FILENO);
      ret = parse_exec(&parsable[head]);
      close(tubing[0]);
      dup2(oldredir2, STDIN_FILENO);
      exit(ret);
    }
    else {
      fflush(stdout);
      close(tubing[0]);
      oldredir2 = dup(STDOUT_FILENO);
      dup2(tubing[1], STDOUT_FILENO);
      find_and_exec(command,params);
      fflush(stdout);
      close(tubing[1]);
      dup2(oldredir2, STDOUT_FILENO);
      wait(&ret);
    }
  }
  else{
    ret = find_and_exec(command,params);
  }
  if(pipev == PIPE_IN){
    dup2(oldredir,STDIN_FILENO);
  }
  else if(pipev == PIPE_OUT){
    fflush(stdout);
    dup2(oldredir,STDOUT_FILENO);
  }
  return ret;
}
int main(int argc, char *argv[], char *envp[]){
  root = NULL;
  history = NULL;
  exit_num = 0;
  size_t buff = BUFFSIZE;
  int bytes_read;
  char * cmd_line;
  cmd_line = (char*)malloc(buff + 1);
  for(;;){
#ifdef PROMPT
    printf("==>");
#endif
    bytes_read = getline(&cmd_line,&buff,stdin);//yeah prompt but stupid trailing newline
    if (bytes_read == -1){return 1;}
    else if(cmd_line[strlen(cmd_line) -1] == '\n'){
      cmd_line[strlen(cmd_line) -1] = '\0';
    }
    history = stack_push(history,cmd_line);
    cmd_line = sub_q(cmd_line);
    exit_num = parse_exec(cmd_line);
  }
  printf("goodbye\n");
  return 1;
}
