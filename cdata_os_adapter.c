#include "cdata_os_adapter.h"

void *OS_Malloc(size_t size)
{
    return malloc(size);
}

void  OS_Free(void* p_mem)
{
    if (p_mem != NULL)
    {
        free(p_mem);
    }
}


OSMutex_t *OS_CreateMutex()
{
    pthread_mutex_t *p_mutex = (pthread_mutex_t* )OS_Malloc(sizeof(pthread_mutex_t));
    if (p_mutex == NULL)
    {
        return NULL;
    }

    pthread_mutex_init(p_mutex, NULL);

    return p_mutex;
}

void  OS_DestroyMutex(OSMutex_t* p_mutex)
{
    if (p_mutex != NULL)
    {
        pthread_mutex_destroy(p_mutex);
        OS_Free(p_mutex);
    }
}

int OS_MutexLock(OSMutex_t* p_mutex)
{
    if (p_mutex != NULL)
    {
        return pthread_mutex_lock(p_mutex);
    }

    return -1;
}

int OS_MutexUnlock(OSMutex_t* p_mutex)
{
    if (p_mutex != NULL)
    {
        return pthread_mutex_unlock(p_mutex);
    }

    return -1;
}



