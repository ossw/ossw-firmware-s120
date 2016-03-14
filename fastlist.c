#include "fastlist.h"
#include <stdlib.h>



typedef struct {
	FastListItem element;
	tm_index_t next;
} Elements;

typedef struct {
	uint16_t size;
	tm_index_t first;
	tm_index_t last;
} FastList;

static FastList * fastList(FastListHandler listIndex) {
	return tm_void_p(listIndex);
}

static Elements * elements(FastListHandler elementIndex) {
	return tm_void_p(elementIndex);
}

tm_index_t FastListCreate(void) {
	tm_index_t result = tm_alloc(sizeof(FastList));
	if (result != 0) {
		FastList * fl = fastList(result);
		fl->size = 0;
		fl->first = 0;
		fl->last = 0;
	}
	return result;
}

void FastListDestroy(FastListHandler list) {
	if (list == 0) {
		return;
	}
	FastList * fl = fastList(list);
	tm_index_t next = fl->first;
	tm_index_t current = 0;
	while (next != 0) {
		current = next;
		next = elements(next)->next;
		tm_free(current);
	}
	tm_free(list);
}

void fastListClear(FastListHandler list) {
	if (list == 0) {
		return;
	}
	FastList * fl = fastList(list);
	if (fl->size > 0) {
		tm_index_t next = fl->first;
		tm_index_t current = 0;
		while (next != 0) {
			current = next;
			next = elements(next)->next;
			tm_free(current);
		}
		fl->first = 0;
		fl->last = 0;
		fl->size = 0;
	}
}

bool fastListAdd(FastListHandler list, FastListItem element) {
	if (list == 0) {
		return false;
	}
	FastList *fl = fastList(list);
	if (fl->size == 65535) {
			return false;
	}
	fl->size++;
	if (fl->first == 0) {
		fl->first = tm_alloc(sizeof(Elements));
		if (fl->first == 0) {
			return false;
		}
		elements(fl->first)->element = element;
		elements(fl->first)->next = 0;
		fl->last = fl->first;
	}
	else {
		elements(fl->last)->next = tm_alloc(sizeof(Elements));
		if (elements(fl->last)->next == 0) {
				return false;
		}
		fl->last = elements(fl->last)->next;
		elements(fl->last)->element = element;
		elements(fl->last)->next = 0;
	}
	return true;
}

bool fastListPushBack(FastListHandler list, FastListItem element) {
	return fastListAdd(list, element);
}

void fastListRemove(FastListHandler list, uint16_t index) {
	if (list == 0) {
		return;
	}
	tm_index_t el = 0;
	tm_index_t toRemove = 0;
	int i = 0;
	FastList * fl = fastList(list);
	if (index >= fl->size) {
		return;
	}

	if (index == 0) {
		toRemove = fl->first;
		fl->first = elements(toRemove)->next;
		tm_free(toRemove);
	}
	else {

		el = fl->first;

		for (i = 1; i < index; i++) {
			el = elements(el)->next;
		}

		toRemove = elements(el)->next;
		elements(el)->next = elements(toRemove)->next;
		tm_free(toRemove);
		if (elements(el)->next == 0) {
			fl->last = el;
		}
	}
	fl->size--;

}

int fastListSize(FastListHandler list) {
	if (list == 0) {
		return 0;
	}
	return fastList(list)->size;
}

FastListItem fastListFront(FastListHandler list) {
	if (list == 0) {
		return FastListItemNull;
	}
	if (fastList(list)->first != 0) {
		return elements(fastList(list)->first)->element;
	}
	return FastListItemNull;
}

FastListItem fastListBack(FastListHandler list) {
	if (list == 0) {
		return FastListItemNull;
	}
	if (fastList(list)->last != 0) {
		return elements(fastList(list)->last)->element;
	}
	return FastListItemNull;
}


FastListItem fastListPopFront(FastListHandler list) {
	if (list == 0) {
		return FastListItemNull;
	}
	FastListItem result = FastListItemNull;
	tm_index_t toRemove = 0;
	FastList * fl = fastList(list);
	if (fl->first != 0) {
		result = elements(fl->first)->element;
		toRemove = fl->first;
		fl->first = elements(toRemove)->next;
		tm_free(toRemove);
		fl->size--;
	}

	return result;
}

FastListItem fastListGet(FastListHandler list, uint16_t index) {
	if (list == 0) {
		return FastListItemNull;
	}
	tm_index_t el = 0;
	FastListItem result = FastListItemNull;
	int i = 0;

	if (index >= fastList(list)->size) {
		return result;
	}

	el = fastList(list)->first;
	for (i = 1; i <= index; i++) {
		el = elements(el)->next;
	}

	return elements(el)->element;
}

double fastListAvg(FastListHandler list) {
	if (list == 0) {
		return 0.0;
	}
	int i = 0;
	double sum = 0.0;
	int size = fastListSize(list);
	for (i = 0; i < size; i++) {
		sum += fastListGet(list, i);
	}
	if (size > 0) {
		return sum / (double)size;
	}
	return 0.0;
}

