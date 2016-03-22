
#include "fastarrayint.h"


bool FastIntArrayInit(FastArray * arr) {
	return FastArrayInit(arr);
}

fa_element_iterator fastIntArrayLastIterator(FastArray * arr) {
	return fastArrayLastIterator(arr);
}

fa_element_iterator fastIntArrayFirstIterator(FastArray * arr) {
	return fastArrayFirstIterator(arr);
}

fa_element_iterator fastIntArrayNextIterator(FastArray * arr, fa_element_iterator current) {
	return fastArrayNextIterator(arr, current);
}

fa_element_iterator fastIntArrayGetAtIterator(FastArray * arr, int index) {
	return fastArrayGetAtIterator(arr, index);
}

bool fastIntArrayAdd(FastArray * arr, fa_element_type data) {
	return fastArrayAdd(arr, &data, sizeof(fa_element_type));
}

bool fastIntArrayPushBack(FastArray * arr, fa_element_type data) {
	return fastArrayPushBack(arr, &data, sizeof(fa_element_type));
}

void fastIntArrayRemoveAt(FastArray * arr, int index) {
	fastArrayRemoveAt(arr, index);
}

void fastIntArrayRemove(FastArray * arr, fa_element_iterator it) {
	fastArrayRemove(arr, it);
}

fa_element_type fastIntArrayFront(FastArray * arr) {
	fa_element_type result = 0;
	fastArrayFront(arr, &result, sizeof(fa_element_type));
	return result;
}

fa_element_type fastIntArrayBack(FastArray * arr) {
	fa_element_type result = 0;
	fastArrayBack(arr, &result, sizeof(fa_element_type));
	return result;
}

fa_element_type fastIntArrayGet(FastArray * arr, fa_element_iterator it) {
	fa_element_type result = 0;
	fastArrayGet(arr, it, &result, sizeof(fa_element_type));
	return result;
}

fa_element_type fastIntArrayGetAt(FastArray * arr, int index) {
	fa_element_type result = 0;
	fastArrayGetAt(arr, index, &result, sizeof(fa_element_type));
	return result;
}

bool fastIntArraySet(FastArray * arr, fa_element_iterator it, fa_element_type data) {
	return fastArraySet(arr, it, &data, sizeof(fa_element_type));
}

bool fastIntArraySetAt(FastArray * arr, int index, fa_element_type data) {
	return fastArraySetAt(arr, index, &data, sizeof(fa_element_type));
}

fa_element_type fastIntArrayPopFront(FastArray * arr) {
	fa_element_type result = 0;
	fastArrayPopFront(arr, &result, sizeof(fa_element_type));
	return result;
}

int fastIntArraySize(FastArray * arr) {
	return fastArraySize(arr);
}

int fastIntArrayFreeIndexes(FastArray * arr) {
	return fastArrayFreeIndexes(arr);
}

void fastIntArrayClear(FastArray * arr) {
	fastArrayClear(arr);
}


double fastIntArrayAvg(FastArray * arr) {
	double result = 0.0;
	if (arr == 0) {
		return result;
	}
	double sum = 0.0;
	int size = 0;
	fa_element_iterator it = fastIntArrayFirstIterator(arr);
	while (it != NULL_ITERATOR) {
		sum += fastIntArrayGet(arr, it);
		it = fastIntArrayNextIterator(arr, it);
		size++;
		
	}

	if (size > 0) {
		result = sum / (double)size;
	}
	return result;
}
