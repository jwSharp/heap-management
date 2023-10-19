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