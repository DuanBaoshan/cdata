/*
MIT License

Copyright (c) 2018 DuanBaoshan

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Author:DuanBaoshan
E-Mail:duanbaoshan79@163.com
Date:2019.4.30
*/

#include "cdata_queue.h"
#include "cdata_os_adapter.h"
#include "cdata_list.h"
#include "list_internal.h"

#ifndef _DEBUG_LEVEL_
#define _DEBUG_LEVEL_  _DEBUG_LEVEL_I_
#endif
#include "debug.h"

/*=============================================================================*
 *                        Macro definition
 *============================================================================*/
#define TO_QUEUE(_queue_) (Queue_st*)(_queue_)
/*=============================================================================*
 *                        Const definition
 *============================================================================*/

/*=============================================================================*
 *                    New type or enum declaration
 *============================================================================*/
typedef struct
{
    OSCond_t cond;
    List_t   list;
    CdataBool empty;
}Queue_st;

typedef struct
{
    void*p_userData;
    QueueTraverse_fn traverseFn;
}QueueTraverseUserData_t;

/*=============================================================================*
 *                    Inner function declaration
 *============================================================================*/
static Queue_t CreateQueue(QueueName_t name, List_DataType_e dataType, int dataSize);
static void ListTraverseFn(ListTraverseNodeInfo_t* p_nodeInfo, void* p_userData, CdataBool* p_needStopTraverse);
static void CheckIfNeedNotifyCondSignal(Queue_st *p_queue);
/*=============================================================================*
 *                    Outer function implemention
 *============================================================================*/
int Queue_Create(QueueName_t name, int dataSize, Queue_t* p_queue)
{
    CHECK_PARAM(p_queue != NULL, ERR_BAD_PARAM);

    Queue_t newQueue = CreateQueue(name, LIST_DATA_TYPE_VALUE_COPY, dataSize);
    if (newQueue == NULL)
    {
        LOG_E("Fail to create queue:'%s'.\n", name);
        return ERR_FAIL;
    }

    *p_queue = newQueue;
    return ERR_OK;
}
int Queue_CreateRef(QueueName_t name, Queue_t* p_queue)
{
    CHECK_PARAM(p_queue != NULL, ERR_BAD_PARAM);

    Queue_t newQueue = CreateQueue(name, LIST_DATA_TYPE_VALUE_REFERENCE, 0);
    if (newQueue == NULL)
    {
        LOG_E("Fail to create queue:'%s'.\n", name);
        return ERR_FAIL;
    }

    *p_queue = newQueue;
    return ERR_OK;
}

const char* Queue_Name(Queue_t queue)
{
    CHECK_PARAM(queue != NULL, NULL);

    Queue_st *p_queue = TO_QUEUE(queue);
    return List_Name(p_queue->list);
}

CdataCount_t Queue_Count(Queue_t queue)
{
    CHECK_PARAM(queue != NULL, ERR_BAD_PARAM);
    Queue_st *p_queue = TO_QUEUE(queue);

    return List_Count(p_queue->list);
}

int Queue_SetFreeFunc(Queue_t queue, QueueFreeData_fn freeFn)
{
    CHECK_PARAM(queue != NULL, ERR_BAD_PARAM);

    Queue_st *p_queue = TO_QUEUE(queue);

    return List_SetFreeDataFunc(p_queue->list, freeFn);
}

int Queue_Push(Queue_t queue, void *p_data)
{
    CHECK_PARAM(queue != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(p_data != NULL, ERR_BAD_PARAM);

    Queue_st *p_queue = TO_QUEUE(queue);

    List_InsertData(p_queue->list, p_data);
    CheckIfNeedNotifyCondSignal(p_queue);

    return ERR_OK;
}
int Queue_Push2Head(Queue_t queue, void *p_data)
{
    CHECK_PARAM(queue != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(p_data != NULL, ERR_BAD_PARAM);

    Queue_st *p_queue = TO_QUEUE(queue);

    List_InsertData2Head(p_queue->list, p_data);
    CheckIfNeedNotifyCondSignal(p_queue);

    return ERR_OK;
}

void* Queue_Pop(Queue_t queue)
{
    CHECK_PARAM(queue != NULL, ERR_BAD_PARAM);
    Queue_st *p_queue = TO_QUEUE(queue);

    List_DetachHeadData(p_queue->list);

    OS_CondLock(p_queue->cond);
    p_queue->empty = (List_Count(p_queue->list) == 0) ? CDATA_TRUE : CDATA_FALSE;
    OS_CondUnlock(p_queue->cond);

    return ERR_OK;
}
void* Queue_Head(Queue_t queue)
{
    CHECK_PARAM(queue != NULL, ERR_BAD_PARAM);
    Queue_st *p_queue = TO_QUEUE(queue);

    return List_GetHeadData(p_queue->list);
}

void Queue_Lock(Queue_t queue)
{
    CHECK_PARAM(queue != NULL, ERR_BAD_PARAM);
    Queue_st *p_queue = TO_QUEUE(queue);

    List_Lock(p_queue->list);

    return;
}
void Queue_UnLock(Queue_t queue)
{
    CHECK_PARAM(queue != NULL, ERR_BAD_PARAM);
    Queue_st *p_queue = TO_QUEUE(queue);

    List_UnLock(p_queue->list);

    return;
}

int Queue_WaitDataReady(Queue_t queue)
{
    CHECK_PARAM(queue != NULL, ERR_BAD_PARAM);
    Queue_st *p_queue = TO_QUEUE(queue);

    OS_CondLock(p_queue->cond);
    while (p_queue->empty)
    {
        OS_CondWait(p_queue->cond);
    }
    OS_CondUnlock(p_queue->cond);

    return ERR_OK;
}
int Queue_TimedWaitDataReady(Queue_t queue, CdataTime_t timeOutMs)
{
    CHECK_PARAM(queue != NULL, ERR_BAD_PARAM);
    Queue_st *p_queue = TO_QUEUE(queue);

    OS_CondLock(p_queue->cond);
    while (p_queue->empty)
    {
        OS_CondTimedWait(p_queue->cond, timeOutMs);
    }
    OS_CondUnlock(p_queue->cond);

    return ERR_OK;
}

int Queue_Traverse(Queue_t queue, void*p_userData, QueueTraverse_fn traverseFn)
{
    CHECK_PARAM(queue != NULL, ERR_BAD_PARAM);
    Queue_st *p_queue = TO_QUEUE(queue);

    QueueTraverseUserData_t userData;

    userData.p_userData = p_userData;
    userData.traverseFn = traverseFn;
    return List_Traverse(p_queue->list, &userData, ListTraverseFn);
}

int Queue_Clear(Queue_t queue)
{
    CHECK_PARAM(queue != NULL, ERR_BAD_PARAM);
    Queue_st *p_queue = TO_QUEUE(queue);

    return List_Clear(p_queue->list);
}
int Queue_Destroy(Queue_t queue)
{
    CHECK_PARAM(queue != NULL, ERR_BAD_PARAM);
    Queue_st *p_queue = TO_QUEUE(queue);

    return List_Destroy(p_queue->list);
}

/*=============================================================================*
 *                    Inner function implemention
 *============================================================================*/
static Queue_t CreateQueue(QueueName_t name, List_DataType_e dataType, int dataSize)
{
    int ret = 0;
    Queue_st *p_queue = NULL;

    p_queue = (Queue_st*)OS_Malloc(sizeof(Queue_st));
    if (p_queue == NULL)
    {
        LOG_E("Fail to mallocate queue:'%s'.\n", name);
        return NULL;
    }

    memset(p_queue, 0, sizeof(Queue_st));
    p_queue->empty = CDATA_FALSE;

    p_queue->cond = OS_CondCreate();
    if (p_queue->cond == NULL)
    {
        LOG_E("Fail to create cond for queue:'%s'.\n", name);

        OS_Free(p_queue);
        return NULL;
    }

    if (dataType == LIST_DATA_TYPE_VALUE_COPY)
    {
        ret = List_Create(name, LIST_TYPE_SINGLE_LINK, dataSize, &p_queue->list);
    }
    else if (dataType == LIST_DATA_TYPE_VALUE_REFERENCE)
    {
        ret = List_CreateRef(name, LIST_TYPE_SINGLE_LINK, &p_queue->list);
    }
    else
    {
        LOG_E("Bad data type:%d.\n", dataType);

        ret = ERR_BAD_PARAM;
    }


    if (ret != ERR_OK)
    {
        LOG_E("Fail to create list for queue:'%s'.\n", name);

        OS_CondDestroy(p_queue->cond);
        OS_Free(p_queue);
        return NULL;
    }

    return (Queue_t)p_queue;
}

static void ListTraverseFn(ListTraverseNodeInfo_t* p_nodeInfo, void* p_userData, CdataBool* p_needStopTraverse)
{
    if(p_nodeInfo == NULL)
    {
        LOG_E("p_nodeInfo is NULL.\n");
        return;
    }

    if(p_userData == NULL)
    {
        LOG_E("p_userData is NULL.\n");
        return;
    }

    QueueTraverseUserData_t *p_traverseUserData = (QueueTraverseUserData_t*)p_userData;
    QueueTraverseDataInfo_t dataInfo;

    dataInfo.index = p_nodeInfo->index;
    dataInfo.p_data = p_nodeInfo->p_data;
    p_traverseUserData->traverseFn(&dataInfo, p_traverseUserData->p_userData);

    return;
}

static void CheckIfNeedNotifyCondSignal(Queue_st *p_queue)
{
    ASSERT(p_queue != NULL);

    OS_CondLock(p_queue->cond);
    p_queue->empty = (List_Count(p_queue->list) > 0) ? CDATA_FALSE : CDATA_TRUE;
    if (!p_queue->empty)
    {
        OS_CondSignal(p_queue->cond);
    }
    OS_CondUnlock(p_queue->cond);

    return;
}


/*=============================================================================*
 *                                End of file
 *============================================================================*/


