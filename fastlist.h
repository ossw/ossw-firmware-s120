#ifndef DOUBLES_LIST_H
#define DOUBLES_LIST_H


#ifdef __cplusplus
extern "C" {
#endif

#include "tinymem.h"

#define FastListItem int
#define FastListItemNull 0

typedef tm_index_t FastListHandler;

FastListHandler FastListCreate(void);
void FastListDestroy(FastListHandler list);
bool fastListAdd(FastListHandler list, FastListItem element);
void fastListRemove(FastListHandler list, uint16_t index);
int fastListSize(FastListHandler list);
FastListItem fastListFront(FastListHandler list);
FastListItem fastListBack(FastListHandler list);
bool fastListPushBack(FastListHandler list, FastListItem element);
FastListItem fastListPopFront(FastListHandler list);
FastListItem fastListGet(FastListHandler list, uint16_t index);
void fastListClear(FastListHandler list);

double fastListAvg(FastListHandler list);

#ifdef __cplusplus
}
#endif

#endif // DOUBLES_LIST_H
