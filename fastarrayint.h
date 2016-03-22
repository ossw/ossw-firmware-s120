#ifndef FAST_ARRAY_INT_H
#define FAST_ARRAY_INT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "fastarray.h"

typedef int fa_element_type;

bool FastIntArrayInit(FastArray * arr);

fa_element_iterator fastIntArrayLastIterator(FastArray * arr);
fa_element_iterator fastIntArrayFirstIterator(FastArray * arr);
fa_element_iterator fastIntArrayNextIterator(FastArray * arr, fa_element_iterator current);
fa_element_iterator fastIntArrayGetAtIterator(FastArray * arr, int index);
bool fastIntArrayAdd(FastArray * arr, fa_element_type data);
bool fastIntArrayPushBack(FastArray * arr, fa_element_type data);
void fastIntArrayRemoveAt(FastArray * arr, int index);
void fastIntArrayRemove(FastArray * arr, fa_element_iterator it);
fa_element_type fastIntArrayFront(FastArray * arr);
fa_element_type fastIntArrayBack(FastArray * arr);
fa_element_type fastIntArrayGet(FastArray * arr, fa_element_iterator it);
fa_element_type fastIntArrayGetAt(FastArray * arr, int index);
bool fastIntArraySet(FastArray * arr, fa_element_iterator it, fa_element_type data);
bool fastIntArraySetAt(FastArray * arr, int index, fa_element_type data);
fa_element_type fastIntArrayPopFront(FastArray * arr);
int fastIntArraySize(FastArray * arr);
int fastIntArrayFreeIndexes(FastArray * arr);
void fastIntArrayClear(FastArray * arr);

double fastIntArrayAvg(FastArray * arr);

#ifdef __cplusplus
	}
#endif
#endif // DOUBLES_LIST_H
