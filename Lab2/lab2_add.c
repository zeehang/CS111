#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>


//initizliation of global variables
long long counter = 0;
int opt_yield;
int additionalflags = 0; //this flag is 1 if the output is NOT add-none
char syncvar;
int protectionflags = 0;
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
volatile int spin_lock = 0;

void add(long long *pointer, long long value)
{
  long long sum = *pointer + value;
  if(opt_yield)
    sched_yield();
  *pointer = sum;
}

void* newthread()
{
  //this calls add! friends are family!
  if(protectionflags)
  {
    switch(syncvar)
    {
      case 'm':
        pthread_mutex_lock(&mut);
        add(&counter, 1);
        add(&counter, -1);
        break;
      case 's':
        while(__sync_lock_test_and_set(&spin_lock, 1));
        add(&counter, 1);
        add(&counter, -1);
        break;
      case 'c':
        __sync_val_compare_and_swap(&counter, counter, counter+1);
        __sync_val_compare_and_swap(&counter, counter, counter-1);
        break;
    }
  }
  else
  {
    add(&counter, 1);
   // fprintf(stdout, "%lli\n", counter);
    add(&counter, -1);
  }
   if(protectionflags)
  {
    switch(syncvar)
    {
      case 'm':
        pthread_mutex_unlock(&mut);
        break;
      case 's':
        __sync_lock_release(&spin_lock);
        break;
    }
  }
}

int main(int argc, char* argv[])
{
  //Initialization of variables
  int numthreads = 1;
  int iterations = 1;
  struct timespec starttime, endtime;
  int opt = 0;
  int long_index = 0;
  fprintf(stdout, "%s", "add");
  //options structure
  static struct option long_options[]=
  {
    {"threads", required_argument, 0, 't'},
    {"iterations", required_argument, 0, 'i'},
    {"yield", no_argument, 0, 'y'},
    {"sync", required_argument, 0, 's'},
    {0, 0, 0, 0}
  };

  //struct for thread_start
  struct thread_info {
    pthread_t thread_id;
    int thread_num;
    char *argv_string;
  };

  struct arg_struct {
    long long* pointer;
    long long value;
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
      case 'y':
        opt_yield = 1;
        additionalflags = 1;
       // fprintf(stdout, "%s", "-yield");
        break;
      case 's':
        protectionflags = 1;
        char* typelock = optarg;
        switch(*typelock)
        {
          case 'm':
      //      fprintf(stdout, "%s,", "-m");
            syncvar = 'm';
            break;
          case 's':
       //     fprintf(stdout, "%s,", "-s");
            syncvar = 's';
            break;
          case 'c':
       //     fprintf(stdout, "%s,", "-c");
            syncvar = 'c';
            break;
          //TODO: make it add or add-yield depedning on flags, dynamically reallocate the string
        }
      break;
    }
  }
  if(opt_yield)
  {
  //   fprintf(stdout, "%s", "-none");
  // else
    fprintf(stdout, "%s", "-yield");
  }
  if(!protectionflags)
    fprintf(stdout, "%s", "-none");
  else
    fprintf(stdout, "-%c", syncvar);

  clock_gettime(CLOCK_MONOTONIC, &starttime);
//for loop to create all the threads
  pthread_t threadId[numthreads];
  struct arg_struct args;
  args.value = 1;
  args.pointer = &counter;
  for (int j = 0; j < iterations; j++)
  {
    for (int i = 0; i < numthreads; i++)
    {
      int rs = pthread_create(&(threadId[i]), NULL, newthread, NULL);
     if(rs != 0)
     {
        fprintf(stderr, "Error with pthread_create\n");
        exit(1);
      }
    }
   for (int i = 0; i < numthreads; i++)
   {
     int rs = pthread_join(threadId[i], NULL);
     if(rs != 0)
     {
        fprintf(stderr, "Error with pthread_join\n");
        exit(1);
      }
   }
  }
  clock_gettime(CLOCK_MONOTONIC, &endtime);
  long int elapsedntime = 1000000000L*(endtime.tv_sec - starttime.tv_sec) + (endtime.tv_nsec - starttime.tv_nsec);
  int numops = numthreads*iterations*2;
  double averagetime = (double)elapsedntime/(double)numops;
  // if(additionalflags || protectionflags)
  //   fprintf(stdout, "-none,%d,%d,%d,%li,%.0f,%lli\n", numthreads, iterations, numops, elapsedntime, averagetime, counter);
  // else
    fprintf(stdout, ",%d,%d,%d,%li,%.0f,%lli\n", numthreads, iterations, numops, elapsedntime, averagetime, counter);
  exit(0);
}
