#include "SortedList.h"
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void SortedList_insert(SortedList_t *list, SortedListElement_t *element)
{
    SortedListElement_t *prev = list;
    SortedListElement_t *curr = prev -> next;
   //fprintf(stdout, "Element ptr in sorted list %p\n", element);
    while (curr != NULL)
    {
        if(strcmp(element->key, curr->key) <= 0)
            break;
        prev = curr;
        curr = prev -> next;
    }

    if(opt_yield & INSERT_YIELD)
        sched_yield();

    element->prev = prev;
    element->next = curr;
    prev -> next = element;
    if(curr != NULL)
     curr -> prev = element;

}

int SortedList_delete(SortedListElement_t *element)
{
    SortedListElement_t *prev = element -> prev;
    SortedListElement_t *next = element -> next;
  
   //fprintf(stderr, "Deleted element %s\n", element -> key);
    if(opt_yield & DELETE_YIELD)
        sched_yield();
    if((prev == NULL && next == NULL) || element == NULL)
    {
        //free(element);
        return 0;
    }
    else if(prev == NULL && next->prev == element)
    {
      next -> prev = NULL;
      return 0;
    }
    else if(next == NULL && prev->next == element)
    {
      prev -> next = NULL;
      return 0;
    }
    else if(prev->next == element && next -> prev == element)
    {
        prev -> next = next;
        next -> prev = prev;
        return 0;
    }
    else
    {
        fprintf(stderr, "%s\n", "Corrupted pointers!");
        return 1;
    }
    
    //free(element);
  
    
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key)
{
    SortedListElement_t *search = list -> next;
    if(opt_yield & LOOKUP_YIELD)
        sched_yield();
    while(search != NULL)
    {
        if(strcmp(key, search -> key)==0)
        {
            return search;
        }
        search = search -> next;
    }
    return NULL;
}

int SortedList_length(SortedList_t *list)
{
    int count = 0;
    if (list == NULL)
        return count;
    if(opt_yield & LOOKUP_YIELD)
        sched_yield();
    if (list -> next == NULL)
        return count;
    SortedListElement_t *prev = list -> next;
    SortedListElement_t *curr = prev -> next;
    if(curr == NULL)
        return 1;
    count++;
    while(curr != NULL)
    {
       
        if(curr-> prev != prev && prev->next != curr)
        {
             fprintf(stderr, "Adress pointer curr -> prev: %p\n", curr -> prev);
             fprintf(stderr, "Adress pointer prev -> next: %p\n", prev -> next);
            fprintf(stderr, "%s\n", "Corrupted pointer in length function!");
           // return -1;
        }
        prev = curr;
        curr = curr -> next;
        count++;
    }
    return count;
}