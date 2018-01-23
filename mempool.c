/* mempool.c -- Memory pool with fixed blocksize, 16 byte alligned.
 *
 *              Uses system allocation (malloc) when the pool is full or
 *              data size is larger than a block.
 */

#include <stdlib.h>
#include <stdio.h>

#include "mempool.h"


typedef unsigned char uchar;
typedef unsigned int uint;

typedef struct memPool memPool;

struct memPool {
  uchar*    next;
  uchar*    memstart;
  uint      maxblocks;
  uint      initialized;    /* blocks */
  uint      freeblocks;
  uint      blocksize;      /* in uchar */
};


static memPool* mp = 0;


void mp_create(size_t size_of_block, size_t nr_of_blocks) {

  /*** Allign to 16 byte multiples ***/
  size_of_block =   size_of_block + 15 & ~0x0F;
  size_t mempoolsize = sizeof(memPool) + 15 & ~0x0F;

  mp = malloc(nr_of_blocks * size_of_block + mempoolsize);

  mp->memstart      = (uchar*)mp + mempoolsize;
  mp->next          = mp->memstart;
  mp->blocksize     = size_of_block;
  mp->maxblocks     = nr_of_blocks;
  mp->freeblocks    = nr_of_blocks;
  mp->initialized   = 0;
}


static uchar* AddressFromIndex(const uint index) {
  return mp->memstart + index * mp->blocksize;
}


static uint IndexFromAddress(const uchar* p) {
  return (((uint)(p - mp->memstart)) / mp->blocksize);
}


void* mp_alloc(void) {
  /*** Returns one free block from memPool ***/
  
  /*** Fill up the memory pool to the end first ***/
  if (mp->initialized < mp->maxblocks) {

    uint* p = (uint*)AddressFromIndex(mp->initialized);

    /*** Write index of next block in this new block ***/
    *p = ++mp->initialized;;
  }

  void* ret = 0;

  if (mp->freeblocks > 0) {
    ret = (void*)mp->next;
    --mp->freeblocks;
    if (mp->freeblocks != 0) {
      mp->next = AddressFromIndex( *((uint*)mp->next) );
    } else {
      mp->next = 0;
    }
  }

  return ret;
}


void mp_free(void* block) {
  /*** Frees one block of memory ***/

  if (mp->next != 0) {

    /*** Write index from next free block in this freed block ***/
    (*(uint*)block) = IndexFromAddress(mp->next);
    mp->next = (uchar*)block;
  } else {

    /*** Pool was full, write max_blocks in this freed block ***/
    *((uint*)block) = mp->maxblocks;
    mp->next = (uchar*)block;
  }

  ++mp->freeblocks;
}


#ifdef MAIN

#include <time.h>


int main(void) {

  const size_t maxsize = 1000000;
  long msec;

  struct s {
    int a;
    int b;
  };


  /*** Time mempool ***/

  clock_t start = clock(), diff;

  mp_create(sizeof(struct s), maxsize);

  struct s* data[maxsize];
  for (size_t i = 0; i < maxsize; ++i) {
    data[i] = mp_alloc();
    if (!data[i]) {
      printf("Out of memory at i = %d\n", i);
    }
  }

  diff = clock() - start;
  msec = diff * 1000 / CLOCKS_PER_SEC;
  printf("Mempool %d. Time in msec: %d\n", maxsize, msec);


  /*** Time malloc ***/

  start = clock();
  struct s* data2;
  for (size_t i = 0; i < maxsize; ++i) {
    data2 = malloc(sizeof(struct s));
    if (!data2) {
      printf("malloc memory error at %d\n", i);
    }
  }

  diff = clock() - start;
  msec = diff * 1000 / CLOCKS_PER_SEC;
  printf("Malloc %d. Time in msec: %d\n", maxsize, msec);

  return 0;
}

#endif
