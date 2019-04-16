#ifndef _CDATA_OS_ADAPTER_H_
#define _CDATA_OS_ADAPTER_H_

#include <stdlib.h>
#include <pthread.h>

typedef pthread_mutex_t OSMutex_t;

#ifdef __cplusplus
extern "C" {
#endif

void *OS_Malloc(size_t size);
void  OS_Free(void* p_mem);


OSMutex_t *OS_CreateMutex();
void  OS_DestroyMutex(OSMutex_t* p_mutex);

int OS_MutexLock(OSMutex_t* p_mutex);
int OS_MutexUnlock(OSMutex_t* p_mutex);

#ifdef __cplusplus
}
#endif

#endif //_CDATA_OS_ADAPTER_H_

