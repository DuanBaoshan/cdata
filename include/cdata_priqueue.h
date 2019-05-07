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

/*
 *PriQueue: Priority queue, when push a data into pri_queue, you can specify its
 *priority. The queue will store the highest priority data to the head, if the data
 *has same priority, the order in queue will be first in first out. The minimum
 *value has the lowest priority.
 */

#ifndef _CDATA_PRI_QUEUE_H_
#define _CDATA_PRI_QUEUE_H_

#include "cdata_types.h"

__BEGIN_EXTERN_C_DECL__

typedef struct
{
    CdataIndex_t index;
    void*        p_data;
    int          priority;
}PriQueueTraverseDataInfo_t;

typedef int (*PriQueueValueCp_fn)(void *p_queueData, void* p_userData);
typedef void (*PriQueueFreeData_fn)(void* p_data);
typedef void (*PriQueueTraverse_fn)(PriQueueTraverseDataInfo_t *p_queueData, void* p_userData);

int PriQueue_Create(QueueName_t name, int dataSize, PriQueueValueCp_fn valueCpFn, Queue_t* p_queue);
int PriQueue_CreateRef(QueueName_t name, PriQueueValueCp_fn valueCpFn, Queue_t* p_queue);

const char*  PriQueue_Name(Queue_t queue);
CdataCount_t PriQueue_Count(Queue_t queue);

int PriQueue_SetFreeFunc(Queue_t queue, PriQueueFreeData_fn freeFn);

int PriQueue_Push(Queue_t queue, void *p_data, int priority);

int  PriQueue_GetHead(Queue_t queue, void* p_headData, int* p_priority);
int  PriQueue_Pop(Queue_t queue);


int PriQueue_WaitDataReady(Queue_t queue);
int PriQueue_TimedWaitDataReady(Queue_t queue, CdataTime_t timeOutMs);

int PriQueue_Traverse(Queue_t queue, void*p_userData, PriQueueTraverse_fn traverseFn);

int PriQueue_Clear(Queue_t queue);
int PriQueue_Destroy(Queue_t queue);


__END_EXTERN_C_DECL__

#endif
