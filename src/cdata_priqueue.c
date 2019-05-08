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
Date:2019.5.7
*/

#include "cdata_priqueue.h"
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
#define TO_PRIQUEUE(_queue_) (PriQueue_st*)(_queue_)

/*=============================================================================*
 *                        Const definition
 *============================================================================*/

/*=============================================================================*
 *                    New type or enum declaration
 *============================================================================*/
typedef struct
{
    OSCond_t  cond;
    CdataBool empty;

    List_DataType_e dataType;
    int             dataSize;

    PriQueueValueCp_fn     valueCpFn;
    PriQueueFreeData_fn freeFn;
    List_t   list;
}PriQueue_st;

typedef struct
{
    void*p_userData;
    PriQueueTraverse_fn traverseFn;
}PriQueueTraverseUserData_t;

typedef struct
{
    void *p_data;
    int  priority;
}PriQueueData_t;

/*=============================================================================*
 *                    Inner function declaration
 *============================================================================*/
static Queue_t CreatePriQueue(QueueName_t name, List_DataType_e dataType, int dataSize, PriQueueValueCp_fn valueCpFn);

static PriQueueData_t *CreateQueueData(PriQueue_st *p_queue, void *p_data, int priority);
static void DestroyQueueData(PriQueue_st *p_queue, PriQueueData_t *p_data);

CdataBool UserPriorityLtNode(void* p_nodeData, void* p_userData);


static void PriQueueTraverseFn(ListTraverseNodeInfo_t* p_nodeInfo, void* p_userData, CdataBool* p_needStopTraverse);
/*=============================================================================*
 *                    Outer function implemention
 *============================================================================*/
int PriQueue_Create(QueueName_t name, int dataSize, PriQueueValueCp_fn valueCpFn, Queue_t* p_queue)
{
    CHECK_PARAM(p_queue != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(valueCpFn != NULL, ERR_BAD_PARAM);

    Queue_t newQueue = CreatePriQueue(name, LIST_DATA_TYPE_VALUE_COPY, dataSize, valueCpFn);
    if (newQueue == NULL)
    {
        LOG_E("Fail to create queue:'%s'.\n", name);
        return ERR_FAIL;
    }

    *p_queue = newQueue;
    return ERR_OK;
}
int PriQueue_CreateRef(QueueName_t name, PriQueueValueCp_fn valueCpFn, Queue_t* p_queue)
{
    CHECK_PARAM(p_queue != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(valueCpFn != NULL, ERR_BAD_PARAM);

    Queue_t newQueue = CreatePriQueue(name, LIST_DATA_TYPE_VALUE_REFERENCE, 0, valueCpFn);
    if (newQueue == NULL)
    {
        LOG_E("Fail to create queue:'%s'.\n", name);
        return ERR_FAIL;
    }

    *p_queue = newQueue;
    return ERR_OK;
}

int PriQueue_SetFreeFunc(Queue_t queue, PriQueueFreeData_fn freeFn)
{
    CHECK_PARAM(queue != NULL, ERR_BAD_PARAM);

    PriQueue_st *p_queue = TO_PRIQUEUE(queue);
    p_queue->freeFn = freeFn;

    return ERR_OK;
}

const char* PriQueue_Name(Queue_t queue)
{
    CHECK_PARAM(queue != NULL, NULL);

    PriQueue_st *p_queue = TO_PRIQUEUE(queue);
    return List_Name(p_queue->list);
}

CdataCount_t PriQueue_Count(Queue_t queue)
{
    CHECK_PARAM(queue != NULL, ERR_BAD_PARAM);
    PriQueue_st *p_queue = TO_PRIQUEUE(queue);

    return List_Count(p_queue->list);
}

int PriQueue_Push(Queue_t queue, void *p_data, int priority)
{
    CHECK_PARAM(queue != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(p_data != NULL, ERR_BAD_PARAM);

    int ret = ERR_OK;
    PriQueue_st *p_queue = TO_PRIQUEUE(queue);
    PriQueueData_t *p_queueNode = NULL;

    PriQueueData_t *p_queueData = CreateQueueData(p_queue, p_data, priority);
    if (p_queueData == NULL)
    {
        LOG_E("Fail to create queue data.\n");
        return ERR_FAIL;
    }

    if (List_InsertDataDescently(p_queue->list,  p_queueData) == NULL)
    {
        LOG_E("Fail to insert data.\n");

        /*If push fail, we need destroy p_queueData, but the user data we cann't destroy.*/
        p_queueData->p_data = NULL;
        DestroyQueueData(p_queue, p_queueData);

        return ERR_FAIL;
    }

    OS_CondLock(p_queue->cond);
    p_queue->empty = CDATA_FALSE;
    OS_CondSignal(p_queue->cond);
    OS_CondUnlock(p_queue->cond);

    return ERR_OK;
}

int PriQueue_GetHead(Queue_t queue, void* p_headData, int *p_priority)
{
    CHECK_PARAM(queue != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(p_headData != NULL, ERR_BAD_PARAM);

    int ret = ERR_OK;
    PriQueue_st *p_queue = TO_PRIQUEUE(queue);
    void *p_queueData = NULL;

    List_Lock(p_queue->list);
    p_queueData = List_GetHeadDataNL(p_queue->list);
    if (p_queueData == NULL)
    {
        LOG_E("Queue data is NULL.\n");
        ret = ERR_FAIL;
        goto EXIT;

    }

    ret = p_queue->valueCpFn(((PriQueueData_t*)p_queueData)->p_data, p_headData);
    if (ret != 0)
    {
        LOG_E("Fail to copy head data to user.\n");
        ret = ERR_FAIL;
        goto EXIT;
    }

    if (p_priority != NULL)
    {
        *p_priority = ((PriQueueData_t*)p_queueData)->priority;
    }

    ret = ERR_OK;
    EXIT:
    List_UnLock(p_queue->list);

    return ret;
}

int PriQueue_Pop(Queue_t queue)
{
    CHECK_PARAM(queue != NULL, ERR_BAD_PARAM);
    PriQueue_st *p_queue = TO_PRIQUEUE(queue);

    PriQueueData_t *p_queueData = (PriQueueData_t*)List_DetachHeadData(p_queue->list);
    if (p_queueData == NULL)
    {
        LOG_W("The head of queue is NULL.\n");
        return ERR_OK;
    }

    DestroyQueueData(p_queue, p_queueData);

    OS_CondLock(p_queue->cond);
    p_queue->empty = (List_Count(p_queue->list) == 0) ? CDATA_TRUE : CDATA_FALSE;
    OS_CondUnlock(p_queue->cond);

    return ERR_OK;
}

int PriQueue_WaitDataReady(Queue_t queue)
{
    CHECK_PARAM(queue != NULL, ERR_BAD_PARAM);
    PriQueue_st *p_queue = TO_PRIQUEUE(queue);

    OS_CondLock(p_queue->cond);
    while (p_queue->empty)
    {
        OS_CondWait(p_queue->cond);
    }
    OS_CondUnlock(p_queue->cond);

    return ERR_OK;
}
int PriQueue_TimedWaitDataReady(Queue_t queue, CdataTime_t timeOutMs)
{
    CHECK_PARAM(queue != NULL, ERR_BAD_PARAM);
    PriQueue_st *p_queue = TO_PRIQUEUE(queue);
    int ret = 0;

    OS_CondLock(p_queue->cond);
    while (p_queue->empty)
    {
        ret = OS_CondTimedWait(p_queue->cond, timeOutMs);
        if (ret == ERR_TIME_OUT)
        {
            break;
        }
    }
    OS_CondUnlock(p_queue->cond);

    return ret;
}

int PriQueue_Traverse(Queue_t queue, void*p_userData, PriQueueTraverse_fn traverseFn)
{
    CHECK_PARAM(queue != NULL, ERR_BAD_PARAM);
    PriQueue_st *p_queue = TO_PRIQUEUE(queue);

    PriQueueTraverseUserData_t userData;

    userData.p_userData = p_userData;
    userData.traverseFn = traverseFn;
    return List_Traverse(p_queue->list, &userData, PriQueueTraverseFn);
}

int PriQueue_Clear(Queue_t queue)
{
    CHECK_PARAM(queue != NULL, ERR_BAD_PARAM);

    while(PriQueue_Count(queue) > 0)
    {
        PriQueue_Pop(queue);
    }

    return ERR_OK;
}

int PriQueue_Destroy(Queue_t queue)
{
    CHECK_PARAM(queue != NULL, ERR_BAD_PARAM);
    PriQueue_st *p_queue = TO_PRIQUEUE(queue);

    OS_CondLock(p_queue->cond);
    OS_CondBroadcast(p_queue->cond);
    OS_CondUnlock(p_queue->cond);

    PriQueue_Clear(queue);
    List_Destroy(p_queue->list);
    OS_CondDestroy(p_queue->cond);
    OS_Free(p_queue);

    return ERR_OK;
}

/*=============================================================================*
 *                    Inner function implemention
 *============================================================================*/
static Queue_t CreatePriQueue(QueueName_t name, List_DataType_e dataType, int dataSize, PriQueueValueCp_fn valueCpFn)
{
    int ret = 0;
    PriQueue_st *p_queue = NULL;

    p_queue = (PriQueue_st*)OS_Malloc(sizeof(PriQueue_st));
    if (p_queue == NULL)
    {
        LOG_E("Fail to mallocate queue:'%s'.\n", name);
        return NULL;
    }

    memset(p_queue, 0, sizeof(PriQueue_st));

    p_queue->empty = CDATA_TRUE;

    p_queue->dataType = dataType;
    p_queue->dataSize = dataSize;

    p_queue->valueCpFn = valueCpFn;
    p_queue->freeFn = NULL;

    p_queue->cond = OS_CondCreate();
    if (p_queue->cond == NULL)
    {
        LOG_E("Fail to create cond for pri_queue:'%s'.\n", name);

        OS_Free(p_queue);
        return NULL;
    }

    ret = List_CreateRef(name, LIST_TYPE_SINGLE_LINK, &p_queue->list);
    if (ret != ERR_OK)
    {
        LOG_E("Fail to create list for queue:'%s'.\n", name);

        OS_CondDestroy(p_queue->cond);
        OS_Free(p_queue);
        return NULL;
    }

    List_SetUserLtNodeFunc(p_queue->list, UserPriorityLtNode);

    return (Queue_t)p_queue;
}

static PriQueueData_t *CreateQueueData(PriQueue_st *p_queue, void *p_data, int priority)
{
    ASSERT(p_queue != NULL);
    ASSERT(p_data != NULL);

    PriQueueData_t *p_queueData = (PriQueueData_t*)OS_Malloc(sizeof(PriQueueData_t));
    if (p_queueData == NULL)
    {
        LOG_E("Fail to allocate memory for queue data.\n");
        return NULL;
    }

    p_queueData->priority = priority;

    if (p_queue->dataType == LIST_DATA_TYPE_VALUE_REFERENCE)
    {
        p_queueData->p_data = p_data;
        return p_queueData;
    }

    p_queueData->p_data = OS_Malloc(p_queue->dataSize);
    if (p_queueData->p_data == NULL)
    {
        LOG_E("Fail to allocate memory.\n");

        OS_Free(p_queueData);
        return NULL;
    }

    memset(p_queueData->p_data, 0, p_queue->dataSize);
    memcpy(p_queueData->p_data, p_data, p_queue->dataSize);

    return p_queueData;
}

static void DestroyQueueData(PriQueue_st *p_queue, PriQueueData_t *p_data)
{
    ASSERT(p_queue != NULL);
    ASSERT(p_data != NULL);

    if (p_data->p_data != NULL)
    {
        if (p_queue->freeFn != NULL)
        {
            p_queue->freeFn(p_data->p_data);
        }
        else
        {
            OS_Free(p_data->p_data);
        }
    }

    OS_Free(p_data);
}

CdataBool UserPriorityLtNode(void* p_nodeData, void* p_userData)
{
    if (p_nodeData == NULL)
    {
        LOG_E("Qeue node data is NULL.\n");
        return CDATA_FALSE;
    }

    if (p_userData == NULL)
    {
        LOG_E("User data is NULL.\n");
        return CDATA_FALSE;
    }

    return ((PriQueueData_t*)p_nodeData)->priority >= ((PriQueueData_t*)p_userData)->priority;
}

static void PriQueueTraverseFn(ListTraverseNodeInfo_t* p_nodeInfo, void* p_userData, CdataBool* p_needStopTraverse)
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

    PriQueueTraverseUserData_t *p_traverseUserData = (PriQueueTraverseUserData_t*)p_userData;
    PriQueueTraverseDataInfo_t dataInfo;
    PriQueueData_t* p_queueData = NULL;

    p_queueData = (PriQueueData_t*)(p_nodeInfo->p_data);
    dataInfo.index = p_nodeInfo->index;
    dataInfo.p_data = p_queueData->p_data;
    dataInfo.priority = p_queueData->priority;
    p_traverseUserData->traverseFn(&dataInfo, p_traverseUserData->p_userData);

    return;
}

/*=============================================================================*
 *                                End of file
 *============================================================================*/


