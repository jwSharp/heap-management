#include <stdio.h>
#include <stdlib.h>

#include "memlib.h"
#include "mm.h"

/** Get more heap space of exact size reqSize. */
void *requestMoreSpace(size_t reqSize)
{
    void *ret = UNSCALED_POINTER_ADD(mem_heap_lo(), heap_size);
    heap_size += reqSize;

    void *mem_sbrk_result = mem_sbrk(reqSize);
    if ((size_t)mem_sbrk_result == -1)
    {
        printf("ERROR: mem_sbrk failed in requestMoreSpace\n");
        exit(0);
    }

    return ret;
}

/** Initialize the allocator. */
int mm_init()
{
    free_list_head = NULL;
    malloc_list_tail = NULL;
    heap_size = 0;

    return 0;
}

/** Gets the first block in the heap or returns NULL if there is not one. */
Block *first_block()
{
    Block *first = (Block *)mem_heap_lo();
    if (heap_size == 0)
    {
        return NULL;
    }

    return first;
}

/** Gets the adjacent block or returns NULL if there is not one. */
Block *next_block(Block *block)
{
    size_t distance = (block->info.size > 0) ? block->info.size : -block->info.size;

    Block *end = (Block *)UNSCALED_POINTER_ADD(mem_heap_lo(), heap_size);
    Block *next = (Block *)UNSCALED_POINTER_ADD(block, sizeof(BlockInfo) + distance);
    if (next >= end)
    {
        return NULL;
    }

    return next;
}

/** Print the heap by iterating through it as an implicit free list. */
void examine_heap()
{
    // print to stderr so output isn't buffered and not output if we crash
    Block *curr = (Block *)mem_heap_lo();                                 // pointer to the current block
    Block *end = (Block *)UNSCALED_POINTER_ADD(mem_heap_lo(), heap_size); // pointer to the tail block
    fprintf(stderr, "heap size:\t0x%lx\n", heap_size);
    fprintf(stderr, "heap start:\t%p\n", curr);
    fprintf(stderr, "heap end:\t%p\n", end);

    fprintf(stderr, "free_list_head: %p\n", (void *)free_list_head);

    fprintf(stderr, "malloc_list_tail: %p\n", (void *)malloc_list_tail);

    while (curr && curr < end)
    {
        // print out common block attributes and allocated/free specific data
        fprintf(stderr, "%p: %ld\t", (void *)curr, curr->info.size);
        if (curr->info.size > 0)
        {
            fprintf(stderr, "ALLOCATED\tprev: %p\n", (void *)curr->info.prev);
        }
        else
        {
            fprintf(stderr, "FREE\tnextFree: %p, prevFree: %p, prev: %p\n", (void *)curr->freeNode.nextFree, (void *)curr->freeNode.prevFree, (void *)curr->info.prev);
        }

        curr = next_block(curr);
    }
    fprintf(stderr, "END OF HEAP\n\n");

    curr = free_list_head;
    fprintf(stderr, "Head ");
    while (curr)
    {
        fprintf(stderr, "-> %p ", curr);
        curr = curr->freeNode.nextFree;
    }
    fprintf(stderr, "\n");
}

/** Checks the heap data structure for consistency. */
int check_heap()
{
    Block *curr = (Block *)mem_heap_lo();
    Block *end = (Block *)UNSCALED_POINTER_ADD(mem_heap_lo(), heap_size);
    Block *last = NULL;
    long int free_count = 0;

    while (curr && curr < end)
    {
        if (curr->info.prev != last)
        {
            fprintf(stderr, "check_heap: Error: previous link not correct.\n");
            examine_heap();
        }

        if (curr->info.size <= 0)
        {
            // Free
            free_count++;
        }

        last = curr;
        curr = next_block(curr);
    }

    curr = free_list_head;
    last = NULL;
    while (curr)
    {
        if (curr == last)
        {
            fprintf(stderr, "check_heap: Error: free list is circular.\n");
            examine_heap();
        }
        last = curr;
        curr = curr->freeNode.nextFree;
        if (free_count == 0)
        {
            fprintf(stderr, "check_heap: Error: free list has more items than expected.\n");
            examine_heap();
        }
        free_count--;
    }

    return 0;
}
