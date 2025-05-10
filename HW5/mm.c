/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
 /* ---------- constants ---------- */
 #define WSIZE 8
 #define DSIZE 16
 #define CHUNKSIZE (1<<12)
 #define ALIGNMENT 8
 #define ALIGN(x) (((x)+(ALIGNMENT-1)) & ~0x7)
 
 #define PACK(sz,a) ((sz)|(a))
 #define GET(p)     (*(size_t *)(p))
 #define PUT(p,v)   (*(size_t *)(p) = (v))
 #define GET_SIZE(p) (GET(p) & ~0x7)
 #define GET_ALLOC(p) (GET(p) & 0x1)
 
 #define HDRP(bp)  ((char *)(bp)-WSIZE)
 #define FTRP(bp)  ((char *)(bp)+GET_SIZE(HDRP(bp))-DSIZE)
 #define NEXT_BLKP(bp) ((char *)(bp)+GET_SIZE(((char *)(bp)-WSIZE)))
 #define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE(((char *)(bp)-DSIZE)))
 
 /* free-list payload */
 #define NEXT_FREE(bp) (*(void **)(bp))
 #define PREV_FREE(bp) (*((void **)(bp)+1))
 
 /* ---------- 20 scalar list heads ---------- */
 static void *h00,*h01,*h02,*h03,*h04,*h05,*h06,*h07,*h08,*h09,
             *h10,*h11,*h12,*h13,*h14,*h15,*h16,*h17,*h18,*h19;
 #define H(i) ((i)==0?&h00:(i)==1?&h01:(i)==2?&h02:(i)==3?&h03:(i)==4?&h04:\
               (i)==5?&h05:(i)==6?&h06:(i)==7?&h07:(i)==8?&h08:(i)==9?&h09:\
               (i)==10?&h10:(i)==11?&h11:(i)==12?&h12:(i)==13?&h13:(i)==14?&h14:\
               (i)==15?&h15:(i)==16?&h16:(i)==17?&h17:(i)==18?&h18:&h19)
 
 static char *heap_listp;
 
 /* ---------- internal protos ---------- */
 static int  idx_from_size(size_t);
 static void insert_free(void *);
 static void remove_free(void *);
 static void *extend_heap(size_t);
 static void *coalesce(void *);
 static void  split(void *,size_t,size_t);
 static void *best_fit(int,size_t);



/* 
 * mm_init - initialize the malloc package.
 */
 int mm_init(void)
 {
     if((heap_listp=mem_sbrk(4*WSIZE))==(void *)-1) return -1;
     PUT(heap_listp,0);
     PUT(heap_listp+WSIZE,PACK(DSIZE,1));
     PUT(heap_listp+2*WSIZE,PACK(DSIZE,1));
     PUT(heap_listp+3*WSIZE,PACK(0,1));
     heap_listp+=2*WSIZE;
     h00=h01=h02=h03=h04=h05=h06=h07=h08=h09=
     h10=h11=h12=h13=h14=h15=h16=h17=h18=h19=NULL;
     return extend_heap(CHUNKSIZE/WSIZE)?0:-1;
 }
 


/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
 {
     if(size==0) return NULL;
     size_t asz = (size<=DSIZE)?2*DSIZE:ALIGN(size+DSIZE);
     for(int i=idx_from_size(asz); i<20; ++i){
         void *bp=best_fit(i,asz);
         if(bp){
             size_t csz=GET_SIZE(HDRP(bp));
             remove_free(bp);
             if(csz-asz>=64) split(bp,asz,csz);
             else { PUT(HDRP(bp),PACK(csz,1)); PUT(FTRP(bp),PACK(csz,1)); }
             return bp;
         }
     }
     size_t ext=(asz>CHUNKSIZE)?asz:CHUNKSIZE;
     void *bp=extend_heap(ext/WSIZE);
     if(!bp) return NULL;
     remove_free(bp);
     size_t csz=GET_SIZE(HDRP(bp));
     if(csz-asz>=64) split(bp,asz,csz);
     else { PUT(HDRP(bp),PACK(csz,1)); PUT(FTRP(bp),PACK(csz,1)); }
     return bp;
 }
 

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
    if(!bp) return;
    size_t sz=GET_SIZE(HDRP(bp));
    PUT(HDRP(bp),PACK(sz,0));
    PUT(FTRP(bp),PACK(sz,0));
    coalesce(bp);
}


/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr,size_t size)
{
    if(ptr==NULL) return mm_malloc(size);
    if(size==0){ mm_free(ptr); return NULL; }

    size_t asz=(size<=DSIZE)?2*DSIZE:ALIGN(size+DSIZE);
    size_t old=GET_SIZE(HDRP(ptr));

    if(asz<=old){ if(old-asz>=64) split(ptr,asz,old); return ptr; }

    void *next=NEXT_BLKP(ptr);
    if(!GET_ALLOC(HDRP(next))){
        size_t comb=old+GET_SIZE(HDRP(next));
        if(comb>=asz){
            remove_free(next);
            PUT(HDRP(ptr),PACK(comb,1));
            PUT(FTRP(ptr),PACK(comb,1));
            return ptr;
        }
    }

    void *newp=mm_malloc(size);
    if(!newp) return NULL;
    memcpy(newp,ptr,old-DSIZE<size?size:old-DSIZE);
    mm_free(ptr);
    return newp;
}

/* ---------------- helpers ---------------- */
static int idx_from_size(size_t s){
    int idx=0; size_t cap=32;
    while(idx<19 && s>cap){ cap<<=1; idx++; }
    return idx;
}

static void insert_free(void *bp)
{
    int i=idx_from_size(GET_SIZE(HDRP(bp)));
    void **head=H(i);
    if(!*head){ NEXT_FREE(bp)=PREV_FREE(bp)=bp; *head=bp; return; }
    NEXT_FREE(bp)=*head;
    PREV_FREE(bp)=PREV_FREE(*head);
    NEXT_FREE(PREV_FREE(*head))=bp;
    PREV_FREE(*head)=bp;
    *head=bp;
}

static void remove_free(void *bp)
{
    int i=idx_from_size(GET_SIZE(HDRP(bp)));
    void **head=H(i);
    if(NEXT_FREE(bp)==bp)*head=NULL;
    else{
        NEXT_FREE(PREV_FREE(bp))=NEXT_FREE(bp);
        PREV_FREE(NEXT_FREE(bp))=PREV_FREE(bp);
        if(*head==bp)*head=NEXT_FREE(bp);
    }
}

static void *extend_heap(size_t words)
{
    size_t sz=(words&1)?(words+1)*WSIZE:words*WSIZE;
    char *bp=mem_sbrk(sz);
    if(bp==(void *)-1) return NULL;
    PUT(HDRP(bp),PACK(sz,0));
    PUT(FTRP(bp),PACK(sz,0));
    PUT(HDRP(NEXT_BLKP(bp)),PACK(0,1));
    return coalesce(bp);
}

static void split(void *bp,size_t need,size_t csz)
{
    PUT(HDRP(bp),PACK(need,1));
    PUT(FTRP(bp),PACK(need,1));
    void *tail=NEXT_BLKP(bp);
    PUT(HDRP(tail),PACK(csz-need,0));
    PUT(FTRP(tail),PACK(csz-need,0));
    insert_free(tail);
}

static void *coalesce(void *bp)
{
    size_t size=GET_SIZE(HDRP(bp));
    int prev_alloc=GET_ALLOC(HDRP(PREV_BLKP(bp)));
    int next_alloc=GET_ALLOC(HDRP(NEXT_BLKP(bp)));

    if(!prev_alloc){
        void *prev=PREV_BLKP(bp);
        remove_free(prev);
        size+=GET_SIZE(HDRP(prev));
        PUT(HDRP(prev),PACK(size,0));
        PUT(FTRP(prev),PACK(size,0));
        bp=prev;
    }
    if(!next_alloc){
        void *next=NEXT_BLKP(bp);
        remove_free(next);
        size+=GET_SIZE(HDRP(next));
        PUT(HDRP(bp),PACK(size,0));
        PUT(FTRP(bp),PACK(size,0));
    }
    insert_free(bp);
    return bp;
}

static void *best_fit(int idx,size_t need)
{
    void **head=H(idx);
    if(!*head) return NULL;
    void *best=NULL,*bp=*head;
    do{
        size_t sz=GET_SIZE(HDRP(bp));
        if(sz>=need && (!best || sz<GET_SIZE(HDRP(best)))) best=bp;
        bp=NEXT_FREE(bp);
    }while(bp!=*head && best!=need);   /* perfect fit early exit */
    return best;
}














