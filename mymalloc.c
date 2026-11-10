#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#include <malloc.h>
#include <stdio.h>

// Include any other headers we need here
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

// NOTE: You should NOT include <stdlib.h> in your final implementation.
// malloc.h turns every call to malloc/calloc/free in this file into a call
// to mymalloc/mycalloc/myfree, so calling them here would recurse forever.

#include <debug.h> // definition of debug_printf

//assuptions: all of the blocks are kept in sbrk order, alloaction happens via first-fit, memory isn't returned to the Operating System, no splits

typedef struct block {
  size_t size;
  struct block *next; //next block in sbrk order - in line w assumptions
  int free;
} block_t;

static block_t *head = NULL;
static block_t *tail = NULL;

static block_t *request_block(size_t s) {
  assert(s > 0);

  size_t total_size = sizeof(block_t) + s;
  void *req = sbrk(total_size);
  if (req == (void*)-1) {
    return NULL;
  }

  block_t *new_block = (block_t *)req;
  new_block->size = s;
  new_block->free = 0;
  new_block->next = NULL;

  if (head == NULL) {
    head = new_block;
    tail = new_block;
  } else {
    tail->next = new_block;
    tail = new_block;
    }

  return new_block;
}

//find the first free block that fits, or request a new one with sbrk
static block_t *find_free_block(size_t s) {
  assert(s > 0);

  block_t *curr = head;
  while (curr != NULL) {
    if (curr->free && curr->size >= s) {
      return curr;
    }
    curr = curr->next;
  }
  return NULL;
}

void *mymalloc(size_t s) {
  if (s == 0) {
    return NULL;
  }

  // find the first free block that fits, or request a new one with sbrk
  block_t *block = find_free_block(s);
  if (block != NULL) {
    block->free = 0;
  } else {
    block = request_block(s);
    if (block == NULL) {
      // we are out of memory
      // if sbrk fails (returns (void *) -1)
      return NULL;
    }
  }

  assert(!block->free && block->size >= s);
  debug_printf("Malloc %zu bytes\n", s);
  return (void *)(block + 1);
}

void *mycalloc(size_t nmemb, size_t s) {

  if (nmemb != 0 && s > SIZE_MAX / nmemb) {
    return NULL;
  }
  size_t total_size = nmemb * s;

  void *p = mymalloc(total_size);
  if(p == NULL) {
    // We are out of memory
    // if we get NULL back from mymalloc
    return NULL;
  }

  debug_printf("Calloc %zu bytes\n", total_size);
  memset(p, 0, total_size);
  return p;
}

void myfree(void *ptr) {
  if (ptr == NULL) {
    return;
  }
  //the size stored in ptr's block header
  block_t *block = (block_t *)ptr -1;
  size_t size = block->size;

 //mark ptr's block as free, block stays in list for reuse (not returned to OS, according to assumptions)
  block->free = 1;
  debug_printf("Freed %zu bytes\n", size);
}
