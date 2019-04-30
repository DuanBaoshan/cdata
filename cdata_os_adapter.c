#include <errno.h>
#include <pthread.h>

#include "cdata_types.h"
#include "cdata_os_adapter.h"

#ifndef _DEBUG_LEVEL_
#define _DEBUG_LEVEL_  2
#endif
#include "debug.h"


/*=============================================================================*
 *                        Const definition
 *============================================================================*/

/*=============================================================================*
 *                    New type or enum declaration
 *============================================================================*/
typedef struct
{
    OSMutex_t mutex;
    pthread_cond_t cond;
}OSCond_st;

/*=============================================================================*
 *                        Macro definition
 *============================================================================*/
#define TO_MUTEX(_mutex_)       (pthread_mutex_t*)(_mutex_)
#define TO_COND(_cond_)         (OSCond_st*)(_cond_)
/*=============================================================================*
 *                    Inner function declaration
 *============================================================================*/

 /*=============================================================================*
  *                    Outer function implemention
  *============================================================================*/
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


OSMutex_t OS_MutexCreate()
{
    pthread_mutex_t *p_mutex = (pthread_mutex_t* )OS_Malloc(sizeof(pthread_mutex_t));
    if (p_mutex == NULL)
    {
        LOG_E("Fail to malloc mutex.\n");
        return NULL;
    }

    pthread_mutex_init(p_mutex, NULL);

    return (OSMutex_t)p_mutex;
}

void  OS_MutexDestroy(OSMutex_t mutex)
{
    if (mutex != NULL)
    {
        pthread_mutex_destroy(TO_MUTEX(mutex));
        OS_Free(mutex);
    }
}

int OS_MutexLock(OSMutex_t mutex)
{
    CHECK_PARAM(mutex != NULL, ERR_BAD_PARAM);

    int ret = 0;

    errno = 0;
    ret = pthread_mutex_lock(TO_MUTEX(mutex));
    if (ret != 0)
    {
        LOG_E("Fail to lock mutex, error:%d, '%s'.\n", errno, strerror(errno));
        return ERR_FAIL;
    }

    return ERR_OK;
}

int OS_MutexUnlock(OSMutex_t mutex)
{
    CHECK_PARAM(mutex != NULL, ERR_BAD_PARAM);

    int ret = 0;

    errno = 0;
    ret = pthread_mutex_unlock(TO_MUTEX(mutex));
    if (ret != 0)
    {
        LOG_E("Fail to unlock mutex, error:%d, '%s'.\n", errno, strerror(errno));
        return ERR_FAIL;
    }

    return ERR_OK;
}

OSCond_t OS_CondCreate()
{
    OSCond_st *p_cond = (OSCond_st*)OS_Malloc(sizeof(OSCond_st));
    if (p_cond == NULL)
    {
        LOG_E("Fail to malloc OSCond_st.\n");
        return NULL;
    }

    p_cond->mutex = OS_MutexCreate();
    if (p_cond->mutex == NULL)
    {
        LOG_E("Fail to create mutex.\n");

        OS_Free(p_cond);
        return NULL;
    }

    if (pthread_cond_init(&p_cond->cond, NULL) != 0)
    {
        LOG_E("Fail to init pthread_cond.\n");

        OS_MutexDestroy(p_cond->mutex);
        OS_Free(p_cond);

        return NULL;
    }

    return p_cond;
}

int OS_CondDestroy(OSCond_t cond)
{
    CHECK_PARAM(cond != NULL, ERR_BAD_PARAM);

    OSCond_st* p_cond = TO_COND(cond);

    OS_MutexDestroy(p_cond->mutex);
    pthread_cond_destroy(&p_cond->cond);
    OS_Free(p_cond);

    return ERR_OK;
}

int OS_CondWait(OSCond_t cond)
{
    CHECK_PARAM(cond != NULL, ERR_BAD_PARAM);

    int ret = 0;
    OSCond_st* p_cond = TO_COND(cond);

    errno = 0;
    ret = pthread_cond_wait(&p_cond->cond, TO_MUTEX(p_cond->mutex));
    if (ret != 0)
    {
        LOG_E("Fail to wait cond, error:%d, '%s'.\n", errno, strerror(errno));
        return ERR_FAIL;
    }

    return ERR_OK;
}

int OS_CondTimedWait(OSCond_t cond, CdataTime_t timeoutMs)
{
    return ERR_OK;
}

int OS_CondLock(OSCond_t cond)
{
    CHECK_PARAM(cond != NULL, ERR_BAD_PARAM);

    OSCond_st* p_cond = TO_COND(cond);
    return OS_MutexLock(p_cond->mutex);
}

int OS_CondUnlock(OSCond_t cond)
{
    CHECK_PARAM(cond != NULL, ERR_BAD_PARAM);

    OSCond_st* p_cond = TO_COND(cond);
    return OS_MutexLock(p_cond->mutex);

}

int OS_CondSignal(OSCond_t cond)
{
    CHECK_PARAM(cond != NULL, ERR_BAD_PARAM);

    int ret = 0;
    OSCond_st* p_cond = TO_COND(cond);

    errno = 0;
    ret = pthread_cond_signal(&p_cond->cond);
    if (ret != 0)
    {
        LOG_E("Fail to signal cond, error:%d, '%s'.\n", errno, strerror(errno));
        return ERR_FAIL;
    }

    return ERR_OK;
}
int OS_CondBroadcast(OSCond_t cond)
{
    CHECK_PARAM(cond != NULL, ERR_BAD_PARAM);

    int ret = 0;
    OSCond_st* p_cond = TO_COND(cond);

    errno = 0;
    ret = pthread_cond_broadcast(&p_cond->cond);
    if (ret != 0)
    {
        LOG_E("Fail to broadcast cond, error:%d, '%s'.\n", errno, strerror(errno));
        return ERR_FAIL;
    }

    return ERR_OK;
}


/*=============================================================================*
 *                                End of file
 *============================================================================*/


