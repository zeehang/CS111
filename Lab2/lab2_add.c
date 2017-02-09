#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>

void add(long long *pointer, long long value)
{
  long long sum = *pointer + value;
  *pointer = sum;
}

int main(int argc, char* argv[])
{
  //Initialization of variables
  int numthreads = 1;
  int iterations = 1;
  long long counter = 0;
  struct timespec startingtime;
  int opt = 0;
  int long_index = 0;

  //options structure
  static struct option long_options[]=
  {
    {"threads", required_argument, 0, 't'},
    {"iterations", required_argument, 0, 'i'},
    {0, 0, 0, 0}
  };

  while((opt = getopt_long(argc, argv, "sci:o:", long_options, &long_index))!=-1)
  {
    switch(opt)
    {
      case 'i':
        iterations = atoi(optarg);
        break;
      case 't':
        numthreads = atoi(optarg);
        break;
    }
  }

  clock_gettime(CLOCK_MONOTONIC, &startingtime);
  
}
