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
Date:2019.4.29
*/

#ifndef _CDATA_QUEUE_H_
#define _CDATA_QUEUE_H_

#include "cdata_types.h"

__BEGIN_EXTERN_C_DECL__

typedef struct
{
    CdataIndex_t index;
    void*        p_data;
}QueueTraverseNodeInfo_t;

typedef void (*QueueFreeData_fn)(void* p_data);

int Queue_Create(QueueName_t name, int dataSize, Queue_t* p_queue);
int Queue_CreateRef(QueueName_t name, Queue_t* p_queue);

int Queue_SetFreeFunc(Queue_t queue, QueueFreeData_fn freeFn);

int Queue_Push(Queue_t queue, void *p_data);
int Queue_Push2Head(Queue_t queue, void *p_data);
int Queue_Pop(Queue_t queue);
void* Queue_GetHead(Queue_t queue);

int Queue_Lock(Queue_t queue);
int Queue_UnLock(Queue_t queue);

int Queue_WaitDataReady(Queue_t queue);
int Queue_TimedWaitDataReady(Queue_t queue, CdataTime_t timeOutMs);

CdataCount_t Queue_GetCount(Queue_t queue);
int Queue_Traverse(Queue_t queue);

int Queue_Clear(Queue_t queue);
int Queue_Destroy(Queue_t queue);


__END_EXTERN_C_DECL__

#endif
