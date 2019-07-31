# What is cdata
Cdata is a module which contains data structure implementation of list(double link list, single link list), queue, priority queue, and written by C language.
# What can cdata do   
   1. It provides simple and powerful operations on list. Based on the list it provides queue and priority queue implementation.   
It provides many new operations on list.  
You can operate on double link list and single link list with a same set of functions, it's very easy to   
transform the list type, from double link to single link and from single to double.  
You can operate on data level, you also can operate on node level.  
You can insert node/data into list in ascending or descending order, you can insert data uniquely.  
You can detach node/data, and you also can remove node/data.  
You find all the match data from head to tail, and from tail to head, by keyword or by a condition.  
You can choose to use lock version functions and no lock version functions, as you wish.  
You can call all the lock version functions in multi-thread environment safely without any external mutex.  
   2. It provides a queue implementation.  
   3. It provides a priority queue implementation.  

# How to use cata  
## Use cdata_list  
### When use cdata_list , you need make some decisions first:  
   1. Which kind of list type you want to use, single link and double link?The most functions in cdata_list  
are same for the both type of link list, but there are a few functions different for single and double link.
Firstly, when you call List_TraverseReversely on a single list, it will fail.Secondly, the time to detach node  
of double list is O(1), but for the single list it's O(n).So if you never use List_TraverseReversely and detach
node, you can use single link list.  
   2. Which kind of data type you want to store, normal data(int, long, float, structure and so on) or pointer.
For the former, the cdata_list  will allocate a same size memory in the node, then memcpy user data to the 
node data, then user can destroy data out of list safely.For the latter, the cdata_list  will store 
pointer of the user data directly, and user cannot destroy the data when the node is in use.It's like the 
two ways to send parameter to a function, value copy and value reference.And List_Create is for the former, 
List_CreateRef is for the latter.But when you want to store the C++ class object, you only can use List_CreateRef.  
   3. How to free the memory of the node data.If the data you stored has no internal pointer, you can let cdata_list 
to free the node data safely, for example:   
```
        struct foo  
        {  
            int value1;  
            float value2;  
            char value3[256];  
        };
```        
But if there are internal pointer(s) in your data, you need free it by yourself, so you need call List_SetFreeDataFunc
to set a customer free function, for example:
```
        struct foo  
        {  
            int value1;  
            int *p_value2;  
            float *p_value3;  
        };  
```
Because cdata_list doesn't know the details of your data, so it cannot free the data correctly.

### About normal and NL functions.  
The functions without suffix "NL" (e.g. List_GetHead, List_InsertData) will call List_Lock and List_UnLock
internally, so they can be called safely in the multi-thread environment.  
For example:  
```
List_t g_testList;  
void Thread1()  
{  
    int i = 0;  
    for (i = 0; i < 10; i++)  
    {  
        List_InsertData(g_testList, &i);  
    }  
}  

void Thread2()  
{  
    int i = 0;  
    for (i = 0; i < 3; i++)  
    {  
        List_DetachTailData(g_testList);  
    }  
}
```
If you decide to call the "NL" functions(NoLock functions, e.g. List_GetHeadNL, List_GetPreNodeNL) in the multi-thread environment, 
you need call List_Lock and List_UnLock explicitly.  
For example:  
```
List_t g_testList;  
void Thread1()  
{  
   ListNode_t node;   
   
   while (1)  
   {  
       List_Lock(g_testList);  
       for (node = List_GetHeadNL(g_testList); node != NULL; node = List_GetNextNodeNL(g_testList, node))  
       {  
           int data = *((int*)List_GetNodeDataNL(g_testList, node));  
           if ((data % 3) == 0)  
           {  
               List_RmNodeNL(g_testList, node);  
               break;  
           }  
       }  
       List_UnLock(g_testList);  
   }  
}  

void Thread2()  
{  
   ListNode_t node;   
   
   while(1)  
   {  
       List_Lock(g_testList);  
       for (node = List_GetHeadNL(g_testList); node != NULL; node = List_GetNextNodeNL(g_testList, node))  
       {  
           int data = *((int*)List_GetNodeDataNL(g_testList, node));  
           if ((data % 12) == 0)  
           {  
               List_RmNodeNL(g_testList, node);  
               break;  
           }  
       }  
       List_UnLock(g_testList);  
   }  
}  
```

### Examples
#### 1. Operate on simple data:
```
    CdataBool IntLtListData(void* p_nodeData, void* p_userData)
    {
        int userData = *(int*)p_userData;
        int nodeData = *(int*)p_nodeData;
		
        return userData < nodeData;
    }
	
    void TraverseIntList(ListTraverseNodeInfo_t *p_info, void *p_userData, CdataBool* p_needStop)
    {
        int data = *((int *)p_info->p_data);
        int index = (int)p_info->index;

        printf("%d:%d\n", index, data);
        return;
    }

    int ShowIntList(List_t list)
    {
        printf("=============%s Begin==================\n", List_Name(list));
        List_Traverse(list, NULL, TraverseIntList);
        printf("=============%s End==================\n", List_Name(list));

        return 0;
    }

    int ShowIntListReversely(List_t list)
    {
        printf("=============%s Begin==================\n", List_Name(list));
        List_TraverseReversely(list, NULL, TraverseIntList);
        printf("=============%s End==================\n", List_Name(list));

        return 0;
    }

    void TraverseStopIntList(ListTraverseNodeInfo_t *p_info, void *p_userData, CdataBool* p_needStop)
    {
        int data = *((int *)p_info->p_data);
        int index = (int)p_info->index;
        int userData = *((int *)p_userData);

        printf("data:%d, userData:%d, index:%d.\n", data, userData, index);

        if (data < userData)
        {
            printf("\nFind a data(%d) < userData(%d), index is:%d.\n", data, userData, index);
            *p_needStop = CDATA_TRUE;
        }

        return;
    }

    int Test()
    {
        int i = 0;
        int value = 0;
        int ret = ERR_OK;
	
        List_t intList;
	
        ret = List_Create("IntList", LIST_TYPE_DOUBLE_LINK, sizeof(int), &intList);
        if (ret != ERR_OK)
        {
            LOG_E("Fail to create normal list.\n");
            return -1;
        }
	
        List_SetUserLtNodeFunc(intList, IntLtListData);
        List_SetEqual2KeywordFunc(intList, IntEqualListData);	
	
        //Test List_InsertData
        printf("Will test List_InsertData.\n");
        for (i = 0; i < 10; i++)
        {
            value = i + 1;
            List_InsertData(intList, &value);
        }
        printf("After inster data:\n");
        ShowIntList(intList);

        printf("Show int list reversely:\n");
        ShowIntListReversely(intList);

        List_Clear(intList);
        printf("After clear, list count:%d.\n", (int)List_Count(intList));

        //Test List_InsertDataAsc
        printf("Will test inserting random data in ascending order.\n");	
        for (i = 0; i < 10; i++)
        {
            value = (random() % 100) + 1;
            List_InsertDataAsc(intList, &value);
        }
        ShowIntList(intList);
        List_Clear(intList);

        //Test List_InsertDataDes
        printf("Will test inserting random data in descending order.\n");
        for (i = 0; i < 10; i++)
        {
            value = (random() % 100) + 1;
            List_InsertDataDes(intList, &value);
        }
        ShowIntList(intList);
        List_Clear(intList);

        //Test match data
        printf("Will test data exists.\n");
        for (i = 0; i < 10; i++)
        {
            value = (random() % 100) + 1;
            if (value > 50)
            {
                value = 3;
            }
            List_InsertData(intList, &value);
        }

        value = 3;
        List_InsertDataAtPos(intList, &value, 0);
        List_InsertDataAtPos(intList, &value, 3);
        List_InsertDataAtPos(intList, &value, List_Count(intList) - 1);
        ShowIntList(intList);

        value = 3;
        if (List_DataExists(intList, &value))
        {
            printf("%d exists.\n", value);
        }
        else
        {
            printf("%d not exists.\n", value);
        }

        count = List_GetMachCount(intList, &value);
        printf("The count of value(%d) in the list is:%d.\n", value, (int)count);

        List_RmAllMatchNodes(intList, &value);
        printf("After rm all value:%d from list.\n", value);
        ShowIntList(intList);

        //Test stop in traverse
        value = 50;
        printf("Will find the data which < userData:%d.\n", value);
        List_Traverse(intList, &value, TraverseStopIntList);

        value = 20;
        printf("Will find the data which < userData:%d reversely.\n", value);
        List_TraverseReversely(intList, &value, TraverseStopIntList);

        List_Destroy(intList);
        return 0;
    }
```
#### 2. Operate on structure without pointer member value:
```
    typedef struct
    {
        char name[128];
        char sex;
        int age;
        int chineseScore;
        int mathScore;
        int englishScore;
    }Student_t;

    Student_t* CreateStudent(const char* p_name,
                                    char sex,
                                    int age,
                                    int chineseScore,
                                    int mathScore,
                                    int englishScore)
    {
        Student_t *p_student = (Student_t *)malloc(sizeof(Student_t));
        
        strcpy(p_student->name, p_name);
        p_student->sex = sex;
        p_student->age = age;
        p_student->chineseScore = chineseScore;
        p_student->mathScore = mathScore;
        p_student->englishScore = englishScore;

        return p_student;
    }

    void PrintStudent(Student_t *p_student)
    {
        int total = p_student->chineseScore + p_student->mathScore + p_student->englishScore;
        printf("Student %d, \tName:%s, \tsex:'%c', \tage:%d, \ttotal:%d, \tchinese:%d, \tmath:%d, \tenglish:%d.\n", (int)(p_nodeInfo->index + 1), p_student->p_name, p_student->sex, p_student->age, total, p_student->chineseScore, p_student->mathScore, p_student->englishScore);    
    }
    
    void ShowStudent(ListTraverseNodeInfo_t* p_nodeInfo, void* p_userData, CdataBool* p_needStopTraverse)
    {
        if (p_nodeInfo == NULL)
        {
            LOG_E("p_nodeInfo is NULL.\n");
            return ;
        }

        Student_t *p_student = (Student_t *)p_nodeInfo->p_data;
        PrintStudent(p_student);
        
        return ;
    }

    int ShowStudentList(List_t list)
    {
        printf("====================%s Begin============================\n", List_Name(list));
        List_Traverse(list, NULL, ShowStudent);
        printf("====================%s End============================\n\n", List_Name(list));
        return 0;
    }

    CdataBool SortedByTotalScore(void* p_listNodeData, void* p_userData)
    {
        Student_t *p_data = (Student_t *)p_userData;
        Student_t *p_nodeData = (Student_t *)p_listNodeData;

        int total1 = p_data->chineseScore + p_data->mathScore + p_data->englishScore;
        int total2 = p_nodeData->chineseScore + p_nodeData->mathScore + p_nodeData->englishScore;

        return total1 < total2;
    }

    CdataBool StudentIsEqual(void* p_listNodeData, void* p_userData)
    {
        Student_t*   p_student = (Student_t*)p_listNodeData;
        char*        p_name = (char*)p_userData;

        return (strcmp(p_student->p_name, p_name) == 0);
    }

    int CreateStudentList(List* p_list)
    {
        Student_t *p_stu = NULL;
        List_t list;
        
        List_CreateRef("StudentList", LIST_TYPE_DOUBLE_LINK, &list);
        List_SetUserLtNodeFunc(p_list, SortedByTotalScore);
        
        p_stu = CreateStudent("Jack", 'M', 11, 78, 82, 67);
        List_InsertDataDes(list, p_stu);

        p_stu = CreateStudent("Jan", 'F', 10, 90, 96, 80);
        List_InsertDataDes(list, p_stu);

        p_stu = CreateStudent("Lily", 'F', 10, 87, 76, 80);
        List_InsertDataDes(list, p_stu);

        p_stu = CreateStudent("Jack", 'M', 10, 89, 93, 78);
        List_InsertDataDes(list, p_stu);

        p_stu = CreateStudent("Alice", 'F', 12, 97, 88, 79);
        List_InsertDataDes(list, p_stu);

        p_stu = CreateStudent("John", 'M', 11, 85, 79, 94);
        List_InsertDataDes(list, p_stu);

        p_stu = CreateStudent("Tom", 'M', 10, 93, 100, 91);
        List_InsertDataDes(list, p_stu);
        
        p_stu = CreateStudent("John", 'M', 12, 62, 74, 76);
        List_InsertDataDes(list, p_stu);
        
        *p_list = list;
        return 0;
    }

    int Test()
    {
        Student_t *p_stu = NULL;
        List_t studentList;
        
        CreateStudentList(&studentList);
        ShowStudentList(studentList);
        
        ListNode_t node = NULL;
        
        for (node = List_GetFirstMatchNodeByCond(studentList, "Jack", StudentIsEqual); 
              node != NULL; 
              node = List_GetNextMatchNodeByCond(studentList, List_GetNextNode(studentList, node), "Jack", StudentIsEqual))
        {
            p_stu = (Student_t*)List_GetNodeData(studentList, node);
            PrintStudent(p_stu);
        }
        
        for (node = List_GetLastMatchNodeByCond(studentList, "John", StudentIsEqual); 
              node != NULL; 
              node = List_GetPreMatchNodeByCond(studentList, List_GetPreNode(studentList, node), "John", StudentIsEqual))
        {
            p_stu = (Student_t*)List_GetNodeData(studentList, node);
            PrintStudent(p_stu);
        }

        List_Destroy(studentList);        
    }

```
#### 3. Operate on structure with pointer member value:
```
    typedef struct
    {
        char *p_name;
        char sex;
        int age;
        int chineseScore;
        int mathScore;
        int englishScore;
    }Student_t;

    Student_t* CreateStudent(const char* p_name,
                                    char sex,
                                    int age,
                                    int chineseScore,
                                    int mathScore,
                                    int englishScore)
    {
        Student_t *p_student = (Student_t *)malloc(sizeof(Student_t));
        
        p_student->name = (char*)malloc(strlen(p_name) + 1);
        strcpy(p_student->name, p_name);
        p_student->sex = sex;
        p_student->age = age;
        p_student->chineseScore = chineseScore;
        p_student->mathScore = mathScore;
        p_student->englishScore = englishScore;

        return p_student;
    }
    
    void FreeStudent(void *p_data)
    {
        Student_t *p_student = (Student_t *)p_data;
        free(p_data->p_name);
        free(p_data);
    }

    int Test()
    {
        List_t studentList;
        
        CreateStudentList(&studentList);
        ShowStudentList(studentList);
        
        ListNode_t node = List_DetachNodeByCond(studentList, "Jack", StudentIsEqual);
        PrintStudent((Student_t*)List_GetNodeData(node));
        List_DestroyNode(node);
        
        ShowStudentList(studentList);
        
        List_Destroy(studentList);
    }
```
## Use cdata_queue 
Example:
```
    typedef struct
    {
        char *p_message;
        int   msgLen;
    }Message_t;
    
    int g_writeDataFinished = 0;
    void InitMsgData(const char* p_msgData, Message_t *p_msg)
    {
        p_msg->msgLen = strlen(p_msgData);
        p_msg->p_message = (char*)OS_Malloc(sizeof(char) *(p_msg->msgLen + 1));
        strcpy(p_msg->p_message, p_msgData);

        return;
    }
    
    void* WriteDataThread(void *p_param)
    {
        Queue_t queue = *((Queue_t*)p_param);
        int i = 0;
        Message_t msg;
        char msgStr[128];

        for (i = 0; i < 10; i++)
        {
            memset(msgStr, 0, sizeof(msgStr));
            sprintf(msgStr, "Msg-%d", i);
            InitMsgData(msgStr, &msg);

            if ((i % 3) == 0)
            {
                printf("Write:Push to back:'%s'.\n", msg.p_message);
                Queue_Push(queue, &msg);
            }
            else
            {
                printf("Write:Push to head:'%s'.\n", msg.p_message);
                Queue_Push2Head(queue, &msg);
            }

            sleep(1);
        }
        
        g_writeDataFinished = 1;

        return NULL;
    }

    static void* ReadDataThread(void *p_param)
    {
        Queue_t queue = *((Queue_t*)p_param);
        int ret = 0;
        int i = 0;
        Message_t msg;

        for (i = 0; i < 12; i++)
        {
            ret = Queue_WaitDataReady(queue);
            if (ret == ERR_OK)
            {
                if (Queue_GetHead(queue, &msg) == ERR_OK)
                {
                    Queue_Pop(queue);

                    printf("==>Read():Data ready, head is:'%s'.\n", msg.p_message);
                    OS_Free(msg.p_message);
                }

                if (g_writeDataFinished)
                {
                    break;
                }
            }
            else
            {
                printf("Data not ready.\n");
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
        Queue_Create("WaitQueue", sizeof(Message_t), CpMessage, &queue);
        Queue_SetFreeFunc(queue, FreeMessage);

        g_writeDataFinished = 0;
        pthread_create(&id1, NULL, ReadDataThread, &queue);
        pthread_create(&id2, NULL, WriteDataThread, &queue);

        pthread_join(id2, NULL);
        if (g_writeDataFinished)
        {
            printf("Push the last data.\n");
            InitMsgData("LastData", &msg);
            Queue_Push(queue, &msg);
        }

        pthread_join(id1, NULL);

        Queue_Destroy(queue);
        return 0;
    }

    //Test timed wait
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

            ret = Queue_TimedWaitDataReady(queue, waitTimeout);
            if (ret == ERR_OK)
            {
                if (Queue_GetHead(queue, &msg) == ERR_OK)
                {
                    Queue_Pop(queue);

                    printf("==>Read():Data ready, head is:'%s'.\n", msg.p_message);
                    OS_Free(msg.p_message);
                }
            }
            else if (ret == ERR_TIME_OUT)
            {
                printf("Time out:%d ms.\n", (int)waitTimeout);
            }

            if (g_writeDataFinished && (Queue_Count(queue) == 0))
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
        Queue_Create("TimedWaitQueue", sizeof(Message_t), CpMessage, &queue);
        Queue_SetFreeFunc(queue, FreeMessage);

        g_writeDataFinished = 0;
        pthread_create(&id1, NULL, TimedReadDataThread, &queue);
        pthread_create(&id2, NULL, WriteDataThread, &queue);

        pthread_join(id2, NULL);
        pthread_join(id1, NULL);

        Queue_Destroy(queue);
        return 0;
    }
``` 
## Use cdata_priqueue  
PriQueue: Priority queue, when push a data into pri_queue, you can specify its priority. The minimum value   
has the lowest priority.The queue will store the highest priority data to the head, if the data has same priority,    
the order in the queue will be first in first out.   
    
Example:
```
    void TraverseIntQueue(PriQueueTraverseDataInfo_t *p_queueData, void* p_userData)
    {
        int *p_data = (int*)p_queueData->p_data;
        printf("%d, value:%d, priority:%d.\n", (int)p_queueData->index, *p_data, p_queueData->priority);
    }

    int CopyIntValue(void *p_queueData, void* p_userData)
    {
        *((int*)p_userData) = *((int*)p_queueData);
        return 0;
    }

    int ShowIntQueue(Queue_t queue)
    {
        printf("\n==================%s=======================\n", PriQueue_Name(queue));
        PriQueue_Traverse(queue, NULL, TraverseIntQueue);
        printf("=======================================================\n");

        return 0;
    }

    int Test()
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
```

