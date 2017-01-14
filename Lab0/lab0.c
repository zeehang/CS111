#include <getopt.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>

void sighandler(int);
void fault();

int main(int argc, char *argv[])
{
  static struct option long_options[] =
  {
    {"input", required_argument, 0, 'i'},
    {"output", required_argument, 0, 'o'},
    {"segfault", no_argument, 0, 's'},
    {"catch", no_argument, 0, 'c'},
    {0,0,0,0}
  };
  int long_index = 0;
  int opt = 0;
  int segflag = 0;
  char* infile = NULL;
  char* outfile = NULL;
  while((opt = getopt_long(argc, argv, "sci:o:", long_options, &long_index))!=-1)
  {
    switch(opt){
      case 'i':
      infile = optarg;
      break;
      case 'o':
      outfile = optarg;
      break;
      case 's':
      segflag =1;
      break;
      case 'c':
      signal(SIGSEGV, sighandler);
      break;
    }
  }
  if(segflag == 1)
  {
    fault();
  }
  if (infile != NULL)
  {
    int ifd = open(infile, O_RDONLY);
    if(errno != 0)
    {
      perror("Error opening file!");
      exit(1);
    }
    if(ifd >= 0)
    {
      close(0);
      dup(ifd);
      close(ifd);
    }
  }
  if(outfile !=NULL)
  {
    int ofd = creat(outfile, 0666);
    if(errno != 0)
    {
      perror("Error creating file!");
      exit(2);
    }
    if(ofd >= 0)
    {
      close(1);
      dup(ofd);
      close(ofd);
    }
  }

  char* buffer;
  buffer = (char*)malloc(sizeof(char));
  int readstatus = read(0, buffer, 1);
  while (readstatus != 0)
  {
    write(1, buffer, 1);
    readstatus = read(0, buffer, 1);
  }
  exit(0);
}
void fault()
{
  char* faultpointer = NULL;
  *faultpointer = 'a';
}
void sighandler(int signum)
{
  if(signum==SIGSEGV)
  {
    write(2, "Error caught: segmentation fault!",33);
  }
  exit(3);
}
