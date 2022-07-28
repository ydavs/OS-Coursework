#include "ealloc.h"

#define MPGS 4
#define BNUM 16

char** addr[MPGS] ; 
char* tmp[MPGS] ;  
int mvec[MPGS][BNUM], csize[MPGS][BNUM] ; 

void init_alloc()
{
    for(int i=0;i<MPGS;i++)
    {
        addr[i] = NULL, tmp[i] = NULL ;  
        for(int j=0;j<BNUM;j++) mvec[i][j]=-1,csize[i][j]=-1 ;
        mvec[i][0] = 0, csize[i][0] = 16 ; 
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
    printf("Alloted page address : %p and num %d\n", *addr[num], num) ; 
    if(*addr[num] == MAP_FAILED) return 1 ; 
    return 0;  
}

char *alloc(int size)
{
    if(size > 4096 || size % 256 !=0) return NULL ; 

    /* Look for free chunks from pg 0 to 3, if needed create a new page */
    int blocks = size/256, ind=-1, is_alloc = 0, pgno=-1 ; 

    for(int pg=0;pg<MPGS;pg++)
    {

        int got_page = 1 ; 
        if(addr[pg] == NULL) got_page = get_page(pg) ; 

        printf("Did we get a page %d\n", got_page) ; 
        ind = -1  ;
        for(int i=0;i<BNUM;i++)
        {
            if(csize[pg][i] >= blocks ){ ind = i ; break ; } 
        }
        if(ind == -1 ) continue;  
        if(ind == -1 && pg == 3) return NULL;

        //printf("Ind is %d\n", ind) ; 

        /* Find an empty place */
        if(csize[pg][ind] >= blocks)
        {
            int new_ind = -1 ; 
            for(int i=0;i<BNUM;i++)
            {
                if(csize[pg][i] == -1 && mvec[pg][i] == -1 ){ new_ind = i ; break ;}
            }
            if(new_ind == -1) is_alloc = 0 ;  
            //printf("New index is %d\n", new_ind) ; 

            mvec[pg][new_ind] = mvec[pg][ind]  + blocks ; 
            csize[pg][new_ind] = csize[pg][ind] - blocks ; 
            csize[pg][ind] = 0 ;
            is_alloc = 1, pgno = pg ;
        }
        printf("Alloc\n") ; 
        for(int i=0;i<5;i++)
        {
            printf("%d %d\n", mvec[pg][i], csize[pg][i] ) ; 
        } 
        for(int i=0;i<MPGS;i++)
        {
            if(addr[i] != NULL) printf("%p ", *addr[i]) ; 
        }
        printf("\n") ;  

        if(is_alloc) break ; 
    }
    //printf("truex\n") ;

    /* No free chunk available across all pages */
    if(pgno == -1 || ind == -1) return NULL ; 

    /* Update bit vector and size */    
    printf("pg no %d ind is %d %p %d %p\n",pgno, ind, *addr[pgno], (256*mvec[pgno][ind]), *addr[pgno]+(256*mvec[pgno][ind]) ) ;

    return *addr[pgno]+(256*mvec[pgno][ind]) ; 
}

void dealloc(char* loc)
{
    int starts[4] ; 

    for(int i=0;i<MPGS;i++){ starts[i] = (loc - *addr[i])/256 ;
    }
    int pgno = 0 ; 
    for(int i=0;i<MPGS;i++)
        if(starts[i] < 16 && starts[i] > -1) pgno = i ; 

    int ind = -1 ; 

    for(int i=0;i<BNUM;i++)
    {
        if(mvec[pgno][i] == starts[pgno]){ind = i ; break; }
    }

    int min_aft = 123123 ; 
    for(int i=0;i<BNUM;i++)
    {
        if(mvec[pgno][i] > mvec[pgno][ind])
        {
            if(min_aft > mvec[pgno][i]) min_aft = i ; 
        }
    }

    csize[pgno][ind] = mvec[pgno][min_aft] - mvec[pgno][ind] ; 

    /* Merge */
    if((csize[pgno][ind] + mvec[pgno][ind] == mvec[pgno][min_aft]) && csize[pgno][min_aft] > 0)
    {
        csize[pgno][ind] += csize[pgno][min_aft] ; 
        mvec[pgno][min_aft] =-1, csize[pgno][min_aft] =-1; 
    }

    int fp = -1 ; 
    for(int i=0;i<BNUM;i++)
    {
        if(mvec[pgno][i] + csize[pgno][i] == mvec[pgno][ind]) fp = i ; 
    }       

   if(fp > -1 && csize[pgno][fp] > 0)
    {
        csize[pgno][fp] += csize[pgno][ind] ;
        mvec[pgno][ind] =-1, csize[pgno][ind] = -1  ; 
    }
    for(int i=0;i<BNUM;i++)
    {
        if(mvec[pgno][i] == BNUM)
        {
            mvec[pgno][i] = -1, csize[pgno][i] =-1; 
            break ;
        }
    }

    printf("DeAlloc\n") ; 
    for(int i=0;i<5;i++)
    {
        printf("%d %d\n", mvec[pgno][i], csize[pgno][i] ) ; 
    } 

    return ; 
}
