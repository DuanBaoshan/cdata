#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "cdata.h"

#ifndef _DEBUG_LEVEL_
#define _DEBUG_LEVEL_  2
#endif
#include "debug.h"


typedef int (*TestcaseFn_t)(void);

typedef struct
{
    char *p_description;
    TestcaseFn_t testcaseFn;
}Testcase_t;

typedef struct
{
	char *p_name;
	char sex;
	int age;
	int chineseScore;
    int mathScore;
	int englishScore;
}Student_t;

static List_t g_normalList;
static List_t g_studentList;

static void ShowAllTestcases();
static void RunTestcase(int id);

static int InitList();
static int DestroyList();


static void FreeStudentData(void* p_data);
CdataBool TotalScoreDescent(void* p_listNodeData, void* p_userData);

CdataBool IntLtListData(void* p_nodeData, void* p_userData);
static int TestNormalList();
static int TestInsertDataAtPos();
static int TestStopTraverseNormalData();
static int TestSwapPos();
static int TestSwapHeadTail();
static int TestSwapNeighbour();
static int TestSwapNeighbourHeadTail();

static int TestNormalStructureList();
static int TestReferenceList();
static int TestDetach();
static int TestRm();
static int TestMatch();

static int TestMultiThread();
static int TestMultiThreadLock();

static Testcase_t g_testcaseArray[] =
{
    {"Test value copy list.", TestNormalList},
	{"Test insert normal data at position.", TestInsertDataAtPos},
    {"Test stop traverse in a normal data list.", TestStopTraverseNormalData},
	{"Test swap two nodes' position.", TestSwapPos},
	{"Test swap the position of head and tail.", TestSwapHeadTail},
	{"Test swap the position of two neighbour nodes.", TestSwapNeighbour},
	{"Test swap the position of two neighbour nodes which are head and tail.", TestSwapNeighbourHeadTail},
	{"Test value copy structure list.", TestNormalStructureList},
	{"Test reference value list.", TestReferenceList},
	{"Test detach node from list.", TestDetach},
	{"Test rm node from list.", TestRm},
	{"Test match data in list.", TestMatch},
	{"Test multi thread.", TestMultiThread},
	{"Test multi thread with List_Lock", TestMultiThreadLock},
};

static int InitList()
{
    int ret = ERR_OK;

    ret = List_Create("NormalDataList", LIST_TYPE_DOUBLE_LINK, sizeof(int), &g_normalList);
    if (ret != ERR_OK)
    {
        LOG_E("Fail to create normal list.\n");
        return -1;
    }
	List_SetUserLtNodeFunc(g_normalList, IntLtListData);


    ret = List_CreateRef("StudentList", LIST_TYPE_DOUBLE_LINK, &g_studentList);
    if (ret != ERR_OK)
    {
        LOG_E("Fail to create normal list.\n");
        return -1;
    }

	List_SetFreeDataFunc(g_studentList, FreeStudentData);
	List_SetUserLtNodeFunc(g_studentList, TotalScoreDescent);

	return 0;
}

static int DestroyList()
{
    int ret = ERR_OK;

    ret = List_Destory(g_normalList);
    if (ret != ERR_OK)
    {
        LOG_E("Fail to destroy normal list.\n");
        return -1;
    }

    ret = List_Destory(g_studentList);
    if (ret != ERR_OK)
    {
        LOG_E("Fail to destroy pointer list.\n");
        return -1;
    }

	return 0;
}

int main(int argc, char* argv[])
{
    LOG_A("Hello dblist.\n");

    int id = 0;
    char choice[128];

	InitList();

    while (1)
    {
        ShowAllTestcases();

        printf("Please input a testcase id(q for quit):");
        scanf("%s", choice);

        if (strcmp(choice, "q") == 0)
        {
            break;
        }

        id = atoi(choice);
        printf("You select:%d.\n", id);

        RunTestcase(id);
    }

	DestroyList();

    return 0;
}

static void ShowAllTestcases()
{
    int i = 0;

    printf("\n==============================================\n");
    for (i = 0; i < sizeof(g_testcaseArray) / sizeof(g_testcaseArray[0]); i++)
    {
        printf("%d: %s\n", i, g_testcaseArray[i].p_description);
    }
}

static void RunTestcase(int id)
{
    if (id < 0 || id >= sizeof(g_testcaseArray) / sizeof(g_testcaseArray[0]))
    {
        printf("Wrong id:%d.\n", id);
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

CdataBool IntLtListData(void* p_nodeData, void* p_userData)
{
    int userData = *(int*)p_userData;
    int nodeData = *(int*)p_nodeData;

    return userData < nodeData;
}

static void TraverseIntList(ListTraverseNodeInfo_t *p_info, void *p_userData, CdataBool* p_needStop)
{
    int data = *((int *)p_info->p_data);
    int index = (int)p_info->index;

    printf("%d:%d\n", index, data);
    return;
}

static int ShowIntList(List_t list)
{
    printf("=============%s Begin==================\n", List_Name(list));
    List_Traverse(list, NULL, TraverseIntList);
    printf("=============%s End==================\n", List_Name(list));

	return 0;
}

static int ShowIntListReversely(List_t list)
{
    printf("=============%s Begin==================\n", List_Name(list));
    List_TraverseReversely(list, NULL, TraverseIntList);
    printf("=============%s End==================\n", List_Name(list));

	return 0;
}

static int TestNormalList()
{
    printf("\n==>I will test normal data.\n");

	int *p_data = NULL;
    int i = 0;
    int value = 0;
    int count = 10;


    for (i = 0; i < count; i++)
    {
        value = i + 1;
        List_InsertData(g_normalList, &value);
    }
    printf("\n==>After inster data:\n");
	ShowIntList(g_normalList);
	printf("\n==>Show int list reversely:\n");
    ShowIntListReversely(g_normalList);

	p_data = (int*)List_GetDataAtPos(g_normalList, 0);
	if (p_data == NULL)
	{
		LOG_E("Fail to get data.\n");
		return -1;
	}

	printf("Get data:%d at position 0.\n", *p_data);
    List_Clear(g_normalList);
    printf("After clear, list count:%d.\n", (int)List_Count(g_normalList));

	//Test List_InsertData2Head
	count = 10;
    LOG_A("count:%d.\n", count);
    for (i = 0; i < count; i++)
    {
        value = i + 1;
        List_InsertData2Head(g_normalList, &value);
    }
	ShowIntList(g_normalList);
	List_Clear(g_normalList);

	//Test List_InsertDataAscently
    printf("\n==>I will test inserting normal data ascently.\n");
	count = 10;
    LOG_A("count:%d.\n", count);
    for (i = 0; i < count; i++)
    {
        value = i + 1;
        List_InsertDataAscently(g_normalList, &value);
    }
	ShowIntList(g_normalList);
	List_Clear(g_normalList);

	//Test List_InsertDataDescently
    printf("\n==>I will test inserting normal data descently.\n");
	count = 10;
    LOG_A("count:%d.\n", count);
    for (i = 0; i < count; i++)
    {
        value = i + 1;
        List_InsertDataDescently(g_normalList, &value);
    }
    ShowIntList(g_normalList);
	List_Clear(g_normalList);
    return 0;
}

static int TestInsertDataAtPos()
{
	int errRet = 0;
	int ret = ERR_OK;

	int i = 0;
    int value = 0;
	ListNode_t node = NULL;

	for (i = 0; i < 5; i++)
	{
		value = i + 1;
		node = List_InsertData(g_normalList, &value);
		if (node == NULL)
		{
			LOG_E("Fail to insert data.\n");
		}
	}

	ShowIntList(g_normalList);
	LOG_A("List count:%d.\n", (int)List_Count(g_normalList));

	value = 100;
	LOG_A("Wil insert %d at 0.\n", value);
	node = List_InsertDataAtPos(g_normalList, &value, 0);
    if (node == NULL)
    {
        LOG_E("Fail to insert at position.\n");
        errRet = -1;
		goto EXIT;
    }
	ShowIntList(g_normalList);
	LOG_A("List count:%d.\n", (int)List_Count(g_normalList));

	value = 100;
	LOG_A("Wil insert %d at %d.\n", value, (int)(List_Count(g_normalList) - 1));
	node = List_InsertDataAtPos(g_normalList, &value, List_Count(g_normalList) - 1);
    if (node == NULL)
    {
        LOG_E("Fail to insert at position.\n");
        errRet = -1;
		goto EXIT;
    }

	ShowIntList(g_normalList);

	value = 100;
	LOG_A("Wil insert %d at 2.\n", value);
	node = List_InsertDataAtPos(g_normalList, &value, 2);
    if (node == NULL)
    {
        LOG_E("Fail to insert at position.\n");
        errRet = -1;
		goto EXIT;
    }

	ShowIntList(g_normalList);

	errRet = 0;
	EXIT:
	ret = List_Clear(g_normalList);
    if (ret != ERR_OK)
    {
        LOG_E("Fail to destroy list.\n");
        return -1;
    }

    return errRet;
}

static void TraverseStopIntList(ListTraverseNodeInfo_t *p_info, void *p_userData, CdataBool* p_needStop)
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

static int TestStopTraverseNormalData()
{
    printf("\n==>I will test stop traverse in a normal data list.\n");

    int userData = 50;
	int i = 0;
	int value = 0;

	for (i = 0; i < 10; i++)
	{
		value = random() % 100;
		List_InsertData(g_normalList, &value);
	}

    printf("All data in the list:\n");
    ShowIntList(g_normalList);

    printf("\nWill find the data which < userData:%d.\n", userData);
	List_Traverse(g_normalList, &userData, TraverseStopIntList);

	printf("\nWill find the data which < userData:%d reversely.\n", userData);
	List_TraverseReversely(g_normalList, &userData, TraverseStopIntList);

    List_Clear(g_normalList);

    return 0;
}

static int TestSwapPos()
{
	int errRet = 0;
    int ret = ERR_OK;
    int value = 0;
	ListNode_t node = NULL;

	value = 1;
	node = List_InsertData(g_normalList, &value);
	if (node == NULL)
	{
	    LOG_E("Fail to insert data.\n");
        errRet = -1;
		goto EXIT;
	}

	ListNode_t node1;
	value = 2;
	ret = List_CreateNode(g_normalList, &value, &node1);
	if (ret != ERR_OK)
	{
	    LOG_E("Fail to create node.\n");
        errRet = -1;
		goto EXIT;
	}
	LOG_A("node1:%p, next:%p, pre:%p.\n", node1, List_GetNextNode(g_normalList, node1), List_GetPreNode(g_normalList, node1));

	ret = List_InsertNode(g_normalList, node1);
	if (ret != ERR_OK)
	{
	    LOG_E("Fail to insert node.\n");

		List_DestroyNode(node1, NULL);
        errRet = -1;
		goto EXIT;
	}
	LOG_A("node1:%p, next:%p, pre:%p.\n", node1, List_GetNextNode(g_normalList, node1), List_GetPreNode(g_normalList, node1));

	value = 3;
	node = List_InsertData(g_normalList, &value);
	if (node == NULL)
	{
	    LOG_E("Fail to insert data.\n");
        errRet = -1;
		goto EXIT;
	}
	LOG_A("node1:%p, next:%p, pre:%p.\n", node1, List_GetNextNode(g_normalList, node1), List_GetPreNode(g_normalList, node1));

	ListNode_t node2;
	value = 4;
	ret = List_CreateNode(g_normalList, &value, &node2);
	if (ret != ERR_OK)
	{
	    LOG_E("Fail to create node.\n");
        errRet = -1;
		goto EXIT;
	}
	LOG_A("node2:%p, next:%p, pre:%p.\n", node2, List_GetNextNode(g_normalList, node2), List_GetPreNode(g_normalList, node2));

	ret = List_InsertNode(g_normalList, node2);
	if (ret != ERR_OK)
	{
	    LOG_E("Fail to insert node.\n");

		List_DestroyNode(g_normalList, node2);
        errRet = -1;
		goto EXIT;
	}
	LOG_A("node2:%p, next:%p, pre:%p.\n", node2, List_GetNextNode(g_normalList, node2), List_GetPreNode(g_normalList, node2));

	value = 5;
	node = List_InsertData(g_normalList, &value);
	if (node == NULL)
	{
	    LOG_E("Fail to insert data.\n");
        errRet = -1;
		goto EXIT;
	}
	LOG_A("node2:%p, next:%p, pre:%p.\n", node2, List_GetNextNode(g_normalList, node2), List_GetPreNode(g_normalList, node2));

	LOG_A("Before swap position.\n");
	ShowIntList(g_normalList);

	List_SwapPos(g_normalList, node2, node1);

	LOG_A("After swap position.\n");
	ShowIntList(g_normalList);

	errRet = 0;

	EXIT:
    ret = List_Clear(g_normalList);
    if (ret != ERR_OK)
    {
        LOG_E("Fail to destroy list.\n");
        errRet = -1;
    }

	return errRet;
}

static int TestSwapHeadTail()
{
	int errRet = 0;
    int ret = ERR_OK;
    int value = 0;
	ListNode_t node = NULL;

	ListNode_t node1;
	value = 1;
	ret = List_CreateNode(g_normalList, &value, &node1);
	if (ret != ERR_OK)
	{
	    LOG_E("Fail to create node.\n");
        errRet = -1;
		goto EXIT;
	}
	LOG_A("node1:%p, next:%p, pre:%p.\n", node1, List_GetNextNode(g_normalList, node1), List_GetPreNode(g_normalList, node1));

	ret = List_InsertNode(g_normalList, node1);
	if (ret != ERR_OK)
	{
	    LOG_E("Fail to insert node.\n");

		List_DestroyNode(node1, NULL);
        errRet = -1;
		goto EXIT;
	}

	value = 2;
	node = List_InsertData(g_normalList, &value);
	if (node == NULL)
	{
	    LOG_E("Fail to insert data.\n");
        errRet = -1;
		goto EXIT;
	}
	LOG_A("node1:%p, next:%p, pre:%p.\n", node1, List_GetNextNode(g_normalList, node1), List_GetPreNode(g_normalList, node1));

	value = 3;
	node = List_InsertData(g_normalList, &value);
	if (node == NULL)
	{
	    LOG_E("Fail to insert data.\n");
        errRet = -1;
		goto EXIT;
	}

	ListNode_t node2;
	value = 4;
	ret = List_CreateNode(g_normalList, &value, &node2);
	if (ret != ERR_OK)
	{
	    LOG_E("Fail to create node.\n");
        errRet = -1;
		goto EXIT;
	}
	LOG_A("node2:%p, next:%p, pre:%p.\n", node2, List_GetNextNode(g_normalList, node2), List_GetPreNode(g_normalList, node2));

	ret = List_InsertNode(g_normalList, node2);
	if (ret != ERR_OK)
	{
	    LOG_E("Fail to insert node.\n");

		List_DestroyNode(node2, NULL);
        errRet = -1;
		goto EXIT;
	}
	LOG_A("node2:%p, next:%p, pre:%p.\n", node2, List_GetNextNode(g_normalList, node2), List_GetPreNode(g_normalList, node2));

	LOG_A("Before swap position.\n");
	ShowIntList(g_normalList);

	List_SwapPos(g_normalList, node2, node1);

	LOG_A("After swap position.\n");
	ShowIntList(g_normalList);

	errRet = 0;

	EXIT:
    ret = List_Clear(g_normalList);
    if (ret != ERR_OK)
    {
        LOG_E("Fail to destroy list.\n");
        errRet = -1;
    }

	return errRet;
}

static int TestSwapNeighbour()
{
	int errRet = 0;

    int ret = ERR_OK;
    int value = 0;
	ListNode_t node = NULL;

	value = 1;
	node = List_InsertData(g_normalList, &value);
	if (node == NULL)
	{
	    LOG_E("Fail to insert data.\n");
        errRet = -1;
		goto EXIT;
	}

	ListNode_t node1;
	value = 2;
	ret = List_CreateNode(g_normalList, &value, &node1);
	if (ret != ERR_OK)
	{
	    LOG_E("Fail to create node.\n");
        errRet = -1;
		goto EXIT;
	}
	LOG_A("node1:%p, next:%p, pre:%p.\n", node1, List_GetNextNode(g_normalList, node1), List_GetPreNode(g_normalList, node1));

	ret = List_InsertNode(g_normalList, node1);
	if (ret != ERR_OK)
	{
	    LOG_E("Fail to insert node.\n");

		List_DestroyNode(node1, NULL);
        errRet = -1;
		goto EXIT;
	}
	LOG_A("node1:%p, next:%p, pre:%p.\n", node1, List_GetNextNode(g_normalList, node1), List_GetPreNode(g_normalList, node1));

	ListNode_t node2;
	value = 3;
	ret = List_CreateNode(g_normalList, &value, &node2);
	if (ret != ERR_OK)
	{
	    LOG_E("Fail to create node.\n");
        errRet = -1;
		goto EXIT;
	}
	LOG_A("node2:%p, next:%p, pre:%p.\n", node2, List_GetNextNode(g_normalList, node2), List_GetPreNode(g_normalList, node2));

	ret = List_InsertNode(g_normalList, node2);
	if (ret != ERR_OK)
	{
	    LOG_E("Fail to insert node.\n");

		List_DestroyNode(node2, NULL);
        errRet = -1;
		goto EXIT;
	}
	LOG_A("node2:%p, next:%p, pre:%p.\n", node2, List_GetNextNode(g_normalList, node2), List_GetPreNode(g_normalList, node2));
	LOG_A("node1:%p, next:%p, pre:%p.\n", node1, List_GetNextNode(g_normalList, node1), List_GetPreNode(g_normalList, node1));

	value = 4;
	node = List_InsertData(g_normalList, &value);
	if (node == NULL)
	{
	    LOG_E("Fail to insert data.\n");
        errRet = -1;
		goto EXIT;
	}

	LOG_A("Before swap position.\n");
	ShowIntList(g_normalList);

	List_SwapPos(g_normalList, node2, node1);

	LOG_A("After swap position.\n");
	ShowIntList(g_normalList);

	errRet = 0;

	EXIT:
    ret = List_Clear(g_normalList);
    if (ret != ERR_OK)
    {
        LOG_E("Fail to destroy list.\n");
        errRet = -1;
    }

	return errRet;
}

static int TestSwapNeighbourHeadTail()
{
	int errRet = 0;

    int ret = ERR_OK;
    int value = 0;

	ListNode_t node1;
	value = 1;
	ret = List_CreateNode(g_normalList, &value, &node1);
	if (ret != ERR_OK)
	{
	    LOG_E("Fail to create node.\n");
        errRet = -1;
		goto EXIT;
	}
	LOG_A("node1:%p, next:%p, pre:%p.\n", node1, List_GetNextNode(g_normalList, node1), List_GetPreNode(g_normalList, node1));

	ret = List_InsertNode(g_normalList, node1);
	if (ret != ERR_OK)
	{
	    LOG_E("Fail to insert node.\n");

		List_DestroyNode(node1, NULL);
        errRet = -1;
		goto EXIT;
	}
	LOG_A("node1:%p, next:%p, pre:%p.\n", node1, List_GetNextNode(g_normalList, node1), List_GetPreNode(g_normalList, node1));

	ListNode_t node2;
	value = 2;
	ret = List_CreateNode(g_normalList, &value, &node2);
	if (ret != ERR_OK)
	{
	    LOG_E("Fail to create node.\n");
        errRet = -1;
		goto EXIT;
	}
	LOG_A("node2:%p, next:%p, pre:%p.\n", node2, List_GetNextNode(g_normalList, node2), List_GetPreNode(g_normalList, node2));

	ret = List_InsertNode(g_normalList, node2);
	if (ret != ERR_OK)
	{
	    LOG_E("Fail to insert node.\n");

		List_DestroyNode(g_normalList, node2);
        errRet = -1;
		goto EXIT;
	}
	LOG_A("node2:%p, next:%p, pre:%p.\n", node2, List_GetNextNode(g_normalList, node2), List_GetPreNode(g_normalList, node2));
	LOG_A("node1:%p, next:%p, pre:%p.\n", node1, List_GetNextNode(g_normalList, node1), List_GetPreNode(g_normalList, node1));

	LOG_A("Before swap position.\n");
	ShowIntList(g_normalList);

	List_SwapPos(g_normalList, node1, node2);

	LOG_A("After swap position.\n");
	ShowIntList(g_normalList);

	errRet = 0;

	EXIT:
	List_Clear(g_normalList);

	return errRet;
}

static Student_t* CreateStudent(const char* p_name,
								char sex,
								int age,
								int chineseScore,
								int mathScore,
								int englishScore)
{
	if (p_name == NULL)
	{
		LOG_E("p_name is NULL.\n");
		return NULL;
	}

	Student_t *p_student = (Student_t *)malloc(sizeof(Student_t));
	if (p_student == NULL)
	{
		LOG_E("Fail to allocate student.\n");
		return NULL;
	}

	char *p_stuName = (char*)malloc(strlen(p_name) + 1);
	if (p_stuName == NULL)
	{
		LOG_E("Fail to allocate name.\n");

		free(p_student);
		return NULL;
	}

	strcpy(p_stuName, p_name);
	p_student->p_name = p_stuName;

	p_student->sex = sex;
	p_student->age = age;
	p_student->chineseScore = chineseScore;
	p_student->mathScore = mathScore;
	p_student->englishScore = englishScore;

	return p_student;
}


static ListNode_t CreateStudentNode(const char* p_name,
								char sex,
								int age,
								int chineseScore,
								int mathScore,
								int englishScore)
{
	if (p_name == NULL)
	{
		LOG_E("p_name is NULL.\n");
		return NULL;
	}

	Student_t *p_student = CreateStudent(p_name, sex, age, chineseScore, mathScore, englishScore);
	if (p_student == NULL)
	{
		LOG_E("Create student failed.\n");
		return NULL;
	}

	ListNode_t node = NULL;
	if (List_CreateNode(g_studentList, p_student, &node) != ERR_OK)
	{
		LOG_E("Fail to create node.\n");

		free(p_student->p_name);
		free(p_student);
	}

	return node;
}

static void FreeStudentData(void* p_data)
{
	if (p_data == NULL)
	{
		LOG_E("p_data is NULL.\n");
		return ;
	}

	Student_t *p_student = (Student_t*)p_data;

	LOG_I("Free %s.\n", p_student->p_name);
	free(p_student->p_name);
	free(p_student);

	return ;
}

static void ShowStudent(ListTraverseNodeInfo_t* p_nodeInfo, void* p_userData, CdataBool* p_needStopTraverse)
{
	if (p_nodeInfo == NULL)
	{
		LOG_E("p_nodeInfo is NULL.\n");
		return ;
	}

	Student_t *p_student = (Student_t *)p_nodeInfo->p_data;
	int total = p_student->chineseScore + p_student->mathScore + p_student->englishScore;
	printf("Student %d, \tName:%s, \tsex:'%c', \tage:%d, \ttotal:%d, \tchinese:%d, \tmath:%d, \tenglish:%d.\n", (int)(p_nodeInfo->index + 1), p_student->p_name, p_student->sex, p_student->age, total, p_student->chineseScore, p_student->mathScore, p_student->englishScore);

	return ;
}

static int ShowStudentList(List_t list)
{
	printf("====================%s Begin============================\n", List_Name(list));
	List_Traverse(list, NULL, ShowStudent);
	printf("====================%s End============================\n\n", List_Name(list));
	return 0;
}

CdataBool TotalScoreDescent(void* p_listNodeData, void* p_userData)
{
	Student_t *p_data = (Student_t *)p_userData;
	Student_t *p_nodeData = (Student_t *)p_listNodeData;

	int total1 = p_data->chineseScore + p_data->mathScore + p_data->englishScore;
	int total2 = p_nodeData->chineseScore + p_nodeData->mathScore + p_nodeData->englishScore;

	return total1 < total2;
}
static int CreateStudentList()
{
	List_Clear(g_studentList);
	ListNode_t node = NULL;

	node = CreateStudentNode("Jack", 'M', 10, 78, 82, 67);
	List_InsertNodeDescently(g_studentList, node);

	node = CreateStudentNode("Jan", 'F', 10, 90, 96, 80);
	List_InsertNodeDescently(g_studentList, node);

	node = CreateStudentNode("Lily", 'F', 10, 87, 76, 80);
	List_InsertNodeDescently(g_studentList, node);

	node = CreateStudentNode("Jack", 'M', 11, 89, 93, 78);
	List_InsertNodeDescently(g_studentList, node);

	Student_t *p_student = CreateStudent("Alice", 'F', 12, 97, 88, 79);
	List_InsertDataDescently(g_studentList, p_student);

	node = CreateStudentNode("John", 'M', 11, 85, 79, 94);
	List_InsertNodeDescently(g_studentList, node);

	p_student = CreateStudent("John", 'M', 12, 62, 74, 76);
	List_InsertDataDescently(g_studentList, p_student);

	p_student = CreateStudent("Tom", 'M', 10, 93, 100, 91);
	List_InsertDataDescently(g_studentList, p_student);

	return 0;
}

typedef struct
{
	char name[256];
	char sex;
	int  age;
}Condition_t;

CdataBool FindJohn(void* p_nodeData, void* p_userData)
{
	Condition_t *p_condition = (Condition_t*)p_userData;
	Student_t *p_student = (Student_t *)p_nodeData;

	if (strcmp(p_condition->name, p_student->p_name) != 0)
	{
		return CDATA_FALSE;
	}

	if (p_condition->sex != p_student->sex)
	{
		return CDATA_FALSE;
	}

	if (p_condition->age != -1)
	{
		if (p_condition->age != p_student->age)
		{
			return CDATA_FALSE;
		}
	}

	return CDATA_TRUE;
}

CdataBool FindBySex(void* p_nodeData, void* p_userData)
{
	char *p_sex = (char*)p_userData;
	Student_t *p_student = (Student_t *)p_nodeData;

	if (*p_sex != p_student->sex)
	{
		return CDATA_FALSE;
	}

	return CDATA_TRUE;
}

static int TestNormalStructureList()
{
	Student_t student;
	Student_t *p_student = NULL;
	List_t  list;
	int total = 0;
	ListNode_t node = NULL;

	int ret = List_Create("StructureList", LIST_TYPE_DOUBLE_LINK, sizeof(Student_t), &list);
	if (ret != ERR_OK)
	{
		LOG_E("Fail to create list.\n");
		return -1;
	}
	List_SetFreeDataFunc(list, FreeStudentData);

	int nameMaxLen = 128;

	student.p_name = (char*)malloc(nameMaxLen);
	memset(student.p_name, 0, nameMaxLen);
	strcpy(student.p_name, "Jerry");
	student.sex = 'F';
	student.age = 12;
	student.chineseScore = 88;
	student.mathScore = 88;
	student.englishScore = 88;
	node = List_InsertData(list, &student);

	student.p_name = (char*)malloc(nameMaxLen);
	memset(student.p_name, 0, nameMaxLen);
	strcpy(student.p_name, "Jack");
	student.sex = 'M';
	student.age = 11;
	student.chineseScore = 79;
	student.mathScore = 88;
	student.englishScore = 92;
	node = List_InsertData(list, &student);

	student.p_name = (char*)malloc(nameMaxLen);
	memset(student.p_name, 0, nameMaxLen);
	strcpy(student.p_name, "Tom");
	student.sex = 'M';
	student.age = 13;
	student.chineseScore = 72;
	student.mathScore = 69;
	student.englishScore = 91;
	node = List_InsertData(list, &student);

	LOG_I("Count:%d.\n", (int)List_Count(list));

	printf("==>Show student list from head to tail:\n");
	printf("===============%s begin=============================\n", List_Name(list));
	FOR_EACH_IN_LIST(node, list)
	{
		p_student = (Student_t*)List_GetNodeData(list, node);

		total = p_student->chineseScore + p_student->mathScore + p_student->englishScore;
		printf("name:%s, sex:'%c', age:%d, total:%d, chinese:%d, math:%d, english:%d.\n", p_student->p_name, p_student->sex, p_student->age, total, p_student->chineseScore, p_student->mathScore, p_student->englishScore);
	}
	printf("===============%s begin=============================\n", List_Name(list));

	printf("\n\n==>Show student list from tail to head:\n");
	printf("===============%s begin=============================\n", List_Name(list));
	FOR_EACH_IN_DBLIST_REVERSE(node, list)
	{
		p_student = (Student_t*)List_GetNodeData(list, node);

		total = p_student->chineseScore + p_student->mathScore + p_student->englishScore;
		printf("name:%s, sex:'%c', age:%d, total:%d, chinese:%d, math:%d, english:%d.\n", p_student->p_name, p_student->sex, p_student->age, total, p_student->chineseScore, p_student->mathScore, p_student->englishScore);
	}
	printf("===============%s begin=============================\n", List_Name(list));

	List_Destory(list);

	return 0;
}

static int TestReferenceList()
{
	Student_t* p_student = NULL;

	CreateStudentList();
	ShowStudentList(g_studentList);

	Condition_t condition;

	memset(condition.name, 0, sizeof(condition.name));
	strcpy(condition.name, "Jack");
	condition.sex = 'M';
	condition.age = -1;

	// Test List_GetMachCount
	CdataCount_t count = 0;
	List_GetMachCountByCond(g_studentList, &condition, FindJohn, &count);
	LOG_A("Find boys with name '%s', count:%lu.\n", condition.name, count);

	//Test List_DataExistsByCond
	memset(condition.name, 0, sizeof(condition.name));
	strcpy(condition.name, "John");
	condition.age = 11;
	if (List_DataExistsByCond(g_studentList, &condition, FindJohn))
	{
		LOG_A("Find John with age:%d.\n", condition.age);
	}

	condition.age = 14;
	if (List_DataExistsByCond(g_studentList, &condition, FindJohn))
	{
		LOG_A("Find John with age:%d.\n", condition.age);
	}
	else
	{
		LOG_A("Not find John with age:%d.\n", condition.age);
	}

	//Test get node by condition
	condition.age = 11;
	LOG_A("Condition: name:%s, sex:'%c', age:%d.\n", condition.name, condition.sex, condition.age);
	ListNode_t node = List_GetFirstMatchNodeByCond(g_studentList, &condition, FindJohn);
	if (node != NULL)
	{
		p_student = (Student_t *)List_GetNodeData(g_studentList, node);
		LOG_A("Find John's information: chineseScore:%d, math:%d, english:%d.\n", p_student->chineseScore, p_student->mathScore, p_student->englishScore);
	}

	p_student = CreateStudent("BeforeStu", 'M', 12, 80, 98, 90);
	List_InsertDataBefore(g_studentList, node, p_student);
	LOG_A("After insert before:\n");
	ShowStudentList(g_studentList);

	p_student = CreateStudent("AfterStu", 'M', 13, 89, 70, 94);
	List_InsertDataAfter(g_studentList, node, p_student);
	LOG_A("After insert after:\n");
	ShowStudentList(g_studentList);

	List_Clear(g_studentList);
	return 0;
}

static int TestDetach()
{
	Student_t* p_student = NULL;
	ListNode_t node = NULL;

	CreateStudentList();
	ShowStudentList(g_studentList);

	Condition_t condition;

	memset(condition.name, 0, sizeof(condition.name));
	strcpy(condition.name, "Jack");
	condition.sex = 'M';
	condition.age = 11;
	LOG_A("Condition: name:%s, sex:'%c', age:%d.\n", condition.name, condition.sex, condition.age);
	node = List_GetFirstMatchNodeByCond(g_studentList, &condition, FindJohn);
	if (node != NULL)
	{
		p_student = (Student_t *)List_GetNodeData(g_studentList, node);

		List_DetachNode(g_studentList, node);
		LOG_A("After detach stduent(Name:%s, sex:'%c', age:%d) from student list.\n", p_student->p_name, p_student->sex, p_student->age);
		ShowStudentList(g_studentList);
		List_DestroyNode(g_studentList, node);
	}

	node = List_DetachNodeAtPos(g_studentList, 1);
	p_student = (Student_t*)List_DetachNodeData(g_studentList, node);
	List_DestroyNode(g_studentList, node);
	LOG_A("After detach stduent(Name:%s, sex:'%c', age:%d) from student list at position 1.\n", p_student->p_name, p_student->sex, p_student->age);
	FreeStudentData(p_student);
	ShowStudentList(g_studentList);

	LOG_A("Will detach data by condition.\n");
	char sex = 'M';
	p_student = List_DetachDataByCond(g_studentList, &sex, FindBySex);
	LOG_A("After detach first 'M', name:%s, age:%d.\n", p_student->p_name, p_student->age);
	ShowStudentList(g_studentList);
	FreeStudentData(p_student);

	LOG_A("Will detach head.\n");
	node = List_DetachHead(g_studentList);
	p_student = (Student_t*)List_GetNodeData(g_studentList, node);
	LOG_A("Detach head:%s, '%c', %d.\n", p_student->p_name, p_student->sex, p_student->age);
	List_DestroyNode(g_studentList, node);
	LOG_A("After detach head.\n");
	ShowStudentList(g_studentList);

	node = List_DetachTail(g_studentList);
	p_student = List_GetNodeData(g_studentList, node);
	LOG_A("Detach tail:%s, '%c', %d.\n", p_student->p_name, p_student->sex, p_student->age);
	List_DestroyNode(g_studentList, node);
	LOG_A("After detach tail.\n");
	ShowStudentList(g_studentList);

	List_Clear(g_studentList);
	return 0;
}

static int TestRm()
{
	Student_t* p_student = NULL;
	ListNode_t node = NULL;

	CreateStudentList();
	ShowStudentList(g_studentList);

	Condition_t condition;
	memset(condition.name, 0, sizeof(condition.name));
	strcpy(condition.name, "Jack");
	condition.sex = 'M';
	condition.age = 11;
	LOG_A("Condition: name:%s, sex:'%c', age:%d.\n", condition.name, condition.sex, condition.age);
	node = List_GetFirstMatchNodeByCond(g_studentList, &condition, FindJohn);
	if (node != NULL)
	{
		p_student = (Student_t *)List_GetNodeData(g_studentList, node);
		LOG_A("After rm stduent(Name:%s, sex:'%c', age:%d) from student list.\n", p_student->p_name, p_student->sex, p_student->age);

		List_RmNode(g_studentList, node);
		ShowStudentList(g_studentList);
	}


	List_RmHead(g_studentList);
	LOG_A("After rm head:\n");
	ShowStudentList(g_studentList);

	List_RmTail(g_studentList);
	LOG_A("After rm tail:\n");
	ShowStudentList(g_studentList);

	List_RmNodeAtPos(g_studentList, 1);
	LOG_A("After rm from position 1:\n");
	ShowStudentList(g_studentList);

	char sex = 'M';
	List_RmFirstMatchNodeByCond(g_studentList, &sex, FindBySex);
	LOG_A("After rm firt match sex 'M':\n");
	ShowStudentList(g_studentList);

	sex = 'F';
	List_RmAllMatchNodesByCond(g_studentList, &sex, FindBySex);
	LOG_A("After rm all match sex 'F':\n");
	ShowStudentList(g_studentList);

	List_Clear(g_studentList);

	return 0;
}

static int TestMatch()
{
	Student_t* p_student = NULL;
	ListNode_t node = NULL;
	CreateStudentList();
	ShowStudentList(g_studentList);

	LOG_A("Find all 'M' from head to tail.\n");
	char sex = 'M';
	for (node = List_GetFirstMatchNodeByCond(g_studentList, &sex, FindBySex); node != NULL;)
	{
		p_student = (Student_t*)List_GetNodeData(g_studentList, node);
		LOG_A("Find %s, '%c', %d.\n", p_student->p_name, p_student->sex, p_student->age);

		node = List_GetNextNode(g_studentList, node);
		node = List_GetNextMatchNodeByCond(g_studentList, node, &sex, FindBySex);
	}

	LOG_A("Find all 'M' from tail to head.\n");
	for (node = List_GetLastMatchNodeByCond(g_studentList, &sex, FindBySex); node != NULL;)
	{
		p_student = (Student_t*)List_GetNodeData(g_studentList, node);
		LOG_A("Find %s, '%c', %d.\n", p_student->p_name, p_student->sex, p_student->age);

		node = List_GetPreNode(g_studentList, node);
		node = List_GetPreMatchNodeByCond(g_studentList, node, &sex, FindBySex);
	}

	List_Clear(g_studentList);

	return 0;
}

static void* InsertThread(void *p_param)
{
	int count = 0;
	Student_t *p_student = NULL;
	char name[64];

	for (count  = 0; count < 10; count++)
	{
		memset(name, 0, 64);
		sprintf(name, "Student-%d", count + 1);
		p_student = CreateStudent(name, 'M', 12, 90, 83, 81);

		LOG_A("Insert:'%s'.\n", name);
		List_InsertData(g_studentList, p_student);
		sleep(1);
	}
	return NULL;
}

static void* FreeThread(void *p_param)
{
	int count = 0;
	Student_t *p_student;

	for (count  = 0; count < 15; count++)
	{
		p_student = List_GetHeadData(g_studentList);
		if (p_student != NULL)
		{
			LOG_A("Free student:'%s'.\n", p_student->p_name);
		}
		else
		{
			LOG_A("p_student is NULL.\n");
		}
		List_RmHead(g_studentList);
		sleep(1);
	}

	return NULL;
}

static int TestMultiThread()
{
	pthread_t id1;
	pthread_t id2;

	List_Clear(g_studentList);

	pthread_create(&id1, NULL, InsertThread, NULL);
	pthread_create(&id2, NULL, FreeThread, NULL);

	pthread_join(id1, NULL);
	pthread_join(id2, NULL);

	return 0;
}

static void* WriteThread(void *p_param)
{
	List_t list = *((List_t*)(p_param));
	Student_t student;

	int i = 0;
	for (i = 0; i < 10; i++)
	{
		student.p_name = (char*)malloc(64);
		memset(student.p_name, 0, 64);
		sprintf(student.p_name, "Student-%d", i + 1);
		student.sex = 'F';
		student.age = 12;

		LOG_A("Insert %s.\n", student.p_name);
		List_InsertData(list, &student);

		sleep(1);
	}
	return NULL;
}

static void* ReadThread(void *p_param)
{
	List_t list = *((List_t*)(p_param));

	ListNode_t node = NULL;
	Student_t *p_student = NULL;
	int i = 0;

	for (i = 0; i < 11; i++)
	{
		LOG_A("Student list:\n");

		List_Lock(list);
		FOR_EACH_IN_LIST(node, list)
		{
			p_student = List_GetNodeDataNL(list, node);
			printf("%s\n", p_student->p_name);
		}
		List_UnLock(list);

		printf("\n");
		sleep(2);
	}
	return NULL;
}

static int TestMultiThreadLock()
{
	pthread_t id1;
	pthread_t id2;

	List_t list;
	List_Create("TestList", LIST_TYPE_DOUBLE_LINK, sizeof(Student_t), &list);

	pthread_create(&id1, NULL, WriteThread, &list);
	pthread_create(&id2, NULL, ReadThread, &list);

	pthread_join(id1, NULL);
	pthread_join(id2, NULL);

	List_Destory(list);
	return 0;
}
