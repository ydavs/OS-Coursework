/* 
    Allocation is done via first fit approach. 
*/

#include "alloc.h"

char* addr = NULL ; 
int bvec[512], csize[512] ; 

int init_alloc()
{
    addr = mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) ;
    if(addr == MAP_FAILED) return 1 ; 
    
    for(int i=0;i<512;i++)
    {
        bvec[i]=0,csize[i]=0 ;
    }

    return 0;  
}

int cleanup()
{
    return munmap(addr, 4096) ; 
}

char* alloc(int size)
{
    if(size > 4096 || size%8 != 0) return NULL ;
    int blocks = size/8 ; 
    int start = -1 ; 
    
    /* Look for a free chunk */
    for(int i=0;i<512;i++)
    {
        if(bvec[i] == 0)
        {
            int tst = 0 ; 
            for(int j=0;j<blocks;j++)
                if(bvec[j+i]!= 0 ) tst = 1 ; 
            
            if(!tst){ 
                start = i ;
                break ; 
            } 
        }  
    }

    /* Is there a free chunk? */
    if(start == -1) return NULL ; 

    /* Update bit vector and size */
    for(int i=start;i<start+blocks;i++) bvec[i] = 1 ; 
    csize[start] = blocks ;

    return addr+(8*start) ; 
}

void dealloc(char *loc)
{
    int start = (loc - addr)/8 ; 
    for(int i=start;i<start+csize[start];i++) bvec[i] = 0 ; 
    csize[start] = 0 ; 

    return; 
}
