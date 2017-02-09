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
#include <sys/resource.h>
#include <sys/time.h>
#include <string.h>
#include <time.h>

void outputusage(struct rusage begin, struct timeval ustart, struct timeval sstart, bool child);
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
bool profileflag;
struct command cmdarray[1024]; //TODO: make this dynamic!
struct sigaction act;
struct rusage begin, end;
struct timeval ustart, sstart;
//struct rusage begin, end;
struct timeval ustart, uend, sstart, send;

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
      fprintf(stderr, "dup2 error! Invalid file descriptor\n");
    for (int i=3; i<numfiles; i++)
    {
      close(filearray[i]);
    }
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
      close(filearray[atoi(toexecute.args[0])]);
      close(filearray[atoi(toexecute.args[1])]);
      close(filearray[atoi(toexecute.args[2])]);
      toexecute.pid = waitpid(pid, &toexecute.status, 0);
      fprintf(stdout, "Exit status: %i %s", WEXITSTATUS(toexecute.status), "\n");
      //fprintf(stdout, "%s %s", toexecute.args[3], " ");
      for(int i = 3; i<toexecute.numargs; i++)
      {
        fprintf(stdout, "%s %s", toexecute.args[i], " ");
      }
      if(profileflag)
      {
        getrusage(RUSAGE_SELF, &begin);
        fprintf(stdout, "--profile for parent process\n");
        outputusage(begin, ustart, sstart, false);
        getrusage(RUSAGE_CHILDREN, &begin);
        fprintf(stdout, "--profile for child process\n");
        outputusage(begin, ustart, sstart, false);
      }
    }
  }
  return;
}

// void executecommands(int numcmds)
// {
//   for (int i = 0; i < numcmds; i++)
//   {
//     executecommand(cmdarray[i]);
//   }
//   return;
// }

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

void outputusage(struct rusage begin, struct timeval ustart, struct timeval sstart, bool child)
{
    if(child)
    {

    }
    //else
      //getrusage(RUSAGE_SELF, &ru);
  //  ustart = begin.ru_utime;
  //  sstart = begin.ru_stime;
    uend = begin.ru_utime;
    send = begin.ru_stime;
    double uudiff = (double)uend.tv_usec - (double)ustart.tv_usec;
    double usdiff = (double)uend.tv_sec - (double)ustart.tv_sec;
    double sudiff = (double)send.tv_usec - (double)send.tv_sec;
    double ssdiff = (double)send.tv_sec - (double)sstart.tv_sec;
    // fprintf(stdout, "end time -U %.9f\n", uend.tv_usec);
    // fprintf(stdout, "start time -U %.9f\n", ustart.tv_usec);
    // fprintf(stdout, "end time -S %.9f\n", send.tv_usec);
    // fprintf(stdout, "start time -S %.9f\n", sstart.tv_usec);
    // fprintf(stdout, "sdiff %.9f\n", sudiff);
    // fprintf(stdout, "udiff %.9f\n", uudiff);
    if(uudiff < 0)
    {
      usdiff -= 1L;
      uudiff = -uudiff;
    }

    if(sudiff <0)
    {
      ssdiff -= 1L;
      sudiff = -sudiff;
    }
    double const divisor = 1000000L;
    uudiff /= divisor;
    sudiff /= divisor;
  //  long double test = (3.0)/(5.0);
  //  fprintf(stdout, "%f", test);
    fprintf(stdout, "System time taken: %.6f seconds\n", (double)(ssdiff+sudiff),"\n" );
    fprintf(stdout, "User time taken: %.6f seconds\n", (double)(usdiff+uudiff),"\n" );
    fprintf(stdout, "Maximum resident set size: %f\n", begin.ru_maxrss);
    fprintf(stdout, "Voluntary context switches: %d\n", begin.ru_nvcsw);
    fprintf(stdout, "Involuntary context switches: %d\n", begin.ru_nivcsw);
    fprintf(stdout, "Page reclaims: %d\n", begin.ru_minflt);
    fprintf(stdout, "Page faults: %d\n", begin.ru_majflt);
    fprintf(stdout, "Signals recieved: %d\n\n", begin.ru_nsignals);
  //  fprintf(stdout, "\n%d", (3.0/5.6), "\n");
  return;
}

void openfile(int flag)
{
    filearray[numfiles] = open(optarg, flag, S_IRUSR | S_IWUSR | S_IROTH);
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
  // struct timespec overalltimestart, overalltimeend;
  // clock_gettime(CLOCK_MONOTONIC, &overalltimestart);
  static struct option long_options[]=
  {
    {"rdonly", required_argument, 0, 'r'},
    {"wronly", required_argument, 0, 'w'},
    {"rdwr", required_argument, 0, 'b'},
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
    {"profile", no_argument, 0, 'f'},
    {0, 0, 0, 0}
  };
  int fileflags = 0;
  waitflag = false;
  profileflag = false;
  struct command execute;
  act.sa_handler = sighandler;
  int commandcounter = 0;
  verboseflag = false;
  maxfilenum = 10;
  filearray = malloc(maxfilenum*sizeof(int));
  for (int i = 0; i < argc; i++)
  {
    if(strcmp("--wait", argv[i]) ==0)
    {
      waitflag = true;
      break;
    }
  }
  while((opt = getopt_long(argc, argv, "sci:o:", long_options, &long_index))!=-1)
  {
    if(profileflag)
    {
      getrusage(RUSAGE_SELF, &begin);
      ustart = begin.ru_utime;
      sstart = begin.ru_stime;
    }
    switch(opt){
      case 'r':
      fileflags |= O_RDONLY;
      openfile(fileflags);
      if(profileflag)
      {
        getrusage(RUSAGE_SELF, &begin);
        fprintf(stdout, "--profile for opening a file read-only\n");
        outputusage(begin, ustart, sstart, false);
      }
      fileflags =0;
      break;
      case 'w':
      fileflags |= O_WRONLY;
      openfile(fileflags);
      if(profileflag)
      {
        getrusage(RUSAGE_SELF, &begin);
        fprintf(stdout, "--profile for opening a file write-only\n");
        outputusage(begin, ustart, sstart, false);
      }
      fileflags =0;
      break;
      case 'b':
      fileflags |= O_RDWR;
      openfile(fileflags);
      if(profileflag)
      {
        getrusage(RUSAGE_SELF, &begin);
        fprintf(stdout, "--profile for opening a file read and write\n");
        outputusage(begin, ustart, sstart, false);
      }
      fileflags = 0;
      break;
      case 'v':
      verboseflag = true;
      break;
      case 'c':
      execute = readargs(argc, argv);
      // cmdarray[commandcounter] = execute;
      // commandcounter++;
      executecommand(execute);
      if(profileflag  && !waitflag)
      {
        getrusage(RUSAGE_SELF, &begin);
        fprintf(stdout, "--profile for command with no --wait option\n");
        outputusage(begin, ustart, sstart, false);
      }
      break;
      case 'p':
      openpipe();
      if(profileflag)
      {
        getrusage(RUSAGE_SELF, &begin);
        fprintf(stdout, "--profile for opening a pipe\n");
        outputusage(begin, ustart, sstart, false);
      }
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

      sigaction(atoi(optarg), &act, NULL);
      //signal(atoi(optarg), sighandler);
      if(profileflag)
      {
        getrusage(RUSAGE_SELF, &begin);
        fprintf(stdout, "--profile for signal handler\n");
        outputusage(begin, ustart, sstart, false);
      }
      break;
      }

      // case 'i':
      // waitflag = true;
      // break;
      case 'x':
      if(close(filearray[atoi(optarg)])==-1)
        fprintf(stderr, "Error closing file");
        if(profileflag)
        {
          getrusage(RUSAGE_SELF, &begin);
          fprintf(stdout, "--profile for closing a file\n");
          outputusage(begin, ustart, sstart, false);
        }
      break;
      case 'g':
      //signal(atoi(optarg), SIG_IGN);
      {
        struct sigaction ignore;
        ignore.sa_handler = SIG_IGN;
        sigaction(atoi(optarg), &ignore, NULL);
      break;
      }
      case 'd':
      {
        struct sigaction ignore;
        ignore.sa_handler = SIG_DFL;
        sigaction(atoi(optarg), &ignore, NULL);
      break;
      }
      //signal(atoi(optarg), SIG_DFL);
      break;
      case 'u':
      pause();
      break;
      case 'f':
      profileflag = true;
      break;
    }
  }
  //executecommands(commandcounter);
  // if(profileflag)
  // {
  //   clock_gettime(CLOCK_MONOTONIC, &overalltimeend);
  //   fprintf(stdout, "\n====================\noverall time taken simple shell\n");
  //   double ndiff = (double)overalltimeend.tv_nsec - (double)overalltimestart.tv_nsec;
  //   double sdiff = (double)overalltimeend.tv_sec - (double)overalltimestart.tv_sec;
  //   if(ndiff < 0)
  //   {
  //     sdiff -= 1L;
  //     ndiff = -ndiff;
  //   }
  //   double const divisor = 1000000000L;
  //   ndiff /= divisor;
  //   fprintf(stdout, "Overall time used by simple shell in seconds is: %d", (double)(sdiff+ndiff));
  // }
  // exit(errno);

//TODO: make sure verbose works for all options
//TODO: correctly check for file dsecriptors?
}
