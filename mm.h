
#define UNSCALED_POINTER_ADD(p, x) ((void *)((char *)(p) + (x)))
#define UNSCALED_POINTER_SUB(p, x) ((void *)((char *)(p) - (x)))

/** Size of a word on this architecture. */
#define WORD_SIZE sizeof(void *)

/*********************************************/
/************** Block Structure **************/
/*********************************************/

/**
 * Size of the BlockInfo structure.
 */
#define INFO_SIZE (sizeof(BlockInfo))
/**
 * Size of the FreeBlockInfo structure for free blocks.
 * Alignment of blocks returned by mm_malloc.
 */
#define FREE_INFO_SIZE (sizeof(FreeBlockInfo))

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

/** Pointer to the tail in the malloc list */
static Block *malloc_list_tail = NULL;
/** Pointer to the head (a FreeBlockInfo pointer) in the free list. */
static Block *free_list_head = NULL;

/*********************************************/
/************* Manage Heap Memory ************/
/*********************************************/

/** Size of the heap in bytes. */
static size_t heap_size = 0;

/**
 * Allocate a block of memory of the given size.
 * Returns a pointer to the block or, if size is zero, returns NULL.
 */
extern void *mm_malloc(size_t size);

/* Deallocate the given pointer that was previously allocated by mm_malloc. */
extern void mm_free(void *ptr);

/*********************************************/
/**************** Searching  *****************/
/*********************************************/

/**
 * Looks for the first free block that can fit the given amount of space.
 * Returns a pointer to the free block or NULL in such does not exist.
 */
Block *searchList(size_t reqSize);

/**
 * Looks for the first free block that can fit the given amount of space.
 * Returns a pointer to the free block or NULL in such does not exist.
 * Only searches in the list of free blocks.
 */
Block *searchFreeList(size_t reqSize);

/*********************************************/
/************* Resizing Blocks  **************/
/*********************************************/

#define SPLIT_THRESHOLD sizeof(BlockInfo) + sizeof(FreeBlockInfo)

/** Coalesces surrounding free blocks and updates free list. */
void coalesce(Block *block);

/**
 * Splits the block into two separate blocks.
 * While maintaining the required size of the first block,
 * it creates a new block and adds it to the malloc list and free list.
 *
 * @param reqSize Aligned size of a request for memory allocation.
 *          Must be aligned to a multiple of 8.
 */
void split(Block *block, size_t reqSize);

/*********************************************/
/*********** Linked List Functions ***********/
/*********************************************/

/** Append a block to the end of malloc list. */
void insert_at_tail(Block *block);

/**
 * Adds a block to the list of free blocks.
 * Uses space to create to create FreeBlockInfo structure.
 */
void add_to_free_list(Block *block);

/**
 * Removes a block from the list of free blocks.
 */
void remove_from_free_list(Block *block);

/*********************************************/
/************** Inspect  Heap  ***************/
/*********************************************/

/** Prints a thorough listing of the heap data structure. */
void examine_heap();

/** Prints a thorough listing of the free blocks in the heap data structure. */
void examine_free_list();

/** Checks the heap for any issues and prints out errors as it finds them. */
int check_heap();

/*********************************************/
/*************** Backend Heap  ***************/
/*********************************************/

/** Initialize the allocator. */
int mm_init();

/**
 * Have the OS allocate more space for the heap.
 * Returns a pointer to that new space, always larger than the last request and contiguous in memory.
 */
void *requestMoreSpace(size_t reqSize);

/** Returns a pointer to the first block or returns NULL if there is not one. */
Block *first_block();

/**
 * Returns a pointer to the adjacent block or returns NULL if there is not one.
 */
Block *next_block(Block *block);