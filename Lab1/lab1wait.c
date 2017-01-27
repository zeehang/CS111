#include <getopt.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>


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
bool waitflag;
struct command cmdarray[1024]; //TODO: make this dynamic!

void sighandler(int signum)
{
  fprintf(stderr, "%d %s", signum, "caught\n");
  exit(signum);
}


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
    toexecute.pid = getpid();
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
    if(waitflag)
    {
      // for (int i = 0; i < numfiles; i++)
      // {
      //   close(filearray[i]);
      // }
      toexecute.pid = wait(&toexecute.status);
      fprintf(stdout, "Exit status: %i %s", WEXITSTATUS(toexecute.status), "\n");
      //fprintf(stdout, "%s %s", toexecute.args[3], " ");
      for(int i = 3; i<toexecute.numargs; i++)
      {
        fprintf(stdout, "%s %s", toexecute.args[i], " ");
      }

    }
  }
  return;
}

void executecommands(int numcmds)
{
  for (int i = 0; i < numcmds; i++)
  {
    executecommand(cmdarray[i]);
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
  while(argindex < argc && argv[argindex][0] != '-' && argv[argindex][1] != '-')
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


void openpipe()
{
  //TODO: create new pfd array? should it be in command struct? or should it be in file array
  int pipefd[2];
  pid_t cpid;
  if(pipe(pipefd) == -1)
  {
    fprintf(stderr, "Error opening pipe");
  }
  //dup2(filearray[numfiles], pipefd[0]);
  filearray[numfiles] = pipefd[0];
  numfiles++;
  //  dup2(filearray[numfiles], pipefd[1]);

  filearray[numfiles] = pipefd[1];
    //close(pipefd[1]);
  numfiles++;
  return;
}

void fault()
{
  char* faultpointer = NULL;
  *faultpointer = 'a';
}

int main(int argc, char *argv[])
{
  static struct option long_options[]=
  {
    {"rdonly", required_argument, 0, 'r'},
    {"wronly", required_argument, 0, 'w'},
    {"command", required_argument, 0, 'c'},
    {"verbose", no_argument, 0, 'v'},
    {"append", no_argument, 0, '1'},
    {"cloexec", no_argument, 0, '2'},
    {"creat", no_argument, 0, '3'},
    {"directory", no_argument, 0, '4'},
    {"dsync", no_argument, 0, '5'},
    {"excl", no_argument, 0, '6'},
    {"nofollow", no_argument, 0, '7'},
    {"nonblock", no_argument, 0, '8'},
    {"rsync", no_argument, 0, '9'},
    {"sync", no_argument, 0, 's'},
    {"trunc", no_argument, 0, 't'},
    {"pipe", no_argument, 0, 'p'},
    {"abort", no_argument, 0, 'a'},
    {"catch", required_argument, 0, 'h'},
    {"wait", no_argument, 0, 'i'},
    {"close", required_argument, 0, 'x'},
    {"ignore", required_argument, 0, 'g'},
    {"default", required_argument, 0, 'd'},
    {"pause", no_argument, 0, 'u'},
    {0, 0, 0, 0}
  };
  int fileflags = 0;
  waitflag = false;
  struct command execute;

  int commandcounter = 0;
  verboseflag = false;
  maxfilenum = 10;
  filearray = malloc(maxfilenum*sizeof(int));
  int argccopy = argc;
  char *argvcopy = argv;
  while((opt = getopt_long(argc, argv, "sci:o:", long_options, &long_index))!=-1)
  {
    switch(opt){
      case 'i':
      waitflag = true;
      break;
    }
  }
  while((opt = getopt_long(argccopy, argvcopy, "sci:o:", long_options, &long_index))!=-1)
  {
    switch(opt){
      case 'r':
      fileflags |= O_RDONLY;
      openfile(fileflags);
      fileflags =0;
      break;
      case 'w':
      fileflags |= O_WRONLY;
      openfile(fileflags);
      fileflags =0;
      break;
      case 'v':
      verboseflag = true;
      break;
      case 'c':
       execute = readargs(argc, argv);
      // cmdarray[commandcounter] = execute;
      // commandcounter++;
      executecommand(execute);
      break;
      case 'p':
      openpipe();
      break;
      case '1':
      fileflags |= O_APPEND;
      break;
      case '2':
      fileflags |= O_CLOEXEC;
      break;
      case '3':
      fileflags |= O_CREAT;
      break;
      case '4':
      fileflags |= O_DIRECTORY;
      break;
      case '5':
      fileflags |= O_DSYNC;
      break;
      case '6':
      fileflags |= O_EXCL;
      break;
      case '7':
      fileflags |= O_NOFOLLOW;
      break;
      case '8':
      fileflags |= O_NONBLOCK;
      break;
      case '9':
      fileflags |= O_RSYNC;
      break;
      case 's':
      fileflags |= O_SYNC;
      break;
      case 't':
      fileflags |= O_TRUNC;
      break;
      case 'a':
      fault();
      break;
      case 'h':
      {
      // struct sigaction errorfun;
      // errorfun.sa_handler = sighandler;
      signal(atoi(optarg), sighandler);
      break;
      }
      // case 'i':
      // waitflag = true;
      // break;
      case 'x':
      close(filearray[atoi(optarg)]);
      break;
      case 'g':
      signal(atoi(optarg), SIG_IGN);
      break;
      case 'd':
      signal(atoi(optarg), SIG_DFL);
      break;
      case 'u':
      pause();
      break;
    }
  }
  //executecommands(commandcounter);

  exit(errno);

//TODO: make sure verbose works for all options
//TODO: correctly check for file dsecriptors?
}
