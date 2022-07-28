/* 
    
    Allocation is done via first fit approach. 

 */
#include "ealloc.h"

#define MPGS 4
#define BNUM 16

char** addr[MPGS] ; 
char* tmp[MPGS] ;  
int bvec[MPGS][BNUM], csize[MPGS][BNUM] ; 

void init_alloc()
{
    for(int i=0;i<MPGS;i++)
    {
        addr[i] = NULL, tmp[i] = NULL ;  
        for(int j=0;j<BNUM;j++) bvec[i][j]=0,csize[i][j]=0 ;
    }
    return ; 
}

void cleanup()
{
    return ;    
}

int get_page(int num)
{
    tmp[num] = mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) ;
    addr[num] = &tmp[num] ;  
    if(*addr[num] == MAP_FAILED) return 1 ; 
    return 0;  
}

char *alloc(int size)
{
    if(size > 4096 || size % 256 !=0) return NULL ; 

    /* Look for free chunks from pg 0 to 3, if needed create a new page */
    int blocks = size/256, start=-1, is_alloc = 0, pgno=-1 ; 
    for(int pg=0;pg<MPGS;pg++)
    {
        int got_page = 1 ; 
        if(addr[pg] == NULL) got_page = get_page(pg) ; 

        for(int i=0;i<BNUM;i++)
        {
            int tst = 0 ; 
            for(int j=0;j<blocks;j++)
                if(bvec[pg][j+i] != 0) tst = 1 ; 

            if(!tst)
            {
                start = i, is_alloc =1, pgno = pg ; 
                break ; 
            }
        }
        if(is_alloc) break ; 
    }

    /* No free chunk available across all pages */
    if(pgno == -1 || start == -1) return NULL ; 

    /* Update bit vector and size */    
    for(int i=start;i<start+blocks;i++) bvec[pgno][i] = 1 ; 
    csize[pgno][start] = blocks ; 

    return *addr[pgno]+(256*start) ; 
}

void dealloc(char* loc)
{
    int starts[4] ; 

    for(int i=0;i<MPGS;i++){ starts[i] = (loc - *addr[i])/256 ;
    }
    int pgno = 0 ; 
    for(int i=0;i<MPGS;i++)
        if(starts[i] < 16 && starts[i] > -1) pgno = i ; 

    
    for(int i=starts[pgno]; i<starts[pgno]+csize[pgno][starts[pgno]];i++) bvec[pgno][i] = 0 ; 
    csize[pgno][starts[pgno]] = 0 ; 

    return ; 
}
