#include <stdio.h>
#include <stdlib.h>

#include "memlib.h"
#include "mm.h"

/*********************************************/
/************* Manage Heap Memory ************/
/*********************************************/

void *mm_malloc(size_t size)
{
    // zero- or negative-size requests are invalid
    if (size <= 0)
    {
        fprintf(stderr, "mm_malloc(): Cannot allocate negative space or no space.");
        return NULL;
    }

    // determine size of data and size of request
    long int reqSize = FREE_INFO_SIZE * ((size + FREE_INFO_SIZE - 1) / FREE_INFO_SIZE); // adjust for header and alignment

    Block *block = searchList(reqSize);

    // check for no fit
    if (block == NULL)
    {
        // request enough for the header
        block = (Block *)requestMoreSpace(INFO_SIZE + reqSize);
        block->info.size = -reqSize;

        insert_at_tail(block);
    }
    else
    {
        remove_from_free_list(block);
    }

    // allocate block
    block->info.size *= -1;
    // examine_heap(); visual debug
    return UNSCALED_POINTER_ADD(block, INFO_SIZE); // pointer to the data
}

void mm_free(void *ptr)
{
    Block *block = (Block *)UNSCALED_POINTER_SUB(ptr, INFO_SIZE); // pointer to a block

    if (block->info.size >= 0)
    {
        ; // fprintf(stderr, "mm_free(): This block is already free.");
    }
    block->info.size *= -1;

    add_to_free_list(block);
}

/*********************************************/
/*********** Linked List Functions ***********/
/*********************************************/

Block *searchList(size_t reqSize)
{
    // check for positive request size
    if (reqSize <= 0)
    {
        fprintf(stderr, "searchList(): The request size must be more than 0.");
        return NULL;
    }

    Block *curr = first_block();

    // check for empty list
    if (curr == NULL)
    {
        return NULL;
    }

    while ((curr != NULL) &&
           (curr->info.size > 0 ||
            -(curr->info.size) < (signed long long)(reqSize)))
    {
        curr = next_block(curr);
    }

    return curr;
}

void insert_at_tail(Block *block)
{
    block->info.prev = malloc_list_tail;
    malloc_list_tail = block;
}

/*********************************************/
/*************** Free Blocks  ****************/
/*********************************************/

void add_to_free_list(Block *block)
{
    ;
}

void remove_from_free_list(Block *block)
{
    ;
}

/*********************************************/
/************** Inspect  Heap  ***************/
/*********************************************/

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
        // print out common block attributes
        fprintf(stderr, "%p: %ld\t", (void *)curr, curr->info.size);

        // and allocated/free specific data
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

/*********************************************/
/*************** Backend Heap  ***************/
/*********************************************/

int mm_init()
{
    free_list_head = NULL;
    malloc_list_tail = NULL;
    heap_size = 0;

    return 0;
}

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

Block *first_block()
{
    // first address in the heap
    Block *first = (Block *)mem_heap_lo();
    if (heap_size == 0)
    {
        return NULL;
    }

    return first;
}

Block *next_block(Block *block)
{
    size_t distance = (block->info.size > 0) ? block->info.size : -block->info.size;

    // last address in the heap
    Block *end = (Block *)UNSCALED_POINTER_ADD(mem_heap_lo(), heap_size);
    Block *next = (Block *)UNSCALED_POINTER_ADD(block, sizeof(BlockInfo) + distance);
    if (next >= end)
    {
        return NULL;
    }

    return next;
}
