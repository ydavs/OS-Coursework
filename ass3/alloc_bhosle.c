#include "alloc.h"

char* addr  = NULL ; 
int mvec[512], csize[512] ; 

int init_alloc()
{
    addr = mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) ;
    if(addr == MAP_FAILED) return 1 ; 
    
    for(int i=0;i<512;i++)
    {
        mvec[i]=-1,csize[i]=-1 ;
    }
    mvec[0] = 0, csize[0] = 512 ; 

    return 0;  
}

int cleanup()
{
    return munmap(addr, 4096) ; 
}


char* alloc(int size)
{
    if(size> 4096 || size%8!=0) return NULL  ; 
    int blocks = size/8 ; 

    int ind = - 1 ; 
    for(int i=0;i<512;i++)
    {
        if(csize[i] >= blocks ){ ind = i ; break ; } 
    }
    if(ind == -1 ) return NULL ; 

    /* Find an empty place */
    if(csize[ind] >= blocks)
    {
        int new_ind = -1 ; 
        for(int i=0;i<512;i++)
        {
            if(csize[i] == -1 && mvec[i] == -1 ){ new_ind = i ; break ;}
        }
        if(new_ind == -1) return NULL ; 

        mvec[new_ind] = mvec[ind]  + blocks ; 
        csize[new_ind] = csize[ind] - blocks ; 
    }

    csize[ind] = 0 ;

    return addr+(8*mvec[ind]) ; 
}

void dealloc(char* loc)
{
    int start = (loc-addr)/8 ; 
    int ind = -1 ; 

    for(int i=0;i<512;i++)
    {
        if(mvec[i] == start){ind =i ; break; }
    }

    int min_aft = 102301202 ;    
    for(int i=0;i<512;i++)
    {
        if(mvec[i] > mvec[ind])
        {
            if(min_aft > mvec[i]) min_aft = i ;   
        }
    }

    csize[ind] = mvec[min_aft] - mvec[ind] ; 

    /* Merge  */
    if((csize[ind] + mvec[ind] == mvec[min_aft]) && csize[min_aft]>0)
    {
        csize[ind] += csize[min_aft] ;  
        mvec[min_aft] =-1, csize[min_aft] =-1 ;
    }

    int fp = -1 ; 
    for(int i=0;i<512;i++)
    {
        if(mvec[i] + csize[i] == mvec[ind]) fp = i ; 
    }       

   if(fp > -1 && csize[fp] > 0)
    {
        csize[fp] += csize[ind] ;
        mvec[ind] =-1, csize[ind] = -1  ; 
    }

   return ;
}