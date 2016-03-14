#ifndef __tinymem_h
#define __tinymem_h

#ifdef __cplusplus
extern "C" {
#endif


#include "tinymem_platform.h"

#ifdef TM_USE_STDLIB
/*       Std Library       Definitions needed                                */
#include <stdlib.h>     // exit. For tests: srand, rand
#include <stdio.h>      // printf must be defined (can be a no-op)
#include <stdint.h>     // uint8_t, uint32_t, etc.
#include <stdbool.h>    // bool, true and false
#include <string.h>     // memmove
#include <limits.h>     // UINT_MAX, UINT16_MAX, UINT32_MAX and UINT64_MAX
#include <time.h>       // clock, clock_t
#include <assert.h>     // assert
#endif




/*---------------------------------------------------------------------------*/
/**
 * \brief           status bitcodes
 */
#define TM_DEFRAG_FULL      (1<<0)  // a full defrag has been requested
#define TM_DEFRAG_FAST      (1<<1)  // a fast defrag has been requested
#define TM_DEFRAG_FULL_IP   (1<<2)  // A defrag is in progress
#define TM_DEFRAG_FAST_IP   (1<<3)  // A defrag is in progress
#define TM_MOVING           (1<<4)  // the memory manager is currently moving a block
#define TM_DEFRAG_FULL_DONE (1<<5)  // this will be set after a full defrag has happend
#define TM_DEFRAG_FAST_DONE (1<<6)  // this will be set after a fast defrag has happened.
#define TM_ERROR            (1<<7)  // a memory manager internal error occurred
#define TM_DEFRAG_IP        (TM_DEFRAG_FULL_IP | TM_DEFRAG_FAST_IP)             // defrag is in progress
#define TM_ANY_DEFRAG       (TM_DEFRAG_FULL | TM_DEFRAG_FAST | TM_DEFRAG_IP)    // some defrag has been requested


#if     (TM_POOL_INDEXES < 256)
typedef uint8_t         tm_index_t;
#define TM_INDEX_SIZE   1
#elif   (TM_POOL_INDEXES < 65536)
typedef uint16_t        tm_index_t;
#define TM_INDEX_SIZE   2
#else
#error  TM_POOL_INDEXES too large
#endif

#if     (TM_POOL_SIZE < 65536)
typedef uint16_t        tm_size_t;
#else
typedef uint32_t        tm_size_t;
#endif

/*---------------------------------------------------------------------------*/
/**
 * \brief               Completely resets the internal pool.
 *                      ALL DATA WILL BE LOST
 */
void         				tm_reset(void);

/*---------------------------------------------------------------------------*/
/**
 * \brief           Get the sizeof data at index in bytes
 * \return tm_size_t  the sizeof the data pointed to by index
 */
tm_size_t    				tm_sizeof(const tm_index_t index);

/*---------------------------------------------------------------------------*/
/**
 * \brief           cast a void pointer from index
 *
 *                  The value that tm_alloc returns is an INDEX, not a pointer.
 *                  Do NOT use it as a pointer!
 *
 *                  To get a pointer, use this function as:
 *                      void *ptr = tm_void_p(index)
 *
 *                  There are also several casts below, so if you have an array
 *                  of 32 bit integers use:
 *                      uint32_t *data = tm_uint32_p(index)
 *
 *                  Do NOT store the pointer, as the location of the data can
 *                  change once tm_thread() is called.
 *
 *                  It is recommended to ONLY assign pointers to local
 *                  function variables and keep indexes globally.
 *
 * \param index     tm_index_t to get pointer to
 * \return          void* pointer to actual data
 */
void*        				tm_void_p(const tm_index_t index);

/*---------------------------------------------------------------------------*/
/**
 * \brief           allocate memory from pool
 *
 * \param size      size of pointer to allocate
 * \return          tm_index_t corresponding to memory location
 *                  On error or if not enough memory, return value == 0
 */
tm_index_t          tm_alloc(tm_size_t size);

/*---------------------------------------------------------------------------*/
/**
 * \brief           changes the size of memory in the pool
 *                  See standard documentation on realloc for more info
 *
 * \param index     tm_index_t to realloc
 *                  If 0: acts as tm_alloc(size)
 * \param size      new requested size of index
 *                  If 0: acts as tm_free(index)
 * \return          index with new size of memory.
 *                  If this index has changed, the previous index is been freed
 *                  If return value == 0, then no change has been done
 *                  (or index has been freed if size=0)
 */
tm_index_t          tm_realloc(tm_index_t index, tm_size_t size);

/*---------------------------------------------------------------------------*/
/**
 * \brief           free allocated memory from pool
 *
 * \param index     tm_index_t to free
 */
void                tm_free(const tm_index_t index);

/*---------------------------------------------------------------------------*/
/**
 * \brief           return whether the index is valid (can contain data)
 *
 * \param index     tm_index_t to potential data
 * \return bool     true if valid, false otherwise
 */
bool            		tm_valid(const tm_index_t index);

/*---------------------------------------------------------------------------*/
/**
 * \brief           return whether the index is valid (can contain data)
 *                  and the tm_sizeof(index) == size
 *
 *                  It is a common operation to do:
 *                      assert(tm_check(myindex, sizeof(my_data_type)));
 *                  This serves as a kind of (rudimentary) type checking
 *                  because types are lost in this library (but size is kept)
 *
 * \param index     tm_index_t to possibly valid data
 * \return bool     true if valid and size matches, false otherwise
 */
bool     						tm_check(const tm_index_t index, const tm_size_t size);

uint16_t tm_free_pointers(void);
uint16_t tm_free_blocks(void);


/*---------------------------------------------------------------------------*/
/**
 * \brief           Various data type casts
 */
#define tm_char_p(index)        ((char *)tm_void_p(index))
#define tm_int8_p(index)        ((int8_t *)tm_void_p(index))
#define tm_uint8_p(index)       ((uint8_t *)tm_void_p(index))
#define tm_int16_p(index)       ((int16_t *)tm_void_p(index))
#define tm_uint16_p(index)      ((uint16_t *)tm_void_p(index))
#define tm_int32_p(index)       ((int32_t *)tm_void_p(index))
#define tm_uint32_p(index)      ((uint32_t *)tm_void_p(index))


#ifdef __cplusplus
}
#endif

#endif
/** @} */
