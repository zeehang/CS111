#include <getopt.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>

struct command
{
  int pid;
  char **args;
  int status; //whether its paused or not? could be a 0 or 1
  int numargs;
  int maxargs;
};
int long_index = 0;
int opt = 0;
int* filearray;
int numfiles = 0;
int maxfilenum;
bool verboseflag;
int argindex;

void executecommand(struct command toexecute)
{
  int pid;
  if(verboseflag == true)
  {
    fprintf(stdout, "--command");
    for(int i = 0; i < toexecute.numargs; i++)
    {
      fprintf(stdout, toexecute.args[i]);
    }
  }
  pid = fork();
  if (pid == 0)
  {
    bool fderror = false;
    if(dup2(filearray[atoi(toexecute.args[0])], 0)<0)
      fderror = true;
    if(dup2(filearray[atoi(toexecute.args[1])], 1)<0)
      fderror = true;
    if(dup2(filearray[atoi(toexecute.args[2])], 2)<0)
      fderror = true;
    if(fderror)
      fprintf(stderr, "dup2 error!");
    execvp(toexecute.args[3], &toexecute.args[3]);
  }
  else if (pid > 0)
  {

  }
  return;
}

struct command readargs(int argc, char *argv[])
{
  struct command commandtorun;
  commandtorun.numargs = 0;
  commandtorun.maxargs = 10;
  commandtorun.args = malloc(commandtorun.maxargs*sizeof(char*));
  argindex = optind-1;
  while(argindex < argc && argv[argindex][0] != '-')
  {
    commandtorun.args[commandtorun.numargs] = argv[argindex];
    int fdi = atoi(argv[argindex]);
    if(fdi <0 || fdi >= numfiles)
      fprintf(stderr, "Incorrect file descriptor"); //TODO: you should change to just check i o e after command
    commandtorun.numargs++;
    argindex++;
  }
  optind = argindex;
  if(commandtorun.numargs == commandtorun.maxargs)
  {
    commandtorun.maxargs += 10;
    commandtorun.args = realloc(commandtorun.args, commandtorun.maxargs*sizeof(char*));
    //TODO: check for failure to reallocate
  }
  if(commandtorun.numargs < 4)
  {
    fprintf(stderr, "Not enough arguments!");
  }
  return commandtorun;
}

void reallocarray()
{
  maxfilenum += 10;
  filearray = realloc(filearray, maxfilenum*sizeof(int));
  return;
}

void openfile(int flag)
{
    filearray[numfiles] = open(optarg, flag);
    if(filearray[numfiles] == -1)
    {
      fprintf(stderr, "Error opening file: %s\n", optarg);
    }
    numfiles++;
    if(numfiles == maxfilenum)
      reallocarray();

    return;
}

int main(int argc, char *argv[])
{
  static struct option long_options[]=
  {
    {"rdonly", required_argument, 0, 'r'},
    {"wronly", required_argument, 0, 'w'},
    {"command", required_argument, 0, 'c'},
    {"verbose", no_argument, 0, 'v'},
    {0, 0, 0, 0}
  };
  struct command execute;
  verboseflag = false;
  maxfilenum = 10;
  filearray = malloc(maxfilenum*sizeof(int));
  while((opt = getopt_long(argc, argv, "sci:o:", long_options, &long_index))!=-1)
  {
    switch(opt){
      case 'r':
      openfile(O_RDONLY);
      break;
      case 'w':
      openfile(O_WRONLY);
      break;
      case 'v':
      verboseflag = true;
      break;
      case 'c':
      execute = readargs(argc, argv);
      executecommand(execute);
      break;
    }
  }
  exit(errno);

//TODO: make sure verbose works for all options
//TODO: correctly check for file dsecriptors?
}
