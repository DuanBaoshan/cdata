#ifndef _CDATA_OS_ADAPTER_H_
#define _CDATA_OS_ADAPTER_H_

#include <stdlib.h>
#include "cdata_types.h"

typedef void* OSMutex_t;
typedef void* OSCond_t;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _RELEASE_VERSION_
void *OS_Malloc(size_t size);
void  OS_Free(void* p_mem);
#else
#define OS_Free free
#define OS_Malloc malloc
#endif

OSMutex_t OS_MutexCreate();
void  OS_MutexDestroy(OSMutex_t mutex);

int OS_MutexLock(OSMutex_t mutex);
int OS_MutexUnlock(OSMutex_t mutex);


OSCond_t OS_CondCreate();
int OS_CondDestroy(OSCond_t cond);

int OS_CondWait(OSCond_t cond);
int OS_CondTimedWait(OSCond_t cond, CdataTime_t timeoutMs);

int OS_CondLock(OSCond_t cond);
int OS_CondUnlock(OSCond_t cond);

int OS_CondSignal(OSCond_t cond);
int OS_CondBroadcast(OSCond_t cond);

#ifdef __cplusplus
}
#endif

#endif //_CDATA_OS_ADAPTER_H_

