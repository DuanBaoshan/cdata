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
extern TestcaseSet_t ListTestcaseSet;
extern TestcaseSet_t QueueTestcaseSet;

//=============================================================================
static void ShowRootMenu(List_t rootMenuList);
static TestcaseSet_t* GetAndShowSubMenu(List_t rootMenuList, int subMenuId);
static void Traverse(ListTraverseNodeInfo_t * p_info,void * p_userData,CdataBool * p_needStop);
static void FreeData(void *p_data);
//=============================================================================
int main(int argc, char* argv[])
{
    char choice[128];
    int id = 0;
    int curState = 0;
    int testcaseCount = 0;
    int isValidId = 0;

    List_t testcaseList;
    TestcaseSet_t *p_curTestcase = NULL;

    List_CreateRef("TestcaseSetList", LIST_TYPE_SINGLE_LINK, &testcaseList);
    List_SetFreeDataFunc(testcaseList, FreeData);
    List_InsertData(testcaseList, &ListTestcaseSet);
    List_InsertData(testcaseList, &QueueTestcaseSet);

    while (1)
    {
        if (curState == 0)
        {
            ShowRootMenu(testcaseList);
        }
        else if (curState == 1)
        {
            if (p_curTestcase == NULL)
            {
                p_curTestcase = GetAndShowSubMenu(testcaseList, id);
            }
            else
            {
                if (isValidId)
                {
                    p_curTestcase->Run(id);
                }
                p_curTestcase->Show();
            }
        }


        printf("Please select id('q' for quit):");
        scanf("%s", choice);
        if (strcmp(choice, "q") == 0)
        {
            if (curState == 0)
            {
                break;
            }

            if (p_curTestcase != NULL)
            {
                p_curTestcase->Finalize();
                p_curTestcase = NULL;
            }

            curState = 0;
        }
        else
        {
            id = atoi(choice);

            testcaseCount = (p_curTestcase == NULL) ? List_Count(testcaseList) : p_curTestcase->GetTestcaseCount();
            if (id < 0 || id >= testcaseCount)
            {
                printf("Invalid id:%d.\n", id);
                isValidId = 0;
            }
            else
            {
                isValidId = 1;
                curState = 1;
            }

        }

    }

    List_Destroy(testcaseList);

    return 0;
}
//=============================================================================
static void ShowRootMenu(List_t rootMenuList)
{
    printf("\n================================================\n");
    List_Traverse(rootMenuList, NULL, Traverse);
    printf("================================================\n");
}

static TestcaseSet_t* GetAndShowSubMenu(List_t rootMenuList, int subMenuId)
{
    TestcaseSet_t *p_curTestcase = (TestcaseSet_t*)List_GetDataAtPos(rootMenuList, subMenuId);

    if (p_curTestcase == NULL)
    {
        LOG_E("Wrong testcase id:%d.\n", subMenuId);
        return NULL;
    }

    p_curTestcase->Init();
    p_curTestcase->Show();

    return p_curTestcase;
}

static void Traverse(ListTraverseNodeInfo_t *p_info, void * p_userData, CdataBool * p_needStop)
{
    TestcaseSet_t *p_testcaseSet = (TestcaseSet_t*)p_info->p_data;

    printf("%d:%s\n", (int)p_info->index, p_testcaseSet->p_name);
}

static void FreeData(void *p_data)
{
    //Do nothing.
    return;
}


