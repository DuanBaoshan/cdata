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
    CdataIndex_t index;/*The data position index.*/
    void*        p_data;
}QueueTraverseDataInfo_t;

typedef int (*QueueValueCp_fn)(void *p_queueData, void* p_userData);
typedef void (*QueueFreeData_fn)(void* p_data);
typedef void (*QueueTraverse_fn)(QueueTraverseDataInfo_t *p_queueData, void* p_userData);

/**
 * @brief Create a queue.User must provide a valueCpFn which will be used in Queue_GetHead.
 * This queue will store the data as value copy model.
 *
 * For example:
 * @code
   typedef struct foo
   {
       int val;
   }foo_t;
   
   int CpValue(void *p_queueData, void* p_userData)
   {
       foo_t *p_qData = (foo_t*)p_queueData;
       foo_t *p_uData = (foo_t*)p_userData;

       p_uData->val = p_qData->val;
       return 0;
   }
   
   Queue_t queue;
   Queue_Create("Example", sizeof(foo_t), CpValue, &queue); 
   foo_t fooVal;
   for (int i = 0; i < 10; i++)
   {
        fooVal.val = i;
        Queue_Push(queue, &fooVal);
   }
   @endcode
 *
 */
int Queue_Create(QueueName_t name, int dataSize, QueueValueCp_fn valueCpFn, Queue_t* p_queue);

/**
 * @brief Create a queue, and this queue will store the data as value reference model.
 *
 * For example:
 * @code
   Queue_t queue;
   Queue_CreateRef("Example", CpValue, &queue); 
   
   foo_t *p_fooVal = NULL;
   for (int i = 0; i < 10; i++)
   {
        p_fooVal = (foo_t*)malloc(sizeof(foo_t));
        p_fooVal->val = i;
        Queue_Push(queue, p_fooVal);
   }
 * @endcode
 */
int Queue_CreateRef(QueueName_t name, QueueValueCp_fn valueCpFn, Queue_t* p_queue);

const char*  Queue_Name(Queue_t queue);
CdataCount_t Queue_Count(Queue_t queue);

/**
 * @brief If the data contains pointer, and when destroy the queue data in Queue_Pop, user must
 * provide a QueueFreeData_fn to free the queue data, because Queue cannot know how to free the 
 * pointer in the queue data.
 * For example:
   @code
   typedef struct bar
   {
       int val;
       float *p_floatVal;
   }bar_t;

   void FreeFn(void *p_data)
   {
       bar_t *p_val = (bar_t*)p_data;
       
       free(p_val->p_floatVal);
       free(p_val);        
   }
   Queue_SetFreeFunc(queue, FreeFn);
   @endcode
 */
int Queue_SetFreeFunc(Queue_t queue, QueueFreeData_fn freeFn);

/**
 * @brief Push the data to the queue tail.
 */
int Queue_Push(Queue_t queue, void *p_data);

int Queue_Push2Head(Queue_t queue, void *p_data);

int  Queue_GetHead(Queue_t queue, void* p_headData);
int  Queue_Pop(Queue_t queue);

/**
 * @brief Wait for the data ready, if queue is empty it will be blocked, until someone
 * push a data.
 */
int Queue_WaitDataReady(Queue_t queue);

/**
 * @brief Wait for the data ready for a period of time, if queue is empty it will be blocked, until someone
 * push a data or time out.
 * @return Error code
 *   @retval ERR_OK:Data is ready.
 *   @retval ERR_TIME_OUT:Time out.
 */
int Queue_TimedWaitDataReady(Queue_t queue, CdataTime_t timeOutMs);

int Queue_Traverse(Queue_t queue, void*p_userData, QueueTraverse_fn traverseFn);

/**
 * @brief Clear all the data in the queue, the queue can be used still.If there is pointer in the
 * queue data, it needs to call Queue_SetFreeFunc to set a custom free function after creating a queue.
 */
int Queue_Clear(Queue_t queue);

/**
 * @brief Clear all the data in the queue and destroy the queue, the queue can be not used any longer.
 * If there is pointer in the queue data, it needs to call Queue_SetFreeFunc to set a custom free 
 * function after creating a queue.
 */
int Queue_Destroy(Queue_t queue);


__END_EXTERN_C_DECL__

#endif
