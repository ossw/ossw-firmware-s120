#ifndef FAST_ARRAY_H
#define FAST_ARRAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stdbool.h"

typedef uint8_t fa_element_iterator;
#define NULL_ITERATOR (0)

typedef struct {
	uint8_t size;
	fa_element_iterator firstIterator;
	fa_element_iterator lastIterator;
} FastArrayHeader;

typedef struct {
	uint8_t maxArraySize;
	uint8_t iteratorSize;  //1 or 2 or more
	FastArrayHeader freedListHeader;
	FastArrayHeader filledListHeader;
} FastArrayPrivateFields;

typedef struct {
	uint8_t *buffer;
	uint16_t bufferSize;
	uint8_t elementSize;
	bool rotateBuffer;
	FastArrayPrivateFields privateFields;
} FastArray;




bool FastArrayInit(FastArray * fastArray);


fa_element_iterator fastArrayLastIterator(FastArray * arr);
fa_element_iterator fastArrayFirstIterator(FastArray * arr);
fa_element_iterator fastArrayNextIterator(FastArray * arr, fa_element_iterator current);
fa_element_iterator fastArrayGetAtIterator(FastArray * arr, int index);
bool fastArrayAdd(FastArray * arr, void * data, uint16_t dataSize);
bool fastArrayPushBack(FastArray * arr, void *data, uint16_t dataSize);
void fastArrayRemoveAt(FastArray * arr, int index);
void fastArrayRemove(FastArray * arr, fa_element_iterator it);
bool fastArrayFront(FastArray * arr, void* data, uint16_t dataSize);
bool fastArrayBack(FastArray * arr, void* data, uint16_t dataSize);
bool fastArrayGet(FastArray * arr, fa_element_iterator it, void* data, uint16_t dataSize);
bool fastArrayGetAt(FastArray * arr, int index, void* data, uint16_t dataSize);
bool fastArraySet(FastArray * arr, fa_element_iterator it, void* data, uint16_t dataSize);
bool fastArraySetAt(FastArray * arr, int index, void* data, uint16_t dataSize);
bool fastArrayPopFront(FastArray * arr, void * data, uint16_t dataSize);
int fastArraySize(FastArray * arr);
int fastArrayFreeIndexes(FastArray * arr);
void fastArrayClear(FastArray * arr);



#ifdef __cplusplus
	}
#endif
#endif // DOUBLES_LIST_H
