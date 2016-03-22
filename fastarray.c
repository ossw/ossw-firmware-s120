
#include "fastarray.h"
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <assert.h>
#include "string.h"
#endif

#define ELEMENT_SIZE(x) (x->elementSize + x->privateFields.iteratorSize)
#define ADDR_HEADER(x, y) (ELEMENT_SIZE(x) * (y - 1))
#define ADDR_ELEMENT(x, y) (ADDR_HEADER(x, y) + x->privateFields.iteratorSize)


static bool write_data_handler(FastArray * arr, uint16_t address, void *buf, uint8_t size) {
	if (address + size > arr->bufferSize) {
		return false;
	}
	memcpy(arr->buffer + address, buf, size);
	return true;
}

static bool read_data_handler(FastArray * arr, uint16_t address, void *buf, uint8_t size) {
	if (address + size > arr->bufferSize) {
		return false;
	}
	memcpy(buf, arr->buffer + address, size);
	return true;
}

static fa_element_iterator readNextIterator(FastArray *arr, fa_element_iterator it) {
	fa_element_iterator result = NULL_ITERATOR;
	if (!read_data_handler(arr, ADDR_HEADER(arr, it), &result, arr->privateFields.iteratorSize)) {
#ifdef _WIN32
		assert(false);
#endif
		return NULL_ITERATOR;
	}
	return result;
}

static bool writeNextIterator(FastArray *arr, fa_element_iterator it, fa_element_iterator nextIt) {
	return write_data_handler(arr, ADDR_HEADER(arr, it), &nextIt, arr->privateFields.iteratorSize);
}

static bool readData(FastArray *arr, fa_element_iterator it, void* data, uint16_t dataSize) {
	dataSize = dataSize < arr->elementSize ? dataSize : arr->elementSize;
	return read_data_handler(arr, ADDR_ELEMENT(arr, it), data, arr->elementSize);
}

static bool writeData(FastArray *arr, fa_element_iterator it, void* data, uint16_t dataSize) {
	dataSize = dataSize < arr->elementSize ? dataSize : arr->elementSize;
	return write_data_handler(arr, ADDR_ELEMENT(arr, it), data, arr->elementSize);
}



fa_element_iterator fastArrayFirstIterator(FastArray * arr) {
	if (arr == 0) {
		return NULL_ITERATOR;
	}
	return arr->privateFields.filledListHeader.firstIterator;
}

fa_element_iterator fastArrayNextIterator(FastArray * arr, fa_element_iterator current) {
	if (arr == 0) {
		return NULL_ITERATOR;
	}
	return readNextIterator(arr, current);
}

fa_element_iterator fastArrayLastIterator(FastArray * arr) {
	if (arr == 0) {
		return NULL_ITERATOR;
	}
	return arr->privateFields.filledListHeader.lastIterator;
}

fa_element_iterator fastArrayGetAtIterator(FastArray * arr, int index) {
	if (arr == 0) {
		return NULL_ITERATOR;
	}
	fa_element_iterator result = arr->privateFields.filledListHeader.firstIterator;
	for (int i = 1; i <= index; i++) {
		result = readNextIterator(arr, result);
	}
	return result;
}

bool FastArrayInit(FastArray * arr) {
	bool result = false;
	arr->privateFields.freedListHeader.size = 0;
	arr->privateFields.freedListHeader.firstIterator = 1;
	arr->privateFields.freedListHeader.lastIterator = NULL_ITERATOR;

	arr->privateFields.filledListHeader.size = 0;
	arr->privateFields.filledListHeader.firstIterator = NULL_ITERATOR;
	arr->privateFields.filledListHeader.lastIterator = NULL_ITERATOR;

	arr->privateFields.maxArraySize = 0;
	arr->privateFields.iteratorSize = 1;

	fa_element_iterator nextIt = 1;
	fa_element_iterator currentIt = 1;

	while (((arr->privateFields.freedListHeader.size + 1) * ELEMENT_SIZE(arr)) <= arr->bufferSize) {
		currentIt = nextIt;
		nextIt++;
		writeNextIterator(arr, currentIt, nextIt);
		arr->privateFields.freedListHeader.lastIterator = currentIt;
		arr->privateFields.freedListHeader.size++;
		if (nextIt + 1 == NULL_ITERATOR) {
			break;
		}
		if ((nextIt + 1) >= 255) {
			break;
		}
		arr->privateFields.maxArraySize = nextIt - 1;
	}
	fa_element_iterator nullIt = NULL_ITERATOR;
	writeNextIterator(arr, arr->privateFields.freedListHeader.lastIterator, nullIt);
	result = true;
	return result;
}


bool fastArrayAdd(FastArray * arr, void * data, uint16_t dataSize) {

	if (arr == 0) {
		return false;
	}

	if (arr->rotateBuffer) {
		if (arr->privateFields.maxArraySize <= arr->privateFields.filledListHeader.size + 1) {
			fastArrayRemoveAt(arr, 0);
		}
	}

	bool result = false;

	if (arr->privateFields.freedListHeader.firstIterator != NULL_ITERATOR && 
		arr->privateFields.filledListHeader.size <= arr->privateFields.maxArraySize &&
		arr->privateFields.freedListHeader.size > 0) {

		if (arr->privateFields.filledListHeader.firstIterator == NULL_ITERATOR) {
			arr->privateFields.filledListHeader.firstIterator = arr->privateFields.freedListHeader.firstIterator;
			arr->privateFields.filledListHeader.lastIterator = arr->privateFields.freedListHeader.firstIterator;
		}
		else {
			if (writeNextIterator(arr, arr->privateFields.filledListHeader.lastIterator, arr->privateFields.freedListHeader.firstIterator)) {
				arr->privateFields.filledListHeader.lastIterator = arr->privateFields.freedListHeader.firstIterator;
			}
			else {
				return false;
			}
		}

		fa_element_iterator nextFreeIt = readNextIterator(arr, arr->privateFields.freedListHeader.firstIterator);
		if (nextFreeIt != NULL_ITERATOR) {
			if (writeData(arr, arr->privateFields.filledListHeader.lastIterator, data, dataSize)) {
				arr->privateFields.freedListHeader.firstIterator = nextFreeIt;
				fa_element_iterator nullIt = NULL_ITERATOR;
				writeNextIterator(arr, arr->privateFields.filledListHeader.lastIterator, nullIt);
				arr->privateFields.filledListHeader.size++;
				arr->privateFields.freedListHeader.size--;
				result = true;
			}
		}

	}
	return result;

}

bool fastArrayPushBack(FastArray * arr, void *data, uint16_t dataSize) {
	return fastArrayAdd(arr, data, dataSize);
}

void fastArrayRemoveAt(FastArray * arr, int index) {
	if (arr == 0) {
		return;
	}
	if (index >= arr->privateFields.filledListHeader.size) {
#ifdef _WIN32
		assert(false);
#endif
		return;
	}
	fa_element_iterator iterator = arr->privateFields.filledListHeader.firstIterator;
	fa_element_iterator prevIterator = NULL_ITERATOR;
	fa_element_iterator nextIterator = NULL_ITERATOR;
	fa_element_iterator removeIterator = NULL_ITERATOR;
	fa_element_iterator nullIt = NULL_ITERATOR;
	if (index == 0) {
		removeIterator = iterator;
		nextIterator = readNextIterator(arr, removeIterator);
		writeNextIterator(arr, removeIterator, nullIt);
		writeNextIterator(arr, arr->privateFields.freedListHeader.lastIterator, removeIterator);
		arr->privateFields.filledListHeader.firstIterator = nextIterator;
		arr->privateFields.freedListHeader.lastIterator = removeIterator;
	}
	else {
		for (int i = 1; i <= index; i++) {
			iterator = readNextIterator(arr, iterator);
			if (i == index - 1) {
				prevIterator = iterator;
			}
			else if (i == index) {
				removeIterator = iterator;
			}
		}
		nextIterator = readNextIterator(arr, removeIterator);
		writeNextIterator(arr, prevIterator, nextIterator);
		writeNextIterator(arr, arr->privateFields.freedListHeader.lastIterator, removeIterator);
		writeNextIterator(arr, removeIterator, nullIt);
#ifdef _WIN32
		uint8_t buf[255];
		memset(buf, 0, sizeof(buf));
		writeData(arr, removeIterator, buf, sizeof(buf));
#endif
		arr->privateFields.freedListHeader.lastIterator = removeIterator;
		if (arr->privateFields.filledListHeader.lastIterator == removeIterator) {
			arr->privateFields.filledListHeader.lastIterator = prevIterator;
		}
	}
	arr->privateFields.freedListHeader.size++;
	arr->privateFields.filledListHeader.size--;
}

void fastArrayRemove(FastArray * arr, fa_element_iterator it) {
	if (arr == 0) {
		return;
	}
	fa_element_iterator curr = arr->privateFields.filledListHeader.firstIterator;
	if (curr == it) {
		fastArrayRemoveAt(arr, 0);
	}
	else {
		for (int i = 1; i < arr->privateFields.filledListHeader.size; i++) {
			curr = readNextIterator(arr, curr);
			if (curr == it) {
				fastArrayRemoveAt(arr, i);
				break;
			}
		}
	}
}

bool fastArrayFront(FastArray * arr, void* data, uint16_t dataSize) {
	if (arr == 0) {
		return false;
	}
	if (arr->privateFields.filledListHeader.firstIterator != NULL_ITERATOR) {
		return readData(arr, arr->privateFields.filledListHeader.firstIterator, data, dataSize);
	}
#ifdef _WIN32
	assert(false);
#endif
	return false;
}

bool fastArrayBack(FastArray * arr, void* data, uint16_t dataSize) {
	if (arr == 0) {
		return NULL_ITERATOR;
	}
	if (arr->privateFields.filledListHeader.lastIterator != NULL_ITERATOR) {
		return readData(arr, arr->privateFields.filledListHeader.lastIterator, data, dataSize);
	}
	return false;
}

bool fastArrayGet(FastArray * arr, fa_element_iterator it, void* data, uint16_t dataSize) {
	if (arr == 0) {
		return false;
	}
	return readData(arr, it, data, dataSize);
}

bool fastArrayGetAt(FastArray * arr, int index, void* data, uint16_t dataSize) {
	if (arr == 0) {
		return false;
	}
	if (index >= arr->privateFields.filledListHeader.size) {
#ifdef _WIN32
		assert(false);
#endif
		return false;
	}
	fa_element_iterator it = arr->privateFields.filledListHeader.firstIterator;
	for (int i = 1; i <= index; i++) {
		it = readNextIterator(arr, it);
	}

	return readData(arr, it, data, dataSize);
}

bool fastArraySet(FastArray * arr, fa_element_iterator it, void* data, uint16_t dataSize) {
	if (arr == 0) {
		return false;
	}
	return writeData(arr, it, data, dataSize);
}

bool fastArraySetAt(FastArray * arr, int index, void* data, uint16_t dataSize) {
	if (arr == 0) {
		return false;
	}
	if (index >= arr->privateFields.filledListHeader.size) {
#ifdef _WIN32
		assert(false);
#endif
		return false;
	}
	fa_element_iterator it = arr->privateFields.filledListHeader.firstIterator;
	for (int i = 1; i <= index; i++) {
		it = readNextIterator(arr, it);
	}

	return writeData(arr, it, data, dataSize);
}


bool fastArrayPopFront(FastArray * arr, void * data, uint16_t dataSize) {
	bool result = fastArrayFront(arr, data, dataSize);
	if (result) {
		fastArrayRemoveAt(arr, 0);
	}
	return result;
}

int fastArraySize(FastArray * arr) {
	if (arr == 0) {
		return false;
	}
	return arr->privateFields.filledListHeader.size;
}

int fastArrayFreeIndexes(FastArray * arr) {
	if (arr == 0) {
		return false;
	}
	return arr->privateFields.freedListHeader.size;
}

void fastArrayClear(FastArray * arr) {
	if (arr == 0) {
		return;
	}
	while (arr->privateFields.filledListHeader.size > 0) {
			fastArrayRemoveAt(arr, 0);
	}
	
}

