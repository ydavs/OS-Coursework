/* 
    Allocation is done via first fit approach. 
*/

#include "alloc.h"

char* addr = NULL ; 
int blocks;
int check1;
int check2;
int smallest;
int smallestIdx;
int smallest2;
int smallest2Idx;
int tempIdx;

typedef struct MemoryList MemoryList;
struct MemoryList
{
  int occupied;
  int start;
  int idx;
};

MemoryList Memory[512];

int init_alloc(){
    addr = mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) ;
    if(addr == MAP_FAILED) return 1 ; 
    
    for(int i=0;i<512;i++){
        Memory[i].occupied = 0;
        Memory[i].start = 0;
        Memory[i].idx = i;
    }

    return 0;  
}

int cleanup()
{
    return munmap(addr, 4096) ; 
}

char* alloc(int bufferSize)
{
    if(bufferSize > 4096 || bufferSize%8 != 0) return NULL;

    blocks = bufferSize/8 ; 
    check1 = -1;
    

    for (int i=0; i<512; i++)
    {
        if (Memory[i].occupied == 0)
        {
            check2 = 0;
            tempIdx = i;
            for (int j=0; j<blocks; j++)
            {
                if(Memory[i+j].occupied == 1) check2=1;
            }
            if (check2 == 0)
            {
                check1 = tempIdx;
                printf("%d \n", tempIdx);
                break;
            }
        }
    }

    

    /* Is there a free chunk? */
    if(check1 == -1) return NULL ; 

    /* Update bit vector and size */
    Memory[check1].occupied = 1;
    Memory[check1].start = 1;
    for(int i=check1;i<check1+blocks;i++)
    {
        Memory[i].occupied = 1;
    }
    return addr+(8*check1) ; 
}

void dealloc(char *loc)
{
    int start = (loc - addr)/8 ; 
    Memory[start].start = 0;
    for(int i=start;i<512;i++)
    {
        if (Memory[i].occupied == 1 && Memory[i].start != 1) Memory[i].occupied = 0;
        if (Memory[i+1].start == 1) break;

    }

    return; 
}
