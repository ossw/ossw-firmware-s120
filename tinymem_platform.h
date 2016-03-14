#ifndef __tinymem_platform_h
#define __tinymem_platform_h
/*---------------------------------------------------------------------------*/
/**
 * \file            Standard linux platform settings for tinymem
 */

/*---------------------------------------------------------------------------*/
/**
 * \brief           Whether to use the stdlib.
 *                  If not defined, you must define all of the functions and
 *                  declarations specified at the top of tinymem.h
 */
#define TM_USE_STDLIB
//#define TM_PRINT            // comment out to disable printing
//#define TM_TESTS            // comment out to disable compilijng tests
//#define TM_TOOLS            // have access to diagnostic tools

/*---------------------------------------------------------------------------*/
/**
 * \brief           Time module
 *                  This file must include the standard c time module,
 *                  or it must define the following standard C symbols:
 *                      clock()         -- returns total clock cycles
 *                      CLOCKS_PER_SEC  -- macro for number of clocks/second
 *                          returned by clock()
 */
#include "time.h"
#define CPU_CLOCKS_PER_SEC      (16000000UL)     // Defined at 700MHz for general

/*---------------------------------------------------------------------------*/
/**
 * \brief           Max time allowed per run of the thread (in microseconds)
*/
//#define TM_THREAD_TIME_US      5

/*---------------------------------------------------------------------------*/
/**
 * \brief           Maximum number of pointers that can be allocated
 *
 *                  This is the maximum number of pointers that a system can
 *                  allocate. For instance, if TM_POOL_INDEXES == 3 and you
 *                  allocated an integer, a 1000 character array and
 *                  a 30 byte struct, then you would be unable to allocate
 *                  any more data
 *
 *                  On memory constrained systems this number should be low,
 *                  probably either 16 or 32
 *
 *                  This value is highly dependent on implementation details,
 *                  however, it is advisable to leave a bunch of headroom as
 *                  there need to be pointers available for free data, or else
 *                  the system will start the defragmenter when it runs out
 *                  of pointers
 *
 *                  This value is used to determine what type to use for
 *                  tm_index_t.
 *                      size    | type used
 *                      --------|------------
 *                      < 256   | uint8_t
 *                      < 65536 | uint16_t
 *                      >=65536 | INVALID
 *
 *                  Note: currently this value must be divisible by
 *                      sizeof(int) * 8
 *
 *                  Each index uses sizeof(tm_index_t) * 2 bytes of data
 */
#define TM_POOL_INDEXES        (64)

/*---------------------------------------------------------------------------*/
/**
 * \brief           The block size in bytes
 *                  These conditions must be met:
 *                      TM_BLOCK_SIZE >= 2 * sizeof(tm_index_t)
 *
 *                  The total amount of possible memory is determined by the
 *                  block type and the index type
 *
 *                  The maximum amount of memory is:
 *                      2 ^ sizeof(tm_blocks_t) * sizeof(TM_BLOCK_TYPE) - sizeof(TM_BLOCK_TYPE)
 *
 *                  Note: see TM_POOL_SIZE for how to change tm_blocks_t
 *
 *                  If this value is unset, it will automatically be the size of two tm_index_t
 *                  values (see TM_POOL_INDEXES for how to set this size)
 */
#define TM_BLOCK_SIZE           (2)

/*---------------------------------------------------------------------------*/
/**
 * \brief           size of memory pool in bytes
 *                  This is the maximum amount of memory that can be
 *                  allocated in a memory pool
 *
 *                  The maximum value is ((2^16) * 4 - sizeof(TM_BLOCK_TYPE))
 *
 *                  tm_blocks_t is determined by this value:
 *                      TM_POOL_SIZE/TM_BLOCK_SIZE   | type used
 *                      -----------------------------|------------
 *                      < 256                        | uint8_t
 *                      < 65536                      | uint16_t
 *                      >=65536                      | INVALID
 */
#define TM_POOL_SIZE            ((128 + 1)*TM_BLOCK_SIZE - TM_BLOCK_SIZE)

/*---------------------------------------------------------------------------*/
/**
 * \brief           Percentage of fragmentation at which tm_thread will
 *                  automatically defrag
 *
 *                  Note that these won't actually trigger a defrag unless
 *                  the system is at least TM_DEFRAG_MIN fragmentated
 */

#define TM_DEFRAG_SIZE          85      // trigger at this % heap size
#define TM_DEFRAG_INDEXES       90      // trigger at this % used indexes
#define TM_DEFRAG_MIN           2       // Minimum fragmentation that has to exist to operate

#define TM_H_ATTPACKPRE         __attribute__ ((__packed__))
#define TM_H_ATTPACKSUF

#endif
