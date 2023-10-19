
#define UNSCALED_POINTER_ADD(p, x) ((void *)((char *)(p) + (x)))
#define UNSCALED_POINTER_SUB(p, x) ((void *)((char *)(p) - (x)))

/** Size of a word on this architecture. */
#define WORD_SIZE sizeof(void *)

/**
 * Alignment of blocks returned by mm_malloc.
 * Each allocation needs to be at least be big enough for the free space metadata.
 */
#define ALIGNMENT (sizeof(FreeBlockInfo))

/**
 * An BlockInfo contains information about a block, including the size
 * as well as pointers to the next and previous blocks in the free list.
 */
typedef struct _BlockInfo
{
    /**
     * Size of the block and whether or not the block is in use or free.
     * When the size is negative, the block is currently free.
     */
    long int size;
    /** Pointer to the previous block in the list. */
    struct _Block *prev;
} BlockInfo;

/**
 * A FreeBlockInfo contains metadata just for free blocks.
 *
 * These are in the region of memory that is normally used by the
 * program when the block is allocated.
 */
typedef struct _FreeBlockInfo
{
    /** Pointer to the next free block in the list. */
    struct _Block *nextFree;
    /** Pointer to the previous free block in the list. */
    struct _Block *prevFree;
} FreeBlockInfo;

/**
 * A Block serves as a nodes containing information about the block and
 * metadata regarding the free block.
 */
typedef struct _Block
{
    BlockInfo info;
    FreeBlockInfo freeNode;
} Block;

/** Pointer to the first FreeBlockInfo in the free list, the list's head. */
static Block *free_list_head = NULL;
static Block *malloc_list_tail = NULL;

static size_t heap_size = 0;

/**
 * Have the OS allocate more space for the heap.
 * Returns a pointer to that new space, always larger than the last request and
 * contiguous in memory.
 */
void *requestMoreSpace(size_t reqSize);

/** Returns a pointer to the first block or returns NULL if there is not one. */
Block *first_block();

/**
 * Returns a pointer to the adjacent block or returns NULL if there is not one.
 */
Block *next_block(Block *block);

/** Prints a thorough listing of the heap data structure. */
void examine_heap();

/** Checks the heap for any issues and prints out errors as it finds them. */
int check_heap();