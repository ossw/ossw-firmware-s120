/*#define NDEBUG*/
#include "tinymem.h"



/*---------------------------------------------------------------------------*/
/*      Check pre-compiler errors                                            */

#if     (UINT_MAX == UINT16_MAX)
#define INTSIZE     2
#elif   (UINT_MAX == UINT32_MAX)
#define INTSIZE     4
#elif   (UINT_MAX == UINT64_MAX)
#define INTSIZE     8
#endif

#ifndef TM_BLOCK_SIZE       // user didn't set block size, use default
    #define TM_BLOCK_TYPE           free_block
    #define TM_BLOCK_SIZE           (TM_INDEX_SIZE * 2)
#elif   (TM_BLOCK_SIZE == 2)
    #define TM_BLOCK_TYPE           uint16_t
#elif   (TM_BLOCK_SIZE == 4)
    #define TM_BLOCK_TYPE           uint32_t
#elif   (TM_BLOCK_SIZE == 8)
    typedef struct {uint32_t values[2];} TM_BLOCK_TYPE;
#elif   (TM_BLOCK_SIZE == 16)
    typedef struct {uint32_t values[4];} TM_BLOCK_TYPE;
#endif

#if     (TM_BLOCK_SIZE < 2 * TM_INDEX_SIZE)
#error  "block size must be 2 times bigger than index size"
#endif

#define TM_POOL_BLOCKS          (TM_POOL_SIZE / TM_BLOCK_SIZE)  // total blocks available

#if     (TM_POOL_BLOCKS < 256)
    typedef uint8_t         tm_blocks_t;
#else
    typedef uint16_t        tm_blocks_t;
#endif

#if TM_POOL_SIZE % (TM_BLOCK_SIZE)
#error "Invalid pool size, must be divisible by free_block"
#endif

#if (TM_POOL_INDEXES % (8 * INTSIZE))
#error "Invalid pool ptrs size, must be divisible by int"
#endif

/*---------------------------------------------------------------------------*/
/**
 * \brief           poolptr is used by Pool to track memory location and size
 */
typedef struct {
    tm_blocks_t loc;
    tm_index_t next;
} TM_H_ATTPACKSUF poolptr;

/*---------------------------------------------------------------------------*/
/**
 * \brief           free_block is stored INSIDE of freed memory as a linked list
 *                  of all freed data
 */
// TODO: packed!
typedef struct {
    tm_index_t prev;
    tm_index_t next;
} TM_H_ATTPACKSUF free_block;


#define CEILING(x, y)           (((x) % (y)) ? (x)/(y) + 1 : (x)/(y))
#define BOOL(value)             ((value) ? 1: 0)

// Used in defrag to subtract from clocks_left
#define INDEX_REMOVE_CLOCKS     (10)
#define FREED_REMOVE_CLOCKS     (8)
#define SPLIT_CLOCKS            (20)

#define MAX_BIT_INDEXES     (TM_POOL_INDEXES / (8 * sizeof(int)))          // for filled/points
#define MAXUINT             ((unsigned int) 0xFFFFFFFFFFFFFFFF)
#define INTBITS             (sizeof(int) * 8)                               // bits in an integer
// Masks of an integer
#define LOWER_MASK          (((unsigned int)0xFFFFFFFFFFFFFFFF) >> (sizeof(int) * 8 / 2))
#define UPPER_MASK          (LOWER_MASK << (sizeof(int) * 8 / 2))

#define FREED_BINS          (12)                                // bins to store freed values

// data is aligned on blocks
#define ALIGN_BLOCKS(size)  CEILING(size, TM_BLOCK_SIZE)           // get block value that can encompase size
#define ALIGN_BYTES(size)   (ALIGN_BLOCKS(size) * TM_BLOCK_SIZE)   // get value in bytes

#define CPU_CLOCKS_PER_US       ((CPU_CLOCKS_PER_SEC) / 1000000)
#define CPU_CLOCKS_PER_CLOCK    (CPU_CLOCKS_PER_SEC / CLOCKS_PER_SEC)

#ifndef TM_THREAD_TIME_US
#define TM_THREAD_TIME_US      2
#endif

/*---------------------------------------------------------------------------*/
/**
 * \brief           Initialize (reset) the pool
 */
#define tm_init()  ((Pool) {                                    \
    .filled = {1},                      /*NULL is taken*/       \
    .points = {1},                      /*NULL is taken*/       \
    .pointers = {{0, 0}},               /*heap = 0*/            \
    .freed = {0},                                               \
    .filled_blocks = 0,                                         \
    .freed_blocks = 0,                                          \
    .ptrs_filled = 1,                    /*NULL is "filled"*/   \
    .ptrs_freed = 0,                                            \
    .find_index = 0,                                            \
    .first_index = 0,                                           \
    .last_index = 0,                                            \
    .status = 0,                                                \
    .defrag_index=0,                                            \
    .defrag_prev=0,                                             \
})

/*---------------------------------------------------------------------------*/
/**
 * \brief           Pool object to track all memory usage
 *                  This is the main object used by tinymem to do memory
 *                  management
 */
typedef struct {
    TM_BLOCK_TYPE   pool[TM_POOL_BLOCKS];           //!< Actual memory pool (very large)
    unsigned int    filled[MAX_BIT_INDEXES];     //!< bit array of filled pointers (only used, not freed)
    unsigned int    points[MAX_BIT_INDEXES];     //!< bit array of used pointers (both used and freed)
    poolptr         pointers[TM_POOL_INDEXES];     //!< This is the index lookup location
    tm_index_t      freed[FREED_BINS];           //!< binned storage of all freed indexes
    tm_blocks_t     filled_blocks;                //!< total amount of data allocated
    tm_blocks_t     freed_blocks;                 //!< total amount of data freed
    tm_index_t      ptrs_filled;                    //!< total amount of pointers allocated
    tm_index_t      ptrs_freed;                     //!< total amount of pointers freed
    tm_index_t      find_index;                     //!< speed up find index
    tm_index_t      first_index;                    //!< required to start defrag
    tm_index_t      last_index;                     //!< required to allocate off heap
    uint8_t         status;                         //!< status byte. Access with Pool_status macros
    tm_index_t      defrag_index;                   //!< used during defrag
    tm_index_t      defrag_prev;                    //!< used during defrag
} Pool;


Pool tm_pool = { 
.filled = { 1 },                      /*NULL is taken*/       
.points = { 1 },                      /*NULL is taken*/       
.pointers = { { 0, 0 } },               /*heap = 0*/            
.freed = { 0 },                                               
.filled_blocks = 0,                                         
.freed_blocks = 0,                                          
.ptrs_filled = 1,                    /*NULL is "filled"*/   
.ptrs_freed = 0,                                            
.find_index = 0,                                            
.first_index = 0,                                           
.last_index = 0,                                            
.status = 0,                                                
.defrag_index = 0,                                            
.defrag_prev = 0,                                             
};  // holds all allocations, deallocates and pretty much everything else


/*---------------------------------------------------------------------------*/
/*      Local Functions Declarations                                         */

bool     tm_defrag(void);
tm_index_t      find_index(void);
uint8_t         freed_bin(const tm_blocks_t blocks);
uint8_t         freed_bin_get(const tm_blocks_t blocks);
void     freed_remove(const tm_index_t index);
void     freed_insert(const tm_index_t index);
tm_index_t      freed_get(const tm_blocks_t size);

void index_extend(const tm_index_t index, const tm_blocks_t blocks, const bool filled);
void index_remove(const tm_index_t index, const tm_index_t prev_index, const bool defrag);
void index_join(const tm_index_t index, const tm_index_t with_index, int32_t *clocks_left);
bool index_split(const tm_index_t index, const tm_blocks_t blocks, tm_index_t new_index);
#define free_p(index)  ((free_block *)tm_void_p(index))

// For testing
void                freed_print(void);
void                freed_full_print(bool full);
tm_index_t          freed_count_print(tm_size_t *size, bool pnt);
void       				  index_print(tm_index_t index);

tm_index_t          freed_count(tm_size_t *size);
tm_index_t          freed_count_bin(uint8_t bin, tm_size_t *size, bool pnt);
bool                freed_isvalid(void);
bool                freed_isin(const tm_index_t index);
bool                pool_isvalid(void);
void                fill_index(tm_index_t index);
bool                check_index(tm_index_t index);

/*---------------------------------------------------------------------------*/
/**
 * \brief           Access Pool Characteristics
 */
#define BLOCKS_LEFT                 (TM_POOL_BLOCKS - tm_pool.filled_blocks)
#define BYTES_LEFT                  (BLOCKS_LEFT * TM_BLOCK_SIZE)
#define PTRS_USED                   (tm_pool.ptrs_filled + tm_pool.ptrs_freed)
#define PTRS_LEFT                   (TM_POOL_INDEXES - tm_pool.ptrs_filled) // ptrs potentially left
#define PTRS_AVAILABLE              (TM_POOL_INDEXES - PTRS_USED) // ptrs available for immediate use
#define HEAP_LEFT                   (TM_POOL_BLOCKS - HEAP)
#define HEAP_LEFT_BYTES             (HEAP_LEFT * TM_BLOCK_SIZE)

/**
 * \brief           Get, set or clear the status bit (0 or 1) of name
 */
#define STATUS(name)                ((tm_pool.status) & (name))
#define STATUS_SET(name)            (tm_pool.status |= (name))
#define STATUS_CLEAR(name)          (tm_pool.status &= ~(name))

/*---------------------------------------------------------------------------*/
/**
 * \brief           Access index characteristics
 */
#define LOCATION(index)             (tm_pool.pointers[index].loc)
#define HEAP                        (tm_pool.pointers[0].loc)
#define NEXT(index)                 (tm_pool.pointers[index].next)
#define FREE_NEXT(index)            ((free_p(index))->next)
#define FREE_PREV(index)            ((free_p(index))->prev)
#define BLOCKS(index)               ((tm_blocks_t) (LOCATION(tm_pool.pointers[index].next) - \
                                        LOCATION(index)))       // sizeof index in blocks
#define LOC_VOID(loc)               ((void*)(tm_pool.pool + (loc)))

/*---------------------------------------------------------------------------*/
/**
 * \brief           Move memory from one index to another
 */
#define MEM_MOVE(index_to, index_from)  memmove(                \
            tm_void_p(index_to),                                    \
            tm_void_p(index_from),                                  \
            tm_sizeof(index_from)                                   \
        )

/**
 *                  FILLED* does operations on Pool's `filled` bit array
 *                  POINTS* does operations on Pool's `points` bit array
 */
#define BITARRAY_INDEX(index)       ((index) / (sizeof(int) * 8))
#define BITARRAY_BIT(index)         (1 << ((index) % (sizeof(int) * 8)))
#define FILLED(index)               (tm_pool.filled[BITARRAY_INDEX(index)] &   BITARRAY_BIT(index))
#define FILLED_SET(index)           (tm_pool.filled[BITARRAY_INDEX(index)] |=  BITARRAY_BIT(index))
#define FILLED_CLEAR(index)         (tm_pool.filled[BITARRAY_INDEX(index)] &= ~BITARRAY_BIT(index))
#define POINTS(index)               (tm_pool.points[BITARRAY_INDEX(index)] &   BITARRAY_BIT(index))
#define POINTS_SET(index)           (tm_pool.points[BITARRAY_INDEX(index)] |=  BITARRAY_BIT(index))
#define POINTS_CLEAR(index)         (tm_pool.points[BITARRAY_INDEX(index)] &= ~BITARRAY_BIT(index))


/*---------------------------------------------------------------------------*/
/*      Global Function Definitions                                          */
uint16_t tm_free_pointers(void) {
		return TM_POOL_INDEXES - tm_pool.ptrs_filled;
}

uint16_t tm_free_blocks(void) {
		return TM_POOL_BLOCKS - tm_pool.filled_blocks;
}

inline void tm_reset(void){
    tm_pool = tm_init();
}

/*---------------------------------------------------------------------------*/
inline tm_size_t tm_sizeof(const tm_index_t index){
    return BLOCKS(index) * TM_BLOCK_SIZE;
}

/*---------------------------------------------------------------------------*/
void *          tm_void_p(const tm_index_t index){
    // Note: index 0 has location == heap (it is where Pool_heap is stored)
    if(LOCATION(index) >= HEAP) return NULL;
    return tm_pool.pool + LOCATION(index);
}

/*---------------------------------------------------------------------------*/
tm_index_t      tm_alloc(tm_size_t size){
    tm_index_t index;
    size = ALIGN_BLOCKS(size);  // convert from bytes to blocks
    if(BLOCKS_LEFT < size) return 0;
    index = freed_get(size);
    if(index){
        if(BLOCKS(index) != size){ // Split the index if it is too big
            if(!index_split(index, size, 0)){
                // Split can fail if there are not enough pointers
                tm_free(index);
                STATUS_SET(TM_DEFRAG_FAST);  // need more indexes
                return 0;
            }
        }
        return index;
    }
    if(HEAP_LEFT < size){
        STATUS_SET(TM_DEFRAG_FAST);  // need less fragmentation
        return 0;
    }
    if(!PTRS_LEFT) return 0;
    index = find_index();
    if(!index){
        STATUS_SET(TM_DEFRAG_FAST);  // need more indexes
        return 0;
    }
    index_extend(index, size, true);  // extend index onto heap
    return index;
}


/*---------------------------------------------------------------------------*/
tm_index_t      tm_realloc(tm_index_t index, tm_size_t size){
    tm_index_t new_index;
    tm_blocks_t prev_size;
    size = ALIGN_BLOCKS(size);

    assert(0);  // not used currently
    if(!index) return tm_alloc(size);
    if(!FILLED(index)) return 0;
    if(!size){
        tm_free(index);
        return 0;
    }
    new_index = NEXT(index);
    if(!FILLED(new_index)){
        // If next index is free, always join it first
        index_join(index, new_index, NULL);
    }
    prev_size = BLOCKS(index);
    if(size == BLOCKS(index)) return index;
    if(size < prev_size){  // shrink data
        if(!index_split(index, size, 0)) return 0;
        return index;
    } else{  // grow data
        new_index = tm_alloc(size * TM_BLOCK_SIZE);
        if(!new_index) return 0;
        MEM_MOVE(new_index, index);
        tm_free(index);
        return new_index;
    }
}

/*---------------------------------------------------------------------------*/
void            tm_free(const tm_index_t index){
    if(!index) return;      // ISO requires free(NULL) be a NO-OP
    assert(LOCATION(index) < HEAP);
    assert(index < TM_POOL_INDEXES);
    assert(FILLED(index));
    FILLED_CLEAR(index);
    tm_pool.filled_blocks -= BLOCKS(index);
    tm_pool.freed_blocks += BLOCKS(index);
    tm_pool.ptrs_filled--;
    tm_pool.ptrs_freed++;
    freed_insert(index);
    // Join all the way up if next index is free
    if(!FILLED(NEXT(index))){
        index_join(index, NEXT(index), NULL);
    }
}

/*---------------------------------------------------------------------------*/
bool            tm_valid(const tm_index_t index){
    if(index >= TM_POOL_INDEXES)               return false;
    if(LOCATION(index) >= TM_POOL_BLOCKS)          return false;
    if((!POINTS(index)) || (!FILLED(index)))    return false;
    return true;
}

/*---------------------------------------------------------------------------*/
inline bool     tm_check(const tm_index_t index, const tm_size_t size){
    if(!tm_valid(index))         return false;
    if(tm_sizeof(index) != size) return false;
    return true;
}

/*---------------------------------------------------------------------------*/
bool     tm_thread(){
    if(STATUS(TM_ANY_DEFRAG)){
        return tm_defrag();
    }
    if((uint32_t)HEAP * 100 / TM_POOL_BLOCKS >= TM_DEFRAG_SIZE){
        // check if there are blocks to be recovered
        if((uint32_t)tm_pool.freed_blocks * 100 / (tm_pool.filled_blocks + tm_pool.freed_blocks)
                >= TM_DEFRAG_MIN){
            STATUS_SET(TM_DEFRAG_FAST);
        }
        return 1;
    }
    if((uint32_t)PTRS_USED * 100 / TM_POOL_INDEXES >= TM_DEFRAG_INDEXES){
        // check if there are indexes to be recovered
        if((uint32_t)tm_pool.ptrs_freed * 100 / (tm_pool.ptrs_filled + tm_pool.ptrs_freed) >= TM_DEFRAG_MIN){
            STATUS_SET(TM_DEFRAG_FAST);
        }
        return 1;
    }
    return 0;   // no operations pending
}

/*---------------------------------------------------------------------------*/
inline bool         tm_defrag(void){
#ifndef NDEBUG
    tm_index_t i = 0;
    tm_blocks_t used = tm_pool.filled_blocks;
    tm_blocks_t available = BLOCKS_LEFT;
#endif
    int32_t clocks_left = CPU_CLOCKS_PER_US * TM_THREAD_TIME_US;
    tm_blocks_t blocks;
    tm_blocks_t location;
    if(!STATUS(TM_DEFRAG_IP)){
        tm_pool.defrag_index = tm_pool.first_index;
        tm_pool.defrag_prev = 0;
        STATUS_CLEAR(TM_ANY_DEFRAG);
        STATUS_SET(TM_DEFRAG_FULL_IP);
    }
    if(!tm_pool.defrag_index) goto done;
    while(NEXT(tm_pool.defrag_index)){
        if(!FILLED(tm_pool.defrag_index)){
            clocks_left -= 30 + INDEX_REMOVE_CLOCKS + SPLIT_CLOCKS;
            if(!FILLED(NEXT(tm_pool.defrag_index))){
                index_join(tm_pool.defrag_index, NEXT(tm_pool.defrag_index), &clocks_left);
                if(clocks_left < 0) return 1;
            }
            if(!NEXT(tm_pool.defrag_index)) break;

            /*DBGprintf("### Defrag: loop=%-11u", i); index_print(tm_pool.defrag_index);*/
            assert(FILLED(NEXT(tm_pool.defrag_index)));
            blocks = BLOCKS(NEXT(tm_pool.defrag_index));        // store size of actual data
            location = LOCATION(NEXT(tm_pool.defrag_index));    // location of actual data
            clocks_left -= CEILING(blocks * TM_BLOCK_SIZE, sizeof(int));

            // Make index "filled", we will split it up later
            freed_remove(tm_pool.defrag_index);         // 7 clocks
            FILLED_SET(tm_pool.defrag_index);           // 2 clocks
            tm_pool.ptrs_filled++, tm_pool.filled_blocks+=BLOCKS(tm_pool.defrag_index);
            tm_pool.ptrs_freed--, tm_pool.freed_blocks-=BLOCKS(tm_pool.defrag_index);

            // Do an odd join, where the locations are just equal
            LOCATION(NEXT(tm_pool.defrag_index)) = LOCATION(tm_pool.defrag_index);

            // Now remove the index. Note that the size is == 0
            //      Also note that even though it was removed, it's NEXT and LOCATION
            //      are still valid (not changed in remove index)
            index_remove(tm_pool.defrag_index, tm_pool.defrag_prev, true);
            tm_pool.defrag_prev = NEXT(tm_pool.defrag_index);  // defrag_index was removed

            assert(LOCATION(tm_pool.defrag_prev) < TM_POOL_BLOCKS);
            assert(location < TM_POOL_BLOCKS);
            memmove(LOC_VOID(LOCATION(tm_pool.defrag_prev)),
                    LOC_VOID(location), ((tm_size_t)blocks) * TM_BLOCK_SIZE);
            if(!FILLED(NEXT(tm_pool.defrag_prev))){
                index_join(tm_pool.defrag_prev, NEXT(tm_pool.defrag_prev), &clocks_left);
            }
            assert(FILLED(NEXT(tm_pool.defrag_prev)));  // it will never "join up"
            if(!index_split(tm_pool.defrag_prev, blocks, tm_pool.defrag_index)){
                assert(0);
            } // note: tm_pool.defrag_index is now invalid (split used it)
            assert(BLOCKS(tm_pool.defrag_prev) == blocks);

            tm_pool.defrag_index = NEXT(tm_pool.defrag_prev);

            assert(!FILLED(tm_pool.defrag_index));

        } else{
            clocks_left -= 10;
            tm_pool.defrag_prev = tm_pool.defrag_index;
            tm_pool.defrag_index = NEXT(tm_pool.defrag_index);
        }
        assert(tm_pool.defrag_prev != tm_pool.defrag_index);
        assert((i++, used == tm_pool.filled_blocks));
        assert(available == BLOCKS_LEFT);
        /*if(clocks_left < -200) printf("clocks very low=%i\n", clocks_left);*/
        if(clocks_left < 0) return 1;
    }
done:
    if(!FILLED(tm_pool.defrag_index)){
        index_remove(tm_pool.defrag_index, tm_pool.defrag_prev, true);
    }
    STATUS_CLEAR(TM_DEFRAG_IP);
    STATUS_SET(TM_DEFRAG_FULL_DONE);
    /*tm_debug("filled end=%lu, total=%lu, operate=%lu, isavail=%lu",*/
            /*tm_pool.filled_blocks, TM_POOL_BLOCKS, TM_POOL_BLOCKS - tm_pool.filled_blocks,*/
            /*BLOCKS_LEFT);*/
    /*DBGprintf("## Defrag done: Heap left: start=%u, end=%lu, recovered=%lu, ",*/
            /*heap, HEAP_LEFT, HEAP_LEFT - heap);*/
    /*DBGprintf("wasfree=%u was_avail=%lu isavail=%lu,  \n", freed, available, BLOCKS_LEFT);*/
    /*assert(HEAP_LEFT - heap == freed);*/
    /*assert(tm_pool.freed_blocks == 0);*/
    /*assert(HEAP_LEFT == BLOCKS_LEFT);*/

    tm_pool.defrag_index = 0;
    tm_pool.defrag_prev = 0;
    return 0;
}

/*###########################################################################*/
/*      Local Functions                                                      */

/*---------------------------------------------------------------------------*/
tm_index_t      find_index(){
    uint8_t loop;
    unsigned int bits;
    uint8_t bit;
    uint8_t i;
    if(!PTRS_AVAILABLE) return 0;
    for(loop=0; loop<2; loop++){
        for(; tm_pool.find_index < MAX_BIT_INDEXES; tm_pool.find_index++){
            bits = tm_pool.points[tm_pool.find_index];
            if(bits != MAXUINT){
                bit = 0;
                if((bits & LOWER_MASK) == LOWER_MASK){
                    // bits are in the second half
                    bit += sizeof(int)  * 8 / 2;
                    bits = bits >> (sizeof(int) * 8 / 2);
                }
                for(i=0; i < sizeof(int) * 2; i++){
                    switch(bits & 0xF){
                        case 0x00: case 0x02: case 0x04: case 0x06:
                        case 0x08: case 0x0A: case 0x0c: case 0x0e:
                            goto found;
                        case 0x01: case 0x05: case 0x09: case 0x0d:
                            bit += 1;
                            goto found;
                        case 0x03: case 0x0b:
                            bit += 2;
                            goto found;
                        case 0x07:
                            bit += 3;
                            goto found;
                    }
                    bit += 4;
                    bits = bits >> 4;
                }
                assert(0);
found:
                assert(!POINTS(tm_pool.find_index * INTBITS + bit));
                assert(!FILLED(tm_pool.find_index * INTBITS + bit));
                return tm_pool.find_index * INTBITS + bit;
            }
        }
        tm_pool.find_index = 0;
    }
    assert(0);
	return 0;
}

/*---------------------------------------------------------------------------*/
/*      get the freed bin for blocks                                         */
uint8_t         freed_bin(const tm_blocks_t blocks){
    switch(blocks){
        case 1:                     return 0;
        case 2:                     return 1;
        case 3:                     return 2;
    }
    if(blocks < 64){
        if(blocks < 8)              return 3;
        else if(blocks < 16)        return 4;
        else if(blocks < 32)        return 5;
        else                        return 6;
    }
    else if(blocks < 1024){
        if(blocks < 128)            return 7;
        else if(blocks < 256)       return 8;
        else if(blocks < 512)       return 9;
        else                        return 10;
    }
    else                            return 11;
}

uint8_t         freed_bin_get(const tm_blocks_t blocks){
    // Used for getting the bin to return. Makes certain that the size
    // value is >= blocks

    // If it aligns perfectly with power of 2, we can fit it
    switch(blocks){
        case 1:                     return 0;
        case 2:                     return 1;
        case 3:                     return 2;
        case 4:                     return 3;
        case 8:                     return 4;
        case 16:                    return 5;
        case 32:                    return 6;
        case 64:                    return 7;
        case 128:                   return 8;
        case 256:                   return 9;
        case 512:                   return 10;
        case 1024:                  return 11;
    }

    // Else, we only know that the bin above it will fit it
    //      User has to make sure bin != 12, if it does it has to check the size
    return freed_bin(blocks) + 1;
}


inline void     freed_remove(const tm_index_t index){
    // remove the index from the freed array. This doesn't do anything else
    //      It is very important that this is called BEFORE any changes
    //      to the index's size
    assert(!FILLED(index));
#ifdef TM_TESTS  // processor intensive
    /*assert(freed_isin(index));*/
#endif
    if(FREE_PREV(index)){
        // if previous exists, move it's next as index's next
        assert(FREE_NEXT(FREE_PREV(index)) == index);
        FREE_NEXT(FREE_PREV(index)) = FREE_NEXT(index);
    } else{ // free is first element in the bin
        assert(tm_pool.freed[freed_bin(BLOCKS(index))] == index);
        tm_pool.freed[freed_bin(BLOCKS(index))] = FREE_NEXT(index);
    }
    if(FREE_NEXT(index)) FREE_PREV(FREE_NEXT(index)) = FREE_PREV(index);
}


inline void     freed_insert(const tm_index_t index){
    // Insert the index onto the correct freed bin
    //      (inserts at position == 0)
    // Does not do ANY other record keeping (no adding ptrs, blocks, etc)
    uint8_t bin = freed_bin(BLOCKS(index));
    assert(!FILLED(index));
    *free_p(index) = (free_block){.next=tm_pool.freed[bin], .prev=0};
    if(tm_pool.freed[bin]){
        // If a previous index exists, update it's previous value to be index
        FREE_PREV(tm_pool.freed[bin]) = index;
    }
    tm_pool.freed[bin] = index;
}


tm_index_t      freed_get(const tm_blocks_t blocks){
    // Get an index from the freed array of the specified size. The
    //      index settings are automatically set to filled
    tm_index_t index;
    uint8_t bin = freed_bin_get(blocks);
    if(bin == FREED_BINS){  // size is off the binning charts
        index = tm_pool.freed[FREED_BINS-1];
        while(index){
            if(BLOCKS(index) >= blocks) goto found;
            index = FREE_NEXT(index);
        }
        // no need to return here: bin == FREED_BINS
    }
    for(; bin<FREED_BINS; bin++){
        index = tm_pool.freed[bin];
        if(index){
found:
            assert(POINTS(index)); assert(!FILLED(index));
            freed_remove(index);
            FILLED_SET(index);
            // Mark the index as filled. It is already on the indexes list
            tm_pool.filled_blocks += BLOCKS(index);
            tm_pool.freed_blocks -= BLOCKS(index);
            tm_pool.ptrs_filled++;
            tm_pool.ptrs_freed--;
            return index;
        }
    }
    return 0;
}


/*---------------------------------------------------------------------------*/
/*          Index Operations (remove, join, etc)                             */

void index_extend(const tm_index_t index, const tm_blocks_t blocks,
        const bool filled){
    // extend index onto the heap
    assert(!POINTS(index));
    assert(!FILLED(index));
    POINTS_SET(index);
    tm_pool.pointers[index] = (poolptr) {.loc = HEAP, .next = 0};
    HEAP += blocks;
    if(tm_pool.last_index) NEXT(tm_pool.last_index) = index;
    tm_pool.last_index = index;
    if(!tm_pool.first_index) tm_pool.first_index = index;
    if(filled){
        FILLED_SET(index);
        tm_pool.filled_blocks += blocks;
        tm_pool.ptrs_filled++;
    }
    else{
        assert(0);      // not used currently
#if 0
        assert(!FILLED(index));
        tm_pool.freed_blocks += blocks;
        tm_pool.ptrs_freed++;
        freed_insert(index);
#endif
    }
}


void index_remove(const tm_index_t index, const tm_index_t prev_index, bool defrag){
    // Completely remove the index. Used for combining indexes and when defragging
    //      from end (to combine heap).
    //      This function also combines the indexes (NEXT(prev_index) = NEXT(index))
    // Note: NEXT(index) and LOCATION(index) **must not change**. They are used by other code
    //
    //  Update information
    // possibilities:
    //      both are free:              this happens when joining free indexes
    //      prev filled, index free:    this happens for realloc and all the time
    //      prev free, index filled:    this should only happen during defrag
    //      both filled:                this should never happen
    //
    //
    //
    assert(index);
    switch(((FILLED(prev_index) ? 1:0) << 1) + (FILLED(index) ? 1:0)){
        case 0x00:  // merging two free values
            // TODO: this causes failure, find out why
            freed_remove(index);
            tm_pool.ptrs_freed--;  // no change in blocks, both are free
            // if index is last value, freed_blocks will be reduced
            if(!NEXT(index)) tm_pool.freed_blocks -= BLOCKS(index);
            break;
        case 0x02:  // growing prev_index "up"
            freed_remove(index);
            tm_pool.freed_blocks -= BLOCKS(index); tm_pool.ptrs_freed--;
            // grow prev_index, unless index is last value
            if(NEXT(index)) tm_pool.filled_blocks += BLOCKS(index);
            break;
        case 0x03:  // combining two filled indexes, used ONLY in defrag
            assert(defrag);  // defrag is using
            tm_pool.ptrs_filled--;
            break;
        default:
            assert(0);
    }

    if(index == tm_pool.first_index) tm_pool.first_index = NEXT(index);
    // Combine indexes (or heap)
    if(NEXT(index)) {
        NEXT(prev_index) = NEXT(index);
    } else{ // this is the last index, move the heap
        assert(tm_pool.last_index == index);
        tm_pool.last_index = prev_index;
        if(prev_index)  NEXT(prev_index) = 0;
        else            tm_pool.first_index = 0;  // prev_index == 0
        HEAP = LOCATION(index);
    }
    FILLED_CLEAR(index);
    POINTS_CLEAR(index);
    // Check for defragmentation settings
    if(!defrag){
        if(index == tm_pool.defrag_index){
            assert(prev_index == tm_pool.defrag_prev);
            tm_pool.defrag_index = NEXT(index);  // index is gone, defrag should do next index
        } else if(index == tm_pool.defrag_prev){
            tm_pool.defrag_prev = prev_index;  // index is gone, joined with prev_index
        }
    }

}

inline void index_join(tm_index_t index, tm_index_t with_index, int32_t *clocks_left){
    // join index with_index. with_index will be removed
    do{
        if(clocks_left) *clocks_left -= 8;
        assert(!FILLED(with_index));
        assert(LOCATION(index) <= LOCATION(with_index));
        if(!FILLED(index)){
            if(clocks_left) *clocks_left -= FREED_REMOVE_CLOCKS;
            freed_remove(index); // index has to be rebinned, remove before changing size
        }
        // Remove and combine the index
        if(clocks_left) *clocks_left -= INDEX_REMOVE_CLOCKS;
        index_remove(with_index, index, BOOL(clocks_left));
        if(!FILLED(index)) freed_insert(index); // rebin the index
        with_index = NEXT(index);
    }while(!FILLED(with_index));
}

bool index_split(const tm_index_t index, const tm_blocks_t blocks, tm_index_t new_index){
    assert(blocks < BLOCKS(index));
    if(!FILLED(NEXT(index))){
        new_index = NEXT(index);
        // If next index is free, always join it first. This also frees up new_index to
        // use however we want!
        index_join(index, new_index, NULL);
    } else if(new_index){  // an empty index has been given to us
        // pass
    }else{
        new_index = find_index();
        if(!new_index) return false;
    }

    // update new index for freed data
    assert(!POINTS(new_index));
    assert(!FILLED(new_index));
    POINTS_SET(new_index);

    if(FILLED(index)){ // there will be some newly freed data
        tm_pool.freed_blocks += BLOCKS(index) - blocks;
        tm_pool.filled_blocks -= BLOCKS(index) - blocks;
    }

    tm_pool.ptrs_freed++;
    tm_pool.pointers[new_index] = (poolptr) {.loc = LOCATION(index) + blocks,
                                             .next = NEXT(index)};
    NEXT(index) = new_index;

    // mark changes
    freed_insert(new_index);
    if(tm_pool.last_index == index){
        tm_pool.last_index = new_index;
    }
    else{
        assert(NEXT(index));
        assert(NEXT(new_index));
    }
    return true;
}


