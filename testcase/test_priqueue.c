#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "cdata.h"
#include "test_case.h"

#ifndef _DEBUG_LEVEL_
#define _DEBUG_LEVEL_  _DEBUG_LEVEL_I_
#endif
#include "debug.h"


//=============================================================================
static void Init(void);
static void Show(void);
static void Run(int id);
static int GetTestcaseCount(void);
static void Finalize(void);

static int TestBasicFunction();
static int TestStructureData();
static int TestStructureRef();
static int TestMultiThread();
static int TestTimedMultiThread();


static void InitMsgData(const char* p_msgData, Message_t *p_msg);
static Message_t* CreateMsg(const char* p_msgData);
static Queue_t CreateStructureQueue();
static Queue_t CreateStructurePointerQueue();

static int CpMessage(void *p_queueData, void *p_userData);
static void FreeMessage(void *p_message);
static void ShowMsgQueue(Queue_t queue);
static void TraverseMsgQueue(PriQueueTraverseDataInfo_t *p_queueData, void* p_userData);


//=============================================================================
TestcaseSet_t PriQueueTestcaseSet =
{
    "Test all pri_queue functions.",
    Init,
    Show,
    Run,
    GetTestcaseCount,
    Finalize
};

static Testcase_t g_testcaseArray[] =
{
    {"Test all the basic functions of pri_queue.", TestBasicFunction},
    {"Test structure data pri_queue.", TestStructureData},
    {"Test value reference pri_queue.", TestStructureRef},
    {"Test multi thread, writing/reading a same pri_queue.", TestMultiThread},
    {"Test timed wait in multi thread.", TestTimedMultiThread},
};

//=============================================================================
static void Init(void)
{
    return;
}

static void Show(void)
{
    int i = 0;

    printf("\n==============================================\n");
    for (i = 0; i < sizeof(g_testcaseArray) / sizeof(g_testcaseArray[0]); i++)
    {
        printf("%d: %s\n", i, g_testcaseArray[i].p_description);
    }
    printf("==============================================\n");

}
static void Run(int id)
{
    if (id < 0 || id >= sizeof(g_testcaseArray) / sizeof(g_testcaseArray[0]))
    {
        return;
    }

    int ret = g_testcaseArray[id].testcaseFn();
    if (ret == 0)
    {
        printf("Testcase %d passed.\n", id);
    }
    else
    {
        printf("Testcase %d failed.\n", id);
    }

}

static int GetTestcaseCount(void)
{
    return sizeof(g_testcaseArray) / sizeof(g_testcaseArray[0]);
}

static void Finalize(void)
{
    return;
}

//=============================================================================
static void TraverseIntQueue(PriQueueTraverseDataInfo_t *p_queueData, void* p_userData)
{
    int *p_data = (int*)p_queueData->p_data;
    printf("%d, value:%d, priority:%d.\n", (int)p_queueData->index, *p_data, p_queueData->priority);
}

static int CopyIntValue(void *p_queueData, void* p_userData)
{
    if (p_queueData == NULL)
    {
        LOG_E("p_queueData is NULL.\n");
        return -1;
    }

    if (p_userData == NULL)
    {
        LOG_E("p_userData is NULL.\n");
        return -1;
    }

    *((int*)p_userData) = *((int*)p_queueData);

    return 0;
}

static int ShowIntQueue(Queue_t queue)
{
    printf("\n==================%s=======================\n", PriQueue_Name(queue));
    PriQueue_Traverse(queue, NULL, TraverseIntQueue);
    printf("=======================================================\n");

    return 0;
}

static int TestBasicFunction()
{
    int priority = 0;
    Queue_t intQueue;
    PriQueue_Create("IntValueQueue", sizeof(int), CopyIntValue, &intQueue);

    printf("==>Success to create queue:'%s'.\n", PriQueue_Name(intQueue));

    int value = 0;

    printf("\n==>Will test push.\n");
    for (value = 1; value < 10; value++)
    {
        priority = random() % 100;

        printf("Will push, value:%d, priority:%d.\n", value, priority);
        PriQueue_Push(intQueue, &value, priority);
    }
    printf("\n==>After push, count:%d, elements:", (int)PriQueue_Count(intQueue));
    ShowIntQueue(intQueue);

    printf("\n==>Will test pop.\n");
    while (PriQueue_Count(intQueue) > 3)
    {
        PriQueue_GetHead(intQueue, &value, &priority);
        printf("Pop queue head data:%d, priority:%d.\n", value, priority);

        PriQueue_Pop(intQueue);
    }
    LOG_A("After pop, queue count:%d.\n",(int)PriQueue_Count(intQueue));
    
    PriQueue_Clear(intQueue);
    LOG_A("After clear, queue count:%d.\n",(int)PriQueue_Count(intQueue));

    printf("Will test all node has the same priority.\n");
    for (value = 1; value < 10; value++)
    {
        priority = 10;
        printf("Will push, value:%d, priority:%d.\n", value, priority);
        PriQueue_Push(intQueue, &value, priority);
    }
    printf("\n==>After push, count:%d, elements:", (int)PriQueue_Count(intQueue));
    ShowIntQueue(intQueue);

    printf("\n==>Will test pop.\n");
    while (PriQueue_Count(intQueue) > 0)
    {
        PriQueue_GetHead(intQueue, &value, &priority);
        printf("Pop queue head data:%d, priority:%d.\n", value, priority);

        PriQueue_Pop(intQueue);
    }

    PriQueue_Destroy(intQueue);
    return 0;
}

static int TestStructureData()
{
    Queue_t queue;

    queue = CreateStructureQueue();
    ShowMsgQueue(queue);

    printf("\n==>Will test pop:\n");
    while (PriQueue_Count(queue) > 0)
    {
        Message_t msg;

        PriQueue_GetHead(queue, &msg, NULL);
        printf("Pop structure head:'%s'.\n", msg.p_message);
        OS_Free(msg.p_message);

        PriQueue_Pop(queue);
    }

    PriQueue_Destroy(queue);

    return 0;
}

static int TestStructureRef()
{
    Queue_t queue;

    queue = CreateStructurePointerQueue();
    ShowMsgQueue(queue);

    printf("\n==>Will pop:\n");
    while (PriQueue_Count(queue) > 0)
    {
        Message_t msg;

        PriQueue_GetHead(queue, &msg, NULL);
        printf("Pop structure pointer head:'%s'.\n", msg.p_message);
        OS_Free(msg.p_message);

        PriQueue_Pop(queue);
    }

    PriQueue_Destroy(queue);

    return 0;
}

static int g_writeDataFinished = 0;
static void* WriteDataThread(void *p_param)
{
    Queue_t queue = *((Queue_t*)p_param);
    int i = 0;
    Message_t msg;
    char msgStr[128];
    int  priority = 0;

    for (i = 0; i < 30; i++)
    {
        memset(msgStr, 0, sizeof(msgStr));
        sprintf(msgStr, "Msg-%d", i);
        InitMsgData(msgStr, &msg);
        priority = random() % 3;

        printf("Write:Push to back:'%s', priority:%d.\n", msg.p_message, priority);
        PriQueue_Push(queue, &msg, priority);

        sleep(1);
    }

    g_writeDataFinished = 1;
    LOG_A("g_writeDataFinished:%d.\n", g_writeDataFinished);

    return NULL;
}

static void* ReadDataThread(void *p_param)
{
    Queue_t queue = *((Queue_t*)p_param);
    int ret = 0;
    int i = 0;
    Message_t msg;

    for (i = 0; i < 1200; i++)
    {
        ret = PriQueue_WaitDataReady(queue);
        if (ret == ERR_OK)
        {
            ShowMsgQueue(queue);
            if (PriQueue_GetHead(queue, &msg, NULL) == ERR_OK)
            {
                PriQueue_Pop(queue);

                printf("==>Read():Data ready, pop head:'%s'.\n\n", msg.p_message);
                OS_Free(msg.p_message);

                if (i > 4)
                {
                    sleep(2);
                }
            }

            if (g_writeDataFinished)
            {
                LOG_A("Queue count:%d.\n", (int)PriQueue_Count(queue));
                ShowMsgQueue(queue);
                break;
            }
        }
        else
        {
            LOG_E("Data not ready.\n");
        }
    }

    return NULL;
}

static int TestMultiThread()
{
	pthread_t id1;
	pthread_t id2;

    Queue_t queue;
    Message_t msg;
	PriQueue_Create("WaitQueue", sizeof(Message_t), CpMessage, &queue);
    PriQueue_SetFreeFunc(queue, FreeMessage);

    g_writeDataFinished = 0;
	pthread_create(&id1, NULL, ReadDataThread, &queue);
	pthread_create(&id2, NULL, WriteDataThread, &queue);

    pthread_join(id2, NULL);
    if (g_writeDataFinished)
    {
        LOG_A("Push the last data.\n");
        InitMsgData("LastData", &msg);
        PriQueue_Push(queue, &msg, 12);
    }

	pthread_join(id1, NULL);

    PriQueue_Destroy(queue);
    return 0;
}

static void* TimedReadDataThread(void *p_param)
{
    Queue_t queue = *((Queue_t*)p_param);
    int ret = 0;
    int i = 0;
    Message_t msg;
    CdataTime_t waitTimeout = 0;

    i = 0;
    while(1)
    {
        if ((i % 3) == 0)
        {
            waitTimeout = 500;
        }
        else
        {
            waitTimeout = 2000;
        }

        ret = PriQueue_TimedWaitDataReady(queue, waitTimeout);
        if (ret == ERR_OK)
        {
            ShowMsgQueue(queue);
            if (PriQueue_GetHead(queue, &msg, NULL) == ERR_OK)
            {
                PriQueue_Pop(queue);

                printf("==>Read():Data ready, head is:'%s'.\n", msg.p_message);
                OS_Free(msg.p_message);
            }
        }
        else if (ret == ERR_TIME_OUT)
        {
            LOG_E("Time out:%d ms.\n", (int)waitTimeout);
        }

        if (g_writeDataFinished && (PriQueue_Count(queue) == 0))
        {
            break;
        }

        i++;
    }

    return NULL;
}

static int TestTimedMultiThread()
{
	pthread_t id1;
	pthread_t id2;

    Queue_t queue;
	PriQueue_Create("TimedWaitQueue", sizeof(Message_t), CpMessage, &queue);
    PriQueue_SetFreeFunc(queue, FreeMessage);

    g_writeDataFinished = 0;
	pthread_create(&id1, NULL, TimedReadDataThread, &queue);
	pthread_create(&id2, NULL, WriteDataThread, &queue);

    pthread_join(id2, NULL);
	pthread_join(id1, NULL);

    PriQueue_Destroy(queue);
    return 0;
}

static void InitMsgData(const char* p_msgData, Message_t *p_msg)
{
    ASSERT(p_msgData != NULL);
    ASSERT(p_msg != NULL);

    p_msg->msgLen = strlen(p_msgData);
    p_msg->p_message = (char*)OS_Malloc(sizeof(char) *(p_msg->msgLen + 1));
    strcpy(p_msg->p_message, p_msgData);

    return;
}

static Message_t* CreateMsg(const char* p_msgData)
{
    ASSERT(p_msgData != NULL);

    Message_t *p_msg = (Message_t*)OS_Malloc(sizeof(Message_t));
    p_msg->msgLen = strlen(p_msgData);
    p_msg->p_message = (char*)OS_Malloc(sizeof(char) *(p_msg->msgLen + 1));
    strcpy(p_msg->p_message, p_msgData);

    return p_msg;
}

static Queue_t CreateStructureQueue()
{
    Queue_t queue;
    int priority = 0;
    char msgStr[256];
    Message_t msg;
    int i = 0;

    PriQueue_Create("StructureDataQueue", sizeof(Message_t), CpMessage, &queue);
    PriQueue_SetFreeFunc(queue, FreeMessage);

    for (i = 0; i < 10; i++)
    {
        memset(msgStr, 0, sizeof(msgStr));
        sprintf(msgStr, "Msg-%d", i);
        InitMsgData(msgStr, &msg);
        
        priority = (i % 7);
        printf("Push msg:%s, priority:%d.\n", msg.p_message, priority);
        PriQueue_Push(queue, &msg, priority);
    }

    return queue;
}

static Queue_t CreateStructurePointerQueue()
{
    Queue_t queue;

    char msgStr[256];
    Message_t *p_msg;
    int i = 0;
    int priority = 0;

    PriQueue_CreateRef("StructurePointerQueue", CpMessage, &queue);
    PriQueue_SetFreeFunc(queue, FreeMessage);

    for (i = 0; i < 10; i++)
    {
        memset(msgStr, 0, sizeof(msgStr));
        sprintf(msgStr, "Pointer-%d", i);
        p_msg = CreateMsg(msgStr);
        
        priority = random() % 20;
        printf("Push msg:%s, priority:%d.\n", p_msg->p_message, priority);
        PriQueue_Push(queue, p_msg, priority);
    }

    return queue;
}

static int CpMessage(void *p_queueData, void *p_userData)
{
    ASSERT(p_queueData != NULL);
    ASSERT(p_userData != NULL);

    Message_t *p_queueMsg = (Message_t*)p_queueData;
    Message_t *p_userMsg = (Message_t*)p_userData;

    p_userMsg->p_message = (char*)OS_Malloc(sizeof(char) * (p_queueMsg->msgLen + 1));
    strcpy(p_userMsg->p_message, p_queueMsg->p_message);
    p_userMsg->msgLen = p_queueMsg->msgLen;

    return 0;
}

static void FreeMessage(void *p_message)
{
    ASSERT(p_message != NULL);

    Message_t *p_data = (Message_t*)p_message;
    if (p_data->p_message != NULL)
    {
        //LOG_I("Free msg:'%s'.\n\n", p_data->p_message);
        OS_Free(p_data->p_message);
    }

    OS_Free(p_data);

    return;
}

static void ShowMsgQueue(Queue_t queue)
{
    printf("\n==================%s=======================\n", PriQueue_Name(queue));
    PriQueue_Traverse(queue, NULL, TraverseMsgQueue);
    printf("==========================================================\n\n");
}

static void TraverseMsgQueue(PriQueueTraverseDataInfo_t *p_queueData, void* p_userData)
{
    ASSERT(p_queueData != NULL);

    Message_t *p_data = (Message_t*)p_queueData->p_data;
    printf("%d, msg:'%s', priority:%d.\n", (int)p_queueData->index, p_data->p_message, p_queueData->priority);

    return;
}

/*=============================================================================*
 *                                End of file
 *============================================================================*/


