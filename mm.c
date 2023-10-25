#include <stdio.h>
#include <stdlib.h>

#include "memlib.h"
#include "mm.h"

#define DEBUG 0

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

    Block *block = searchFreeList(reqSize);

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

#if DEBUG
    // DEBUG
    check_heap();
#endif

    return UNSCALED_POINTER_ADD(block, INFO_SIZE); // pointer to the data
}

void mm_free(void *ptr)
{
    Block *block = (Block *)UNSCALED_POINTER_SUB(ptr, INFO_SIZE); // pointer to a block

    if (block->info.size <= 0)
    {
        fprintf(stderr, "mm_free(): This block is already free.");
        return;
    }
    block->info.size *= -1;

    // update the free list
    add_to_free_list(block);
    coalesce(block);

#if DEBUG
    // DEBUG
    check_heap();
#endif
}

/*********************************************/
/**************** Searching  *****************/
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

Block *searchFreeList(size_t reqSize)
{
    // check for positive request size
    if (reqSize <= 0)
    {
        fprintf(stderr, "searchList(): The request size must be more than 0.");
        return NULL;
    }

    Block *curr = free_list_head;

    // check for empty list
    if (curr == NULL)
    {
        return NULL;
    }

    // find the next available block
    while ((curr != NULL) &&
           (curr->info.size > 0 ||
            -(curr->info.size) < (signed long long)(reqSize)))
    {
        curr = next_block(curr);
    }

    // split block if possible
    if (curr != NULL && (labs(curr->info.size) - reqSize >= SPLIT_THRESHOLD))
    {
        split(curr, reqSize);
    }

    return curr;
}

/*********************************************/
/************* Resizing Blocks  **************/
/*********************************************/

void coalesce(Block *block)
{
    Block *prev = block->info.prev;
    Block *next = next_block(block);

    if (prev != NULL && prev->info.size < 0)
    {
        // coalesce all three blocks
        if (next != NULL && next->info.size < 0)
        {
            // update previous' size
            prev->info.size -= 2 * INFO_SIZE + labs(block->info.size) + labs(next->info.size);

            // update new nextblock's previous
            Block *new_next = next_block(next);
            if (new_next != NULL)
            {
                new_next->info.prev = prev;
            }
            else // next was the list tail
            {
                malloc_list_tail = prev;
            }

            // remove blocks from free list
            remove_from_free_list(block);
            remove_from_free_list(next);

#if DEBUG
            // DEBUG
            check_heap();
#endif
        }
        // coalesce previous and current blocks
        else
        {
            // update previous' size
            prev->info.size -= INFO_SIZE + labs(block->info.size);

            // update malloc and free lists
            remove_from_free_list(block);
            if (next != NULL) // update previous pointer
            {
                next->info.prev = prev;
            }
            else // update the list tail
            {
                malloc_list_tail = prev;
            }

#if DEBUG
            // DEBUG
            check_heap();
#endif
        }
    }
    // coalesce current and next blocks
    else if (next != NULL && next->info.size < 0)
    {
        // update current's size
        block->info.size -= INFO_SIZE + labs(next->info.size);

        // update new nextblock's previous
        Block *new_next = next_block(next);
        if (new_next != NULL)
        {
            new_next->info.prev = block;
        }
        else // next was the list tail
        {
            malloc_list_tail = block;
        }

        // remove block from free list
        remove_from_free_list(next);

#if DEBUG
        // DEBUG
        check_heap();
#endif
    }
}

void split(Block *block, size_t reqSize)
{
    // create a new block
    Block *new = (Block *)UNSCALED_POINTER_ADD(block, INFO_SIZE + reqSize);
    new->info.prev = block;
    new->info.size = -(labs(block->info.size) - (INFO_SIZE + reqSize)); // already aligned

    // add to lists
    add_to_free_list(new);
    Block *next = next_block(block);
    if (next != NULL)
    {
        next->info.prev = new;
    }
    else
    {
        malloc_list_tail = new;
    }

    // update previous block's size
    block->info.size = -reqSize;

#if DEBUG
    // DEBUG
    check_heap();
#endif
}

/*********************************************/
/*********** Linked List Functions ***********/
/*********************************************/

void insert_at_tail(Block *block)
{
    block->info.prev = malloc_list_tail;
    malloc_list_tail = block;

#if DEBUG
    // DEBUG
    check_heap();
#endif
}

void add_to_free_list(Block *block)
{
    // empty list
    if (free_list_head == NULL)
    {
        block->freeNode.nextFree = NULL;
        block->freeNode.prevFree = NULL;
        free_list_head = block;
    }

    // append to the beginning of the list
    else
    {
        free_list_head->freeNode.prevFree = block;
        block->freeNode.nextFree = free_list_head;
        block->freeNode.prevFree = NULL;
        free_list_head = block;
    }

#if DEBUG
    // DEBUG
    check_heap();
#endif
}

void remove_from_free_list(Block *block)
{
    Block *prev = block->freeNode.prevFree;
    Block *next = block->freeNode.nextFree;

    // set the previous' next to the previous
    if (prev != NULL)
    {
        prev->freeNode.nextFree = next;
    }
    else // update head of the list
    {
        free_list_head = next;
    }

    // set the next's previous to the previous
    if (next != NULL)
    {
        next->freeNode.prevFree = prev;
    }

#if DEBUG
    // DEBUG
    check_heap();
#endif
}

/*********************************************/
/*************** Inspect Heap  ***************/
/*********************************************/

void examine_heap()
{
    // print to stderr so output isn't buffered and not output if we crash
    Block *curr = (Block *)mem_heap_lo();                                 // pointer to the current block
    Block *end = (Block *)UNSCALED_POINTER_ADD(mem_heap_lo(), heap_size); // pointer to the tail block
    void *prev = NULL;

    fprintf(stderr, "heap size:\t0x%lu\n", (long)heap_size);
    fprintf(stderr, "heap start:\t%p\n", curr);
    fprintf(stderr, "heap end:\t%p\n", end);

    fprintf(stderr, "free_list_head: %p\n", (void *)free_list_head);

    fprintf(stderr, "malloc_list_tail: %p\n", (void *)malloc_list_tail);

    int all_correct = 1;
    while (curr && curr < end)
    {
        // print out common block attributes
        fprintf(stderr, "%p: %ld\t", (void *)curr, curr->info.size);

        // and allocated/free specific data
        if (curr->info.size > 0)
        {
            fprintf(stderr, "ALLOCATED\tprev: %p", (void *)curr->info.prev);
        }
        else
        {
            fprintf(stderr, "FREE\tnextFree: %p, prevFree: %p, prev: %p", (void *)curr->freeNode.nextFree, (void *)curr->freeNode.prevFree, (void *)curr->info.prev);
        }

        // verify previous pointers
        if (curr->info.prev == prev)
        {
            fprintf(stderr, " ✓\n");
        }
        else
        {
            fprintf(stderr, " ✗\n");
            all_correct = 0;
        }

        // move to next
        prev = curr;
        curr = next_block(curr);
    }
    fprintf(stderr, "END OF HEAP\n\n");

    if (!all_correct)
    {
        fprintf(stderr, "Not all pointers were correct.");
    }

    examine_free_list();

    fprintf(stderr, "\n");
}

void examine_free_list()
{
    Block *curr = free_list_head;
    fprintf(stderr, "HEAD OF FREE LIST: ");

    // print out links
    while (curr)
    {
        fprintf(stderr, "-> %p ", curr);
        curr = curr->freeNode.nextFree;
    }
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
            examine_heap();
            fprintf(stderr, "check_heap: Error: previous link not correct.\nCurr = %p, curr->prev = %p, previous = %p\n\n",
                    curr, curr->info.prev, last);
        }

        if (curr->info.size <= 0)
        {
            // Free
            free_count++;
        }

        last = curr;
        curr = next_block(curr);
    }

    // check malloc list tail
    if (last != malloc_list_tail)
    {
        fprintf(stderr, "check_heap: Error: malloc list tail incorrect\nCurrent Tail: %p, Correct Tail: %p",
                malloc_list_tail, last);
    }

    curr = free_list_head;
    last = NULL;
    while (curr)
    {
        if (curr == last)
        {
            examine_heap();
            fprintf(stderr, "check_heap: Error: free list is circular.\n\n");
        }
        last = curr;
        curr = curr->freeNode.nextFree;
        if (free_count == 0)
        {
            examine_heap();
            fprintf(stderr, "check_heap: Error: free list has more items than expected.\n\n");
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
