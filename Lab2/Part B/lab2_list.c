#include "SortedList.c"
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>

//initialization of global variables
int numthreads = 1;
int iterations = 1;
int sublists = 1;
int syncmflag = 0;
int syncsflag = 0;
int arrtracker = 0;
const int keylen = 20;
SortedListElement_t* elementarr;
SortedList_t* list;
pthread_mutex_t mut;
volatile int spin_lock = 0;
SortedList_t* sublistarr;
pthread_mutex_t* sublistmuts;
volatile int* subspinlocks;
int numopsyield = 0;
int opt_yield = 0;
long int totallocktime =0;

//TODO: print out none if options aren't selected

int keyhash(const char* string)
{
    char key = string[2];
    return key%sublists;
}

int sublistlength()
{
    int len = 0;
    for(int i = 0; i < sublists; i++)
    {
        if(syncmflag)
        {
            pthread_mutex_lock(&(sublistmuts[i]));
            len += SortedList_length(&(sublistarr[i]));
            pthread_mutex_unlock(&(sublistmuts[i]));
        }
        else if(syncsflag)
        {
            while(__sync_lock_test_and_set(&(subspinlocks[i]), 1));
             len += SortedList_length(&(sublistarr[i]));
            __sync_lock_release(&(subspinlocks[i]));
        }
        else
             len += SortedList_length(&(sublistarr[i]));
    }
}

void* newthreadsublists(void *arrset)
{
    struct timespec lockbegin, lockend;
    int a = *((int*)arrset);
    for (int i = a*iterations; i < ((a+1)*iterations); i++)
    {
        SortedListElement_t* elementptr = &elementarr[i];
       // int listtochoose = atoi((elementptr -> key))%sublists;
        int listtochoose = keyhash(elementptr -> key);
    // fprintf(stderr, "listtochoose: %d\n", listtochoose);
        if(syncmflag)
        {
            clock_gettime(CLOCK_MONOTONIC, &lockbegin);
            pthread_mutex_lock(&(sublistmuts[listtochoose]));
            clock_gettime(CLOCK_MONOTONIC, &lockend);
            long int elapsedntime = 1000000000L*(lockend.tv_sec - lockbegin.tv_sec) + (lockend.tv_nsec - lockbegin.tv_nsec);
            totallocktime += elapsedntime;
         }
        if(syncsflag)
            while(__sync_lock_test_and_set(&(subspinlocks[listtochoose]), 1));
        SortedList_insert(&(sublistarr[listtochoose]), elementptr);
        if(syncmflag)
            pthread_mutex_unlock(&(sublistmuts[listtochoose]));
        if(syncsflag)
            __sync_lock_release(&(subspinlocks[listtochoose]));

    }
    int listlength = sublistlength();
    for (int i = a*iterations; i < ((a+1)*iterations); i++)
    {
        //int listtochoose = (atoi((elementarr[i]).key))%sublists;
        int listtochoose = keyhash(elementarr[i].key);
        const char* key = elementarr[i].key;
        SortedListElement_t* todelete;
        if(syncmflag)
        {
            clock_gettime(CLOCK_MONOTONIC, &lockbegin);
            pthread_mutex_lock(&(sublistmuts[listtochoose]));
            clock_gettime(CLOCK_MONOTONIC, &lockend);
            long int elapsedntime = 1000000000L*(lockend.tv_sec - lockbegin.tv_sec) + (lockend.tv_nsec - lockbegin.tv_nsec);
            totallocktime += elapsedntime;
         }
        if(syncsflag)
            while(__sync_lock_test_and_set(&(subspinlocks[listtochoose]), 1));
        todelete = SortedList_lookup(&(sublistarr[listtochoose]), key);
        if(syncmflag)
            pthread_mutex_unlock(&(sublistmuts[listtochoose]));
        if(syncsflag)
            __sync_lock_release(&(subspinlocks[listtochoose]));
        
        //delete element
        if(syncmflag)
        {
            clock_gettime(CLOCK_MONOTONIC, &lockbegin);
            pthread_mutex_lock(&(sublistmuts[listtochoose]));
            clock_gettime(CLOCK_MONOTONIC, &lockend);
            long int elapsedntime = 1000000000L*(lockend.tv_sec - lockbegin.tv_sec) + (lockend.tv_nsec - lockbegin.tv_nsec);
            totallocktime += elapsedntime;
         }
        if(syncsflag)
            while(__sync_lock_test_and_set(&(subspinlocks[listtochoose]), 1));
        if(SortedList_delete(todelete))
        {
            if(syncmflag)
                pthread_mutex_unlock(&(sublistmuts[listtochoose]));
            if(syncsflag)
                 __sync_lock_release(&(subspinlocks[listtochoose]));
            exit(1);
        }
        if(syncmflag)
            pthread_mutex_unlock(&(sublistmuts[listtochoose]));
        if(syncsflag)
            __sync_lock_release(&(subspinlocks[listtochoose]));
        
    }
}

void* newthread(void *arrset)
{
    int* setnum = (int*)arrset;
    struct timespec lockbegin, lockend;
   // fprintf(stdout, "Which iteration: %d\n", *setnum);
    
    for (int i = (*setnum)*iterations; i < ((*setnum)+1)*(iterations); i++)
    {
        //SortedListElement_t* elementtoadd = elementarr[i];
        SortedListElement_t* elementptr = &elementarr[i];
        if(syncmflag)
        {
            clock_gettime(CLOCK_MONOTONIC, &lockbegin);
            pthread_mutex_lock(&mut);
            clock_gettime(CLOCK_MONOTONIC, &lockend);
            long int elapsedntime = 1000000000L*(lockend.tv_sec - lockbegin.tv_sec) + (lockend.tv_nsec - lockbegin.tv_nsec);
            totallocktime += elapsedntime;
         }
        if(syncsflag)
            while(__sync_lock_test_and_set(&spin_lock, 1));
       // fprintf(stdout, "Element ptr %p\n", elementptr);
        SortedList_insert(list, elementptr);
        if(syncmflag)
            pthread_mutex_unlock(&mut);
        if(syncsflag)
            __sync_lock_release(&spin_lock);
    }
    int listlength;
    listlength = SortedList_length(list);
    if(listlength == 0)
    {
        fprintf(stderr, "No list elements inserted! \n");
        exit(1);
    }
    for (int i = (*setnum)*iterations; i < ((*setnum)+1)*(iterations); i++)
    {
      //  SortedListElement_t* elementtoremove = elementarr[i];
       // fprintf(stdout, "Deleting this element: %d\n", i);
       //look up before deleting
       const char* key = elementarr[i].key;
       if(syncmflag)
        {
            clock_gettime(CLOCK_MONOTONIC, &lockbegin);
            pthread_mutex_lock(&mut);
            clock_gettime(CLOCK_MONOTONIC, &lockend);
            long int elapsedntime = 1000000000L*(lockend.tv_sec - lockbegin.tv_sec) + (lockend.tv_nsec - lockbegin.tv_nsec);
            totallocktime += elapsedntime;
         }
         if(syncsflag)
             while(__sync_lock_test_and_set(&spin_lock, 1));
       SortedList_lookup(list, key);
     if(syncmflag)
        pthread_mutex_unlock(&mut);
    if(syncsflag)
        __sync_lock_release(&spin_lock);
    
        if(syncmflag)
        {
            clock_gettime(CLOCK_MONOTONIC, &lockbegin);
            pthread_mutex_lock(&mut);
            clock_gettime(CLOCK_MONOTONIC, &lockend);
            long int elapsedntime = 1000000000L*(lockend.tv_sec - lockbegin.tv_sec) + (lockend.tv_nsec - lockbegin.tv_nsec);
            totallocktime += elapsedntime;
         }
         if(syncsflag)
             while(__sync_lock_test_and_set(&spin_lock, 1));
        
        if(SortedList_delete(&elementarr[i]))
        {
            if(syncmflag)
                pthread_mutex_unlock(&mut);
            if(syncsflag)
                 __sync_lock_release(&spin_lock);
            exit(1);
        }
            if(syncmflag)
        pthread_mutex_unlock(&mut);
    if(syncsflag)
        __sync_lock_release(&spin_lock);
    }


    // if(listlength)
    // {
    //     fprintf(stderr, "Ending list length is not zero! \n");
    //     exit(1);
    // }
}

void read_yield_args(char* optarg)
{
   // fprintf(stdout, "%c", '-');
   // fprintf(stdout, "Legnth of yield: %d", strlen(optarg));
   // fprintf(stdout, "First character: %c", optarg[0]);
   // fprintf(stdout, "%c", '-');
    for(int i = 0; i < strlen(optarg); i++)
    {
     //   fprintf(stdout, "%c", optarg[i]);
        switch(optarg[i])
        {
            case 'i':
                opt_yield |= INSERT_YIELD;
                //yieldiflag = 1;
               // fprintf(stdout, "%c", 'i');
                numopsyield++;
                break;
            case 'd':
                 opt_yield |= DELETE_YIELD;
                //yielddflag = 1;
              //  fprintf(stdout, "%c", 'd');
                 numopsyield++;
                break;
            case 'l':
                opt_yield |= LOOKUP_YIELD;
               // yieldlflag = 1;
              //  fprintf(stdout, "%c", 'l');
                 numopsyield++;
                break;
        }
    }
}
//TODO: malloc the length of string
void generate_key(int length)
{
    static const char alphanum[]=
    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    for(int j = 0; j< length; j++)
    {
        char* newkey = malloc(sizeof(char)*(keylen));
        for(int i=0; i<keylen; i++)
        {
             newkey[i] = alphanum[rand() % (sizeof(alphanum)-1)];
        }
        newkey[keylen] = 0;
        elementarr[j].key = newkey;
    }
    
}

int main(int argc, char* argv[])
{
    struct timespec starttime, endtime;
    static struct option long_options[]=
    {
        {"threads", required_argument, 0, 't'},
        {"iterations", required_argument, 0, 'i'},
        {"yield", required_argument, 0, 'y'},
        {"sync", required_argument, 0, 's'},
        {"lists", required_argument, 0, 'l'},
        {0, 0, 0, 0}
    };

    int opt = 0;
    int long_index = 0;
    int listflag = 0;
    char* yieldtype;
    char* synctype;
    list = malloc(sizeof(SortedList_t));
    
    while((opt = getopt_long(argc, argv, "sci:o:", long_options, &long_index))!=-1)
    {
        switch(opt)
        {
            case 't':
                numthreads = atoi(optarg);
                break;
            case 'i':
                iterations = atoi(optarg);
                break;
            case 'y':
                yieldtype = optarg;
                read_yield_args(yieldtype);
                break;
            case 's':
                //fprintf(stdout, "%c", '-');
                synctype = optarg;
                switch(*synctype)
                {
                    case 'm':
                        syncmflag = 1; //change, this should set opt_yield
                        pthread_mutex_init(&mut,NULL);
                        break;
                    case 's':
                        syncsflag = 1;
                        break;
                }
                break;
            case 'l':
                sublists = atoi(optarg);
                listflag = 1;
                break;
        }
    }
    if(!listflag)
     fprintf(stdout, "%s", "list");
    else
     fprintf(stdout, "%s", "List");
    if(!opt_yield)
        fprintf(stdout, "%s", "-none");
    else
    {
        fprintf(stdout, "%c", '-');
        if(opt_yield & INSERT_YIELD)
            fprintf(stdout, "%c", 'i');
        if(opt_yield & LOOKUP_YIELD)
            fprintf(stdout, "%c", 'l');
        if(opt_yield & DELETE_YIELD)
            fprintf(stdout, "%c", 'd');
    }
    if(!syncmflag && !syncsflag)
        fprintf(stdout, "%s", "-none");
    else if(syncmflag)
        fprintf(stdout, "%s", "-m");
     else if(syncsflag)
        fprintf(stdout, "%s", "-s");

    //initializing keys based on the number of listlength
    long int listlength = iterations*numthreads;
    elementarr = malloc(sizeof(SortedListElement_t)*listlength);
    generate_key(listlength);
    clock_gettime(CLOCK_MONOTONIC, &starttime);
    pthread_t threadId[numthreads];
   // for (int j = 0; j < iterations; j++)
    //{
    if(sublists == 1)
    {
        for (int i = 0; i < numthreads; i++)
        {
            int *arg = malloc(sizeof(*arg));
            if(arg == NULL)
            {
                fprintf(stderr, "Error with using malloc to pthread_create.\n");
                exit(1);
            }
            *arg = i;
            int rs = pthread_create(&(threadId[i]), NULL, newthread, arg);
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
    int listlengthreturned = SortedList_length(list);
    }
    else
    {
        sublistarr = malloc(sublists*sizeof(SortedList_t));
        sublistmuts = malloc(sublists*sizeof(pthread_mutex_t));
        subspinlocks = malloc(sublists*sizeof(volatile int));
        for(int i = 0; i< numthreads; i++)
        {
            int *arg = malloc(sizeof(*arg));
            if(arg == NULL)
            {
                fprintf(stderr, "Error with using malloc to pthread_create.\n");
                exit(1);
            }
            *arg = i;
            int rs = pthread_create(&(threadId[i]), NULL, newthreadsublists, arg);
            if(rs != 0)
            {
                fprintf(stderr, "Error with pthread_create\n");
                exit(1);
            }
        }
        for (int i = 0; i< numthreads; i++)
        {
             int rs = pthread_join(threadId[i], NULL);
             if(rs != 0)
            {
                fprintf(stderr, "Error with pthread_join\n");
                 exit(1);
            }
        }
    }
   // fprintf(stderr, "List length: %d\n", listlength);
 // }

    //important code here
    //double for loops with the iterations and creating threads and all that jazz

    clock_gettime(CLOCK_MONOTONIC, &endtime);
    long int elapsedntime = 1000000000L*(endtime.tv_sec - starttime.tv_sec) + (endtime.tv_nsec - starttime.tv_nsec);
    int numops; 
  //  if (numopsyield)
    numops = numthreads*iterations*(3);
   // else
    //    numops = numthreads*iterations;
    double averagetime = (double)elapsedntime/(double)numops;
    double locktime = (double)totallocktime/(double)numops;
    fprintf(stdout, ",%d,%d,%d,%d,%li,%.0f,%.0f\n", numthreads, iterations, sublists, numops, elapsedntime, averagetime, locktime);
    exit(0);
        //TODO: print comma to the begnning to separate title
}