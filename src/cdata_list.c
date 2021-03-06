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
Date:2019.4.2
*/

#include "cdata_types.h"
#include "cdata_list.h"
#include "list_internal.h"
#include "cdata_dblist.h"
#include "cdata_sglist.h"

#ifndef _DEBUG_LEVEL_
#define _DEBUG_LEVEL_  _DEBUG_LEVEL_I_
#endif
#include "debug.h"


/*=============================================================================*
 *                        Macro definition
 *============================================================================*/

/*=============================================================================*
 *                        Const definition
 *============================================================================*/

/*=============================================================================*
 *                    New type or enum declaration
 *============================================================================*/


/*=============================================================================*
 *                    Inner function declaration
 *============================================================================*/
static List_t      CreateList(ListName_t name, ListType_e type, List_DataType_e dataType, int dataLength);
static OSMutex_t   CreateGuard();
static void        DeleteGuard(OSMutex_t guard);
static CdataBool   HasDuplicateNode(List_t list, ListNode_t node);
 /*=============================================================================*
 *                    Outer function implemention
 *============================================================================*/

//==============================================================================
//                    List related functions
//==============================================================================
int List_Create(ListName_t name, ListType_e type, int dataLength, List_t* p_list)
{
    CHECK_PARAM(p_list != NULL, ERR_BAD_PARAM);

	List_t list = NULL;

	list = CreateList(name, type, LIST_DATA_TYPE_VALUE_COPY, dataLength);
	if (list == NULL)
	{
		LOG_E("Fail to create list:'%s'.\n", name);
		return ERR_FAIL;
	}

	*p_list = list;

    LOG_I("Success to create:'%s'.\n", name);

    return ERR_OK;
}

int List_CreateRef(ListName_t name, ListType_e type, List_t* p_list)
{
    CHECK_PARAM(p_list != NULL, ERR_BAD_PARAM);

	List_t list = NULL;

	list = CreateList(name, type, LIST_DATA_TYPE_VALUE_REFERENCE, 0);
	if (list == NULL)
	{
		LOG_E("Fail to create list:'%s'.\n", name);
		return ERR_FAIL;
	}

	*p_list = list;

    LOG_I("Success to create list:'%s'.\n", name);

    return ERR_OK;
}

int List_SetFreeDataFunc(List_t list, List_FreeData_fn freeFn)
{
	CHECK_PARAM(list != NULL, ERR_BAD_PARAM);

	List_st* p_list = CONVERT_2_LIST(list);
	p_list->freeFn = freeFn;

	return ERR_OK;
}

int List_SetEqual2KeywordFunc(List_t list, List_Equal2Keyword_fn equal2KeywordFn)
{
	CHECK_PARAM(list != NULL, ERR_BAD_PARAM);

	List_st* p_list = CONVERT_2_LIST(list);
	p_list->equal2KeywordFn = equal2KeywordFn;

	return ERR_OK;
}

int List_SetNodeEqualFunc(List_t list, List_NodeEqual_fn nodeEqualFn)
{
	CHECK_PARAM(list != NULL, ERR_BAD_PARAM);

	List_st* p_list = CONVERT_2_LIST(list);
	p_list->nodeEqualFn = nodeEqualFn;

	return ERR_OK;
}

int List_SetUserLtNodeFunc(List_t list, List_UserLtNode_fn userLtNodeFn)
{
	CHECK_PARAM(list != NULL, ERR_BAD_PARAM);

	List_st* p_list = CONVERT_2_LIST(list);
	p_list->usrLtNodeFn = userLtNodeFn;

	return ERR_OK;
}

const char* List_Name(List_t list)
{
	CHECK_PARAM(list != NULL, NULL);

	List_st* p_list = CONVERT_2_LIST(list);
	return (const char*)p_list->name;
}

CdataCount_t  List_Count(List_t list)
{
    CHECK_PARAM(list != NULL, 0);

	List_st* p_list = CONVERT_2_LIST(list);
    CdataCount_t count = 0;

    List_Lock(list);
    count = p_list->nodeCount;
    List_UnLock(list);

    return count;
}

void List_Lock(List_t list)
{
    if (list == NULL)
    {
        LOG_E("list is NULL.\n");
        return;
    }

    List_st* p_list = CONVERT_2_LIST(list);
    if (OS_MutexLock(p_list->guard) != 0)
    {
        LOG_E("Fail to lock dblist:'%s'.\n", p_list->name);
    }

	return;
}

void List_UnLock(List_t list)
{
    if (list == NULL)
    {
        LOG_E("list is NULL.\n");
        return;
    }

    List_st* p_list = CONVERT_2_LIST(list);
    if (OS_MutexUnlock(p_list->guard) != 0)
    {
        LOG_E("Fail to unlock dblist:'%s'.\n", p_list->name);
    }

	return;
}

int List_Traverse(List_t list, void *p_userData, List_Traverse_fn traverseFn)
{
    CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(traverseFn != NULL, ERR_BAD_PARAM);

    List_st*      			p_list 	 = CONVERT_2_LIST(list);
	void*    				p_head 	 = NULL;
	CdataIndex_t   			pos    	 = 0;
	CdataBool  				needStop = CDATA_FALSE;
	ListTraverseNodeInfo_t 	info;

	List_Lock(list);
	for (p_head = p_list->p_head; p_head != NULL; p_head = List_GetNextNodeNL(list, p_head), pos++)
	{
		info.index = pos;
		info.node = (ListNode_t)p_head;
		info.p_data = List_GetNodeDataNL(list, p_head);
		traverseFn(&info, p_userData, &needStop);
		if (needStop)
		{
			break;
		}
	}
	List_UnLock(list);

    return ERR_OK;
}

int List_TraverseReversely(List_t list, void *p_userData, List_Traverse_fn traverseFn)
{
    CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(traverseFn != NULL, ERR_BAD_PARAM);

    CdataIndex_t    pos    = 0;
    DBListNode_st*  p_tail = NULL;
    List_st*        p_list = CONVERT_2_LIST(list);

    ListTraverseNodeInfo_t info;
    CdataBool  needStop = CDATA_FALSE;

	if (p_list->type == LIST_TYPE_SINGLE_LINK)
	{
		LOG_E("Cannot traverse on single list reversely.\n");
		return ERR_BAD_PARAM;
	}

    List_Lock(list);
    for (p_tail = p_list->p_tail, pos = p_list->nodeCount - 1; p_tail != NULL; p_tail = p_tail->p_pre, pos--)
    {
        info.index = pos;
        info.node = (ListNode_t)p_tail;
        info.p_data = p_tail->p_data;

        traverseFn(&info, p_userData, &needStop);
        if (needStop)
        {
            break;
        }
    }
    List_UnLock(list);

    return ERR_OK;
}

int List_Clear(List_t list)
{
    CHECK_PARAM(list != NULL, ERR_BAD_PARAM);

	List_st*  	p_list = CONVERT_2_LIST(list);
	void* 		p_head = p_list->p_head;
	void* 		p_next = NULL;

	LOG_I("Clear '%s', nodeCount:%llu.\n", p_list->name, p_list->nodeCount);

	List_Lock(list);
	while (p_head != NULL)
	{
		p_next = List_GetNextNodeNL(list, p_head);
		List_DestroyNode(p_list, p_head);
		p_head = p_next;
	}
	p_list->nodeCount = 0;
	p_list->p_head = NULL;
	p_list->p_tail = NULL;

	List_UnLock(list);

    return ERR_OK;
}

int List_Destroy(List_t list)
{
    CHECK_PARAM(list != NULL, ERR_BAD_PARAM);

    List_st*     p_list = CONVERT_2_LIST(list);

    LOG_I("Destroy '%s'.\n", p_list->name);

    List_Clear(list);
    DeleteGuard(p_list->guard);
    OS_Free(p_list);

    return ERR_OK;
}
//=========================================================
//                   Data related functions
//=========================================================
ListNode_t List_InsertData(List_t list, void* p_data)
{
    CHECK_PARAM(list != NULL, NULL);
    CHECK_PARAM(p_data != NULL, NULL);

	int        ret  = ERR_OK;
	ListNode_t node = NULL;

	ret = List_CreateNode(list, p_data, &node);
	if (ret != ERR_OK)
	{
		LOG_E("Fail to create node.\n");
		return NULL;
	}

	ret = List_InsertNode(list, node);
	if (ret != ERR_OK)
	{
		LOG_E("Fail to insert node.\n");
		
		/*If insert failed, we cannot free the user's raw data, we only can destroy the new created node,
		 *so we should detach node data first.*/
		List_DetachNodeData(list, node);
		List_DestroyNode(list, node);
		return NULL;
	}

	return node;
}

ListNode_t List_InsertData2Head(List_t list, void* p_data)
{
    CHECK_PARAM(list != NULL, NULL);
    CHECK_PARAM(p_data != NULL, NULL);

	int      ret    = ERR_OK;
	ListNode_t node = NULL;

	ret = List_CreateNode(list, p_data, &node);
	if (ret != ERR_OK)
	{
		LOG_E("Fail to create node.\n");
		return NULL;
	}

	ret = List_InsertNode2Head(list, node);
	if (ret != ERR_OK)
	{
		LOG_E("Fail to insert node.\n");
		
		List_DetachNodeData(list, node);
		List_DestroyNode(list, node);
		return NULL;
	}

	return node;
}

ListNode_t List_InsertDataAsc(List_t list, void* p_data)
{
    CHECK_PARAM(list != NULL, NULL);
    CHECK_PARAM(p_data != NULL, NULL);

	int      ret    = ERR_OK;
	ListNode_t node = NULL;

	ret = List_CreateNode(list, p_data, &node);
	if (ret != ERR_OK)
	{
		LOG_E("Fail to create node.\n");
		return NULL;
	}

	ret = List_InsertNodeAsc(list, node);
	if (ret != ERR_OK)
	{
		LOG_E("Fail to insert node.\n");
		
		List_DetachNodeData(list, node);
		List_DestroyNode(list, node);
		return NULL;
	}

	return node;
}

ListNode_t List_InsertDataDes(List_t list, void* p_data)
{
    CHECK_PARAM(list != NULL, NULL);
    CHECK_PARAM(p_data != NULL, NULL);

	int      ret    = ERR_OK;
	ListNode_t node = NULL;

	ret = List_CreateNode(list, p_data, &node);
	if (ret != ERR_OK)
	{
		LOG_E("Fail to create node.\n");
		return NULL;
	}

	ret = List_InsertNodeDes(list, node);
	if (ret != ERR_OK)
	{
		LOG_E("Fail to insert node.\n");
		
		List_DetachNodeData(list, node);
		List_DestroyNode(list, node);
		return NULL;
	}

	return node;
}

ListNode_t List_InsertDataUni(List_t list, void* p_data)
{
    CHECK_PARAM(list != NULL, NULL);
    CHECK_PARAM(p_data != NULL, NULL);

	int        ret  = ERR_OK;
	ListNode_t node = NULL;

	ret = List_CreateNode(list, p_data, &node);
	if (ret != ERR_OK)
	{
		LOG_E("Fail to create node.\n");
		return NULL;
	}

	ret = List_InsertNodeUni(list, node);
	if (ret != ERR_OK)
	{
		LOG_E("Fail to insert node.\n");

        List_DetachNodeData(list, node);
		List_DestroyNode(list, node);
		return NULL;
	}

	return node;
}

ListNode_t List_InsertData2HeadUni(List_t list, void* p_data)
{
    CHECK_PARAM(list != NULL, NULL);
    CHECK_PARAM(p_data != NULL, NULL);

	int      ret    = ERR_OK;
	ListNode_t node = NULL;

	ret = List_CreateNode(list, p_data, &node);
	if (ret != ERR_OK)
	{
		LOG_E("Fail to create node.\n");
		return NULL;
	}

	ret = List_InsertNode2HeadUni(list, node);
	if (ret != ERR_OK)
	{
		LOG_E("Fail to insert node.\n");

        List_DetachNodeData(list, node);
		List_DestroyNode(list, node);
		return NULL;
	}

	return node;
}

ListNode_t List_InsertDataBefore(List_t list, void* p_keyword, void *p_data)
{
    CHECK_PARAM(list != NULL, NULL);
	CHECK_PARAM(p_keyword != NULL, NULL);
    CHECK_PARAM(p_data != NULL, NULL);

	ListNode_t newNode = NULL;

	List_Lock(list);
	newNode = List_InsertDataBeforeNL(list, p_keyword, p_data);
	List_UnLock(list);

	return newNode;
}
ListNode_t List_InsertDataBeforeNL(List_t list, void* p_keyword, void *p_data)
{
    CHECK_PARAM(list != NULL, NULL);
    CHECK_PARAM(p_keyword != NULL, NULL);
    CHECK_PARAM(p_data != NULL, NULL);

	int        ret     = ERR_OK;
	ListNode_t newNode = NULL;
    List_st*  p_list = CONVERT_2_LIST(list);
    
	if (p_list->equal2KeywordFn == NULL)
	{
		LOG_E("equal2KeywordFn is NULL, pls set a valid function first.\n");
		return NULL;
	}
	
	ret = List_CreateNode(list, p_data, &newNode);
	if (ret != ERR_OK)
	{
		LOG_E("Fail to create node.\n");
		return NULL;
	}
    
    if (p_list->type == LIST_TYPE_DOUBLE_LINK)
    {
        ret = DBList_InsertDataBefore(list, p_keyword, newNode);
    }
    else if (p_list->type == LIST_TYPE_SINGLE_LINK)
    {
        ret = SGList_InsertDataBefore(list, p_keyword, newNode);
    }
    else
    {
        LOG_E("Invalid list type:%d.\n", p_list->type);
        ret = ERR_FAIL;
    }
    
    if (ret != ERR_OK)
    {        
        List_DetachNodeData(list, newNode);
		List_DestroyNode(list, newNode);
		return NULL;    
    }

	return newNode;
}

ListNode_t List_InsertDataAfter(List_t list, void* p_keyword, void *p_data)
{
    CHECK_PARAM(list != NULL, NULL);
    CHECK_PARAM(p_data != NULL, NULL);

	ListNode_t newNode = NULL;

	List_Lock(list);
	newNode = List_InsertDataAfterNL(list, p_keyword, p_data);
	List_UnLock(list);

	return newNode;
}
ListNode_t List_InsertDataAfterNL(List_t list, void* p_keyword, void *p_data)
{
    CHECK_PARAM(list != NULL, NULL);
    CHECK_PARAM(p_keyword != NULL, NULL);
    CHECK_PARAM(p_data != NULL, NULL);

	int        ret     = ERR_OK;
	ListNode_t newNode = NULL;
    List_st*  p_list = CONVERT_2_LIST(list);
    
	if (p_list->equal2KeywordFn == NULL)
	{
		LOG_E("equal2KeywordFn is NULL, pls set a valid function first.\n");
		return NULL;
	}
	
	ret = List_CreateNode(list, p_data, &newNode);
	if (ret != ERR_OK)
	{
		LOG_E("Fail to create node.\n");
		return NULL;
	}
    
    if (p_list->type == LIST_TYPE_DOUBLE_LINK)
    {
        ret = DBList_InsertDataAfter(list, p_keyword, newNode);
    }
    else if (p_list->type == LIST_TYPE_SINGLE_LINK)
    {
        ret = SGList_InsertDataAfter(list, p_keyword, newNode);
    }
    else
    {
        LOG_E("Invalid list type:%d.\n", p_list->type);
        ret = ERR_FAIL;
    }
    
    if (ret != ERR_OK)
    {        
        List_DetachNodeData(list, newNode);
		List_DestroyNode(list, newNode);
		return NULL;    
    }

	return newNode;
}

ListNode_t List_InsertDataAtPos(List_t list, void* p_data, CdataIndex_t posIndex)
{
    CHECK_PARAM(list != NULL, NULL);
    CHECK_PARAM(p_data != NULL, NULL);

	int 	   ret     = ERR_OK;
	ListNode_t node    = NULL;

	ret = List_CreateNode(list, p_data, &node);
	if (ret != ERR_OK)
	{
		LOG_E("Fail to create node.\n");
		return NULL;
	}

	ret = List_InsertNodeAtPos(list, node, posIndex);
	if (ret != ERR_OK)
	{
		LOG_E("Fail to insert node.\n");
		
		List_DetachNodeData(list, node);
		List_DestroyNode(list, node);
		return NULL;
	}

	return node;
}

CdataBool List_DataExists(List_t list, void* p_keyword)
{
	CHECK_PARAM(list != NULL, CDATA_FALSE);
	CHECK_PARAM(p_keyword != NULL, CDATA_FALSE);

	CdataBool ret    = CDATA_FALSE;
	List_st*  p_list = CONVERT_2_LIST(list);
	void*     p_head = NULL;

	if (p_list->equal2KeywordFn == NULL)
	{
		LOG_E("equal2KeywordFn is NULL, pls set a valid function first.\n");
		return CDATA_FALSE;
	}

	List_Lock(list);
	for (p_head = p_list->p_head; p_head != NULL; p_head = List_GetNextNodeNL(list, p_head))
	{
		void *p_data = List_GetNodeDataNL(list, p_head);
		if (p_list->equal2KeywordFn(p_data, p_keyword))
		{
			ret = CDATA_TRUE;
			break;
		}
	}
	List_UnLock(list);

	return ret;
}

CdataBool List_DataExistsByCond(List_t list, void* p_userData, List_Condition_fn conditionFn)
{
    CHECK_PARAM(list != NULL, CDATA_FALSE);
    CHECK_PARAM(conditionFn != NULL, CDATA_FALSE);

	CdataBool ret    = CDATA_FALSE;
	List_st*  p_list = CONVERT_2_LIST(list);
	void*     p_head = NULL;

	List_Lock(list);
	for (p_head = p_list->p_head; p_head != NULL; p_head = List_GetNextNodeNL(list, p_head))
	{
		void *p_data = List_GetNodeDataNL(list, p_head);
		if (conditionFn(p_data, p_userData))
		{
			ret = CDATA_TRUE;
			break;
		}
	}
	List_UnLock(list);

	return ret;
}

void* List_GetHeadData(List_t list)
{
	CHECK_PARAM(list != NULL, NULL);

	void* p_data = NULL;

	List_Lock(list);
	p_data = List_GetHeadDataNL(list);
	List_UnLock(list);

	return p_data;
}
void* List_GetHeadDataNL(List_t list)
{
    CHECK_PARAM(list != NULL, NULL);

	List_st* p_list = CONVERT_2_LIST(list);
	void *   p_data = NULL;

    if (p_list->nodeCount == 0)
    {
        return NULL;
    }

	if (p_list->type == LIST_TYPE_DOUBLE_LINK)
	{
		DBListNode_st* p_node = (DBListNode_st*)p_list->p_head;
		p_data = p_node->p_data;
	}
	else if (p_list->type == LIST_TYPE_SINGLE_LINK)
	{
		SGListNode_st* p_node = (SGListNode_st*)p_list->p_head;
		p_data = p_node->p_data;
	}
	else
	{
		LOG_E("Invalid list type:%d.\n", p_list->type);
	}

	return p_data;
}

void* List_GetTailData(List_t list)
{
	CHECK_PARAM(list != NULL, NULL);

	void* p_data = NULL;

	List_Lock(list);
	p_data = List_GetTailDataNL(list);
	List_UnLock(list);

	return p_data;
}
void* List_GetTailDataNL(List_t list)
{
    CHECK_PARAM(list != NULL, NULL);

	List_st* p_list = CONVERT_2_LIST(list);
	void *   p_data = NULL;

    if (p_list->nodeCount == 0)
    {
        return NULL;
    }

	if (p_list->type == LIST_TYPE_DOUBLE_LINK)
	{
		DBListNode_st* p_node = (DBListNode_st*)p_list->p_tail;
		p_data = p_node->p_data;
	}
	else if (p_list->type == LIST_TYPE_SINGLE_LINK)
	{
		SGListNode_st* p_node = (SGListNode_st*)p_list->p_tail;
		p_data = p_node->p_data;
	}
	else
	{
		LOG_E("Invalid list type:%d.\n", p_list->type);
	}

	return p_data;
}

CdataCount_t List_GetMachCount(List_t list, void* p_keyword)
{
    CHECK_PARAM(list != NULL, 0);
    CHECK_PARAM(p_keyword != NULL, 0);

	List_st* 	 p_list = CONVERT_2_LIST(list);
	void *	 	 p_head = NULL;
	void *		 p_data = NULL;
	CdataCount_t count  = 0;

	if (p_list->equal2KeywordFn == NULL)
	{
		LOG_E("equal2KeywordFn is NULL, pls set a valid equal2KeywordFn first.\n");
		return 0;
	}

	List_Lock(list);
	for (p_head = p_list->p_head; p_head != NULL; p_head = List_GetNextNodeNL(list, p_head))
	{
		p_data = List_GetNodeDataNL(list, p_head);
		if (p_list->equal2KeywordFn(p_data, p_keyword))
		{
			count++;
		}
	}
	List_UnLock(list);

	return count;
}

CdataCount_t List_GetMachCountByCond(List_t list, void* p_userData, List_Condition_fn conditionFn)
{
    CHECK_PARAM(list != NULL, 0);
    CHECK_PARAM(conditionFn != NULL, 0);

	List_st* 	 p_list = CONVERT_2_LIST(list);
	void *	 	 p_head = NULL;
	void *		 p_data = NULL;
	CdataCount_t count  = 0;

	List_Lock(list);
	for (p_head = p_list->p_head; p_head != NULL; p_head = List_GetNextNodeNL(list, p_head))
	{
		p_data = List_GetNodeDataNL(list, p_head);
		if (conditionFn(p_data, p_userData))
		{
			count++;
		}
	}
	List_UnLock(list);

	return count;
}

void* List_GetData(List_t list, void* p_keyword)
{
	CHECK_PARAM(list != NULL, NULL);
	CHECK_PARAM(p_keyword != NULL, NULL);

	List_st* p_list = CONVERT_2_LIST(list);
	void *	 	 p_head = NULL;
	void *		 p_data = NULL;

	if (p_list->equal2KeywordFn == NULL)
	{
		LOG_E("equal2KeywordFn is NULL, pls set a valid equal2KeywordFn first.\n");
		return NULL;
	}

	List_Lock(list);
	for (p_head = p_list->p_head; p_head != NULL; p_head = List_GetNextNodeNL(list, p_head))
	{
		p_data = List_GetNodeDataNL(list, p_head);
		if (p_list->equal2KeywordFn(p_data, p_keyword))
		{
			break;
		}
	}
	List_UnLock(list);

	return p_data;
}
void* List_GetDataByCond(List_t list, void* p_userData, List_Condition_fn conditionFn)
{
	CHECK_PARAM(list != NULL, NULL);
	CHECK_PARAM(conditionFn != NULL, NULL);

	List_st* p_list = CONVERT_2_LIST(list);
	void *	 	 p_head = NULL;
	void *		 p_data = NULL;

	List_Lock(list);
	for (p_head = p_list->p_head; p_head != NULL; p_head = List_GetNextNodeNL(list, p_head))
	{
		p_data = List_GetNodeDataNL(list, p_head);
		if (conditionFn(p_data, p_userData))
		{
			break;
		}
	}
	List_UnLock(list);

	return p_data;
}

void* List_GetDataAtPos(List_t list, CdataIndex_t posIndex)
{
	CHECK_PARAM(list != NULL, NULL);

	List_st*     p_list = CONVERT_2_LIST(list);
	void *	 	 p_head = NULL;
	void *		 p_data = NULL;
	CdataIndex_t pos    = 0;

	List_Lock(list);
	for (p_head = p_list->p_head, pos = 0; p_head != NULL; p_head = List_GetNextNodeNL(list, p_head), pos++)
	{
		if (pos == posIndex)
		{
			p_data = List_GetNodeDataNL(list, p_head);
			break;
		}
	}
	List_UnLock(list);

	return p_data;
}

void* List_DetachData(List_t list, void* p_keyword)
{
	CHECK_PARAM(list != NULL, NULL);
	CHECK_PARAM(p_keyword != NULL, NULL);

	int        ret    = ERR_OK;
	ListNode_t node   = NULL;
	void*      p_data = NULL;

	node = List_DetachNodeByKey(list, p_keyword);
	if (node == NULL)
	{
		LOG_E("Node is NULL.\n");
		return NULL;
	}
	p_data = List_DetachNodeData(list, node);

	ret = List_DestroyNode(list, node);
	if (ret != ERR_OK)
	{
		LOG_E("Fail to destroy node.\n");
	}

	return p_data;
}
void* List_DetachDataByCond(List_t list, void* p_userData, List_Condition_fn conditionFn)
{
	CHECK_PARAM(list != NULL, NULL);
	CHECK_PARAM(conditionFn != NULL, NULL);

	int        ret    = ERR_OK;
	ListNode_t node   = NULL;
	void*      p_data = NULL;

	node = List_DetachNodeByCond(list, p_userData, conditionFn);
	if (node == NULL)
	{
		LOG_E("Node is NULL.\n");
		return NULL;
	}
	p_data = List_DetachNodeData(list, node);

	ret = List_DestroyNode(list, node);
	if (ret != ERR_OK)
	{
		LOG_E("Fail to destroy node.\n");
	}

	return p_data;
}

void* List_DetachHeadData(List_t list)
{
    CHECK_PARAM(list != NULL, NULL);

	int   ret    = ERR_OK;
	void* p_head = NULL;
	void* p_data = NULL;

	p_head = List_DetachHead(list);
	if (p_head == NULL)
	{
		LOG_E("Fail to detach head node.\n");
		return NULL;
	}
	p_data = List_DetachNodeData(list, p_head);

	ret = List_DestroyNode(list, p_head);
	if (ret != ERR_OK)
	{
		LOG_E("Fail to destroy head node.\n");
	}

	return p_data;
}

void* List_DetachTailData(List_t list)
{
    CHECK_PARAM(list != NULL, NULL);

	int    ret    = ERR_OK;
	void*  p_tail = NULL;
	void*  p_data = NULL;

	p_tail = List_DetachTail(list);
	if (p_tail == NULL)
	{
		LOG_E("Fail to detach tail node.\n");
		return NULL;
	}
	p_data = List_DetachNodeData(list, p_tail);

	ret = List_DestroyNode(list, p_tail);
	if (ret != ERR_OK)
	{
		LOG_E("Fail to destroy tail node.\n");
	}

	return p_data;
}

void* List_DetachDataAtPos(List_t list, CdataIndex_t posIndex)
{
	CHECK_PARAM(list != NULL, NULL);

	int ret         = ERR_OK;
	ListNode_t node = NULL;
	void* p_data    = NULL;

	node = List_DetachNodeAtPos(list, posIndex);
	if (node == NULL)
	{
		LOG_E("Fail to detach node at pos:%llu.\n", posIndex);
		return NULL;
	}
	p_data = List_DetachNodeData(list, node);

	ret = List_DestroyNode(list, node);
	if (ret != ERR_OK)
	{
		LOG_E("Fail to destroy node.\n");
	}

	return p_data;
}
//=========================================================
//                   Node related functions
//=========================================================
int List_CreateNode(List_t list, void* p_data, ListNode_t* p_node)
{
	CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
	CHECK_PARAM(p_data != NULL, ERR_BAD_PARAM);
	CHECK_PARAM(p_node != NULL, ERR_BAD_PARAM);

	List_st* p_list = CONVERT_2_LIST(list);

	if (p_list->type == LIST_TYPE_DOUBLE_LINK)
	{
		return DBList_CreateNode(list, p_data, p_node);
	}
	else if (p_list->type == LIST_TYPE_SINGLE_LINK)
	{
		return SGList_CreateNode(list, p_data, p_node);
	}
	else
	{
		LOG_E("Invalid list type:%d.\n", p_list->type);
	}

	return ERR_FAIL;
}

int List_DestroyNode(List_t list, ListNode_t node)
{
	CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
	CHECK_PARAM(node != NULL, ERR_BAD_PARAM);

	List_st* p_list = CONVERT_2_LIST(list);
	void *   p_data = NULL;

	if (p_list->type == LIST_TYPE_DOUBLE_LINK)
	{
		DBListNode_st *p_node = (DBListNode_st*)node;
		p_data = p_node->p_data;
	}
	else if (p_list->type == LIST_TYPE_SINGLE_LINK)
	{
		SGListNode_st *p_node = (SGListNode_st*)node;
		p_data = p_node->p_data;
	}
	else
	{
		LOG_E("Invalid list type:%d.\n", p_list->type);
		return ERR_BAD_PARAM;
	}

	OS_Free(node);
	if (p_data == NULL)
	{
		return ERR_OK;
	}

	if (p_list->freeFn != NULL)
	{
		p_list->freeFn(p_data);
	}
	else
	{
		OS_Free(p_data);
	}

	return ERR_OK;
}

int List_InsertNode(List_t list, ListNode_t node)
{
    CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(node != NULL, ERR_BAD_PARAM);

	int ret = ERR_OK;
	List_st* p_list = CONVERT_2_LIST(list);

	List_Lock(list);
	if (p_list->type == LIST_TYPE_DOUBLE_LINK)
	{
		ret = DBList_InsertNode(list, node);
	}
	else if (p_list->type == LIST_TYPE_SINGLE_LINK)
	{
		ret = SGList_InsertNode(list, node);
	}
	else
	{
		LOG_E("Invalid list type:%d.\n", p_list->type);
		ret = ERR_BAD_PARAM;
	}
	List_UnLock(list);

	return ret;
}

int List_InsertNode2Head(List_t list, ListNode_t node)
{
	CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
	CHECK_PARAM(node != NULL, ERR_BAD_PARAM);

	int ret = ERR_OK;
	List_st* p_list = CONVERT_2_LIST(list);

	List_Lock(list);
	if (p_list->type == LIST_TYPE_DOUBLE_LINK)
	{
		ret = DBList_InsertNode2Head(list, node);
	}
	else if (p_list->type == LIST_TYPE_SINGLE_LINK)
	{
		ret = SGList_InsertNode2Head(list, node);
	}
	else
	{
		LOG_E("Invalid list type:%d.\n", p_list->type);
		ret = ERR_BAD_PARAM;
	}
	List_UnLock(list);

	return ret;
}

int List_InsertNodeAsc(List_t list, ListNode_t node)
{
	CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
	CHECK_PARAM(node != NULL, ERR_BAD_PARAM);

	int ret = ERR_OK;
	List_st* p_list = CONVERT_2_LIST(list);

	List_Lock(list);
	if (p_list->type == LIST_TYPE_DOUBLE_LINK)
	{
		ret = DBList_InsertNodeAsc(list, node);
	}
	else if (p_list->type == LIST_TYPE_SINGLE_LINK)
	{
		ret = SGList_InsertNodeAsc(list, node);
	}
	else
	{
		LOG_E("Invalid list type:%d.\n", p_list->type);
		ret = ERR_BAD_PARAM;
	}
	List_UnLock(list);

	return ret;
}

int List_InsertNodeDes(List_t list, ListNode_t node)
{
	CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
	CHECK_PARAM(node != NULL, ERR_BAD_PARAM);

	int ret = ERR_OK;
	List_st* p_list = CONVERT_2_LIST(list);

	List_Lock(list);
	if (p_list->type == LIST_TYPE_DOUBLE_LINK)
	{
		ret = DBList_InsertNodeDes(list, node);
	}
	else if (p_list->type == LIST_TYPE_SINGLE_LINK)
	{
		ret = SGList_InsertNodeDes(list, node);
	}
	else
	{
		LOG_E("Invalid list type:%d.\n", p_list->type);
		ret = ERR_BAD_PARAM;
	}
	List_UnLock(list);

	return ret;
}

int List_InsertNodeUni(List_t list, ListNode_t node)
{
	CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
	CHECK_PARAM(node != NULL, ERR_BAD_PARAM);

	int ret          = ERR_OK;
	List_st* p_list  = CONVERT_2_LIST(list);

	if (p_list->nodeEqualFn == NULL)
	{
		LOG_E("nodeEqualFn is NULL, pls set a valid function first.\n");
		return ERR_BAD_PARAM;
	}

	if (HasDuplicateNode(list, node))
	{
		return ERR_DATA_EXISTS;
	}

	ret = List_InsertNode(list, node);
	if (ret != ERR_OK)
	{
		LOG_E("Fail to insert node.\n");
	}

	return ret;
}

int List_InsertNode2HeadUni(List_t list, ListNode_t node)
{
	CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
	CHECK_PARAM(node != NULL, ERR_BAD_PARAM);

	int ret          = ERR_OK;
	List_st* p_list  = CONVERT_2_LIST(list);

	if (p_list->nodeEqualFn == NULL)
	{
		LOG_E("nodeEqualFn is NULL, pls set a valid function first.\n");
		return ERR_BAD_PARAM;
	}

	if (HasDuplicateNode(list, node))
	{
		return ERR_DATA_EXISTS;
	}

	ret = List_InsertNode2Head(list, node);
	if (ret != ERR_OK)
	{
		LOG_E("Fail to insert node to head.\n");
	}

	return ret;
}

int List_InsertNodeBefore(List_t list, ListNode_t listNode, ListNode_t newNode)
{
    CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(listNode != NULL, ERR_BAD_PARAM);
	CHECK_PARAM(newNode != NULL, ERR_BAD_PARAM);

	int ret = ERR_OK;

	List_Lock(list);
	ret = List_InsertNodeBeforeNL(list, listNode, newNode);
	List_UnLock(list);

	return ret;
}
int List_InsertNodeBeforeNL(List_t list, ListNode_t listNode, ListNode_t newNode)
{
    CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(listNode != NULL, ERR_BAD_PARAM);
	CHECK_PARAM(newNode != NULL, ERR_BAD_PARAM);

	List_st* p_list = CONVERT_2_LIST(list);

	if (p_list->type == LIST_TYPE_DOUBLE_LINK)
	{
		return DBList_InsertNodeBefore(list, listNode, newNode);
	}
	else if (p_list->type == LIST_TYPE_SINGLE_LINK)
	{
		return SGList_InsertNodeBefore(list, listNode, newNode);
	}
	else
	{
		LOG_E("Invalid list type:%d.\n", p_list->type);
	}

	return ERR_BAD_PARAM;
}

int List_InsertNodeAfter(List_t list, ListNode_t listNode, ListNode_t newNode)
{
    CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(listNode != NULL, ERR_BAD_PARAM);
	CHECK_PARAM(newNode != NULL, ERR_BAD_PARAM);

	int ret = ERR_OK;

	List_Lock(list);
	ret = List_InsertNodeAfterNL(list, listNode, newNode);
	List_UnLock(list);

	return ret;
}
int List_InsertNodeAfterNL(List_t list, ListNode_t listNode, ListNode_t newNode)
{
    CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(listNode != NULL, ERR_BAD_PARAM);
	CHECK_PARAM(newNode != NULL, ERR_BAD_PARAM);

	List_st* p_list = CONVERT_2_LIST(list);

	if (p_list->type == LIST_TYPE_DOUBLE_LINK)
	{
		return DBList_InsertNodeAfter(list, listNode, newNode);
	}
	else if (p_list->type == LIST_TYPE_SINGLE_LINK)
	{
		return SGList_InsertNodeAfter(list, listNode, newNode);
	}
	else
	{
		LOG_E("Invalid list type:%d.\n", p_list->type);
	}

	return ERR_BAD_PARAM;
}

int List_InsertNodeAtPos(List_t list, ListNode_t node, CdataIndex_t posIndex)
{
    CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(node != NULL, ERR_BAD_PARAM);

	int ret         = ERR_OK;
	List_st* p_list = CONVERT_2_LIST(list);
	void*    p_head = NULL;
	CdataIndex_t i  = 0;

	if (posIndex <= 0)
	{
		ret = List_InsertNode2Head(list, node);
		if (ret != ERR_OK)
		{
			LOG_E("Fail to insert node to head.\n");
		}

		return ret;
	}

	if (posIndex >= p_list->nodeCount)
	{
		ret = List_InsertNode(list, node);
		if (ret != ERR_OK)
		{
			LOG_E("Fail to insert node to tail.\n");
		}

		return ret;
	}

	List_Lock(list);
	for (p_head = p_list->p_head, i = 0; p_head != NULL; p_head = List_GetNextNodeNL(list, p_head), i++)
	{
		if (i == (posIndex - 1))
		{
			ret = List_InsertNodeAfterNL(list, p_head, node);
			break;
		}
	}
	List_UnLock(list);

	return ret;
}

ListNode_t List_GetNodeAtPos(List_t list, CdataIndex_t posIndex)
{
    CHECK_PARAM(list != NULL, NULL);

	List_st* p_list = CONVERT_2_LIST(list);
	ListNode_t node = NULL;
	CdataIndex_t pos = 0;

	List_Lock(list);
	for (node = p_list->p_head, pos = 0; node != NULL; node = List_GetNextNodeNL(list, node), pos++)
	{
		if (pos == posIndex)
		{
			break;
		}
	}
	List_UnLock(list);

	return node;
}

ListNode_t List_GetHead(List_t list)
{
	CHECK_PARAM(list != NULL, NULL);

	ListNode_t node = NULL;

	List_Lock(list);
	node = List_GetHeadNL(list);
	List_UnLock(list);

	return node;
}
ListNode_t List_GetHeadNL(List_t list)
{
    CHECK_PARAM(list != NULL, NULL);

	List_st* p_list = CONVERT_2_LIST(list);
	return p_list->p_head;
}

ListNode_t List_GetTail(List_t list)
{
	CHECK_PARAM(list != NULL, NULL);

	ListNode_t node = NULL;

	List_Lock(list);
	node = List_GetTailNL(list);
	List_UnLock(list);

	return node;
}
ListNode_t List_GetTailNL(List_t list)
{
    CHECK_PARAM(list != NULL, NULL);

	List_st* p_list = CONVERT_2_LIST(list);
	return p_list->p_tail;
}

ListNode_t List_GetPreNode(List_t list,ListNode_t node)
{
	CHECK_PARAM(list != NULL, NULL);
	CHECK_PARAM(node != NULL, NULL);

	ListNode_t preNode = NULL;

	List_Lock(list);
	preNode = List_GetPreNodeNL(list, node);
	List_UnLock(list);

	return preNode;
}

ListNode_t List_GetPreNodeNL(List_t list,ListNode_t node)
{
	CHECK_PARAM(list != NULL, NULL);
    CHECK_PARAM(node != NULL, NULL);

	List_st* p_list = CONVERT_2_LIST(list);
	if (p_list->type == LIST_TYPE_SINGLE_LINK)
	{
		LOG_E("Hasn't pre node on a single list node.\n");
		return NULL;
	}

	DBListNode_st *p_node = (DBListNode_st*)node;
	return p_node->p_pre;
}

ListNode_t List_GetNextNode(List_t list, ListNode_t node)
{
	CHECK_PARAM(list != NULL, NULL);
	CHECK_PARAM(node != NULL, NULL);

	ListNode_t nextNode = NULL;

	List_Lock(list);
	nextNode = List_GetNextNodeNL(list, node);
	List_UnLock(list);

	return nextNode;
}
ListNode_t List_GetNextNodeNL(List_t list, ListNode_t node)
{
	CHECK_PARAM(list != NULL, NULL);
    CHECK_PARAM(node != NULL, NULL);

	ListNode_t nextNode = NULL;
	List_st* p_list = CONVERT_2_LIST(list);

	if (p_list->type == LIST_TYPE_DOUBLE_LINK)
	{
		DBListNode_st *p_listNode = (DBListNode_st*)node;
		nextNode = p_listNode->p_next;
	}
	else if (p_list->type == LIST_TYPE_SINGLE_LINK)
	{
		SGListNode_st *p_listNode = (SGListNode_st*)node;
		nextNode = p_listNode->p_next;
	}
	else
	{
		LOG_E("Invalid list type:%d.\n", p_list->type);
	}

	return nextNode;
}

void*  List_GetNodeData(List_t list, ListNode_t node)
{
	CHECK_PARAM(list != NULL, NULL);
    CHECK_PARAM(node != NULL, NULL);

	void *p_data = NULL;

	List_Lock(list);
	p_data = List_GetNodeDataNL(list, node);
	List_UnLock(list);

	return p_data;
}
void*  List_GetNodeDataNL(List_t list, ListNode_t node)
{
	CHECK_PARAM(list != NULL, NULL);
    CHECK_PARAM(node != NULL, NULL);

	List_st* p_list = CONVERT_2_LIST(list);
	void *p_data = NULL;

	if (p_list->type == LIST_TYPE_DOUBLE_LINK)
	{
		DBListNode_st *p_listNode = (DBListNode_st*)node;
		p_data = p_listNode->p_data;
	}
	else if (p_list->type == LIST_TYPE_SINGLE_LINK)
	{
		SGListNode_st *p_listNode = (SGListNode_st*)node;
		p_data = p_listNode->p_data;
	}
	else
	{
		LOG_E("Invalid list type:%d.\n", p_list->type);
	}

	return p_data;
}

void*  List_DetachNodeData(List_t list, ListNode_t node)
{
	CHECK_PARAM(list != NULL, NULL);
    CHECK_PARAM(node != NULL, NULL);

	void *p_data = NULL;

	List_Lock(list);
	p_data = List_DetachNodeDataNL(list, node);
	List_UnLock(list);

	return p_data;
}
void*  List_DetachNodeDataNL(List_t list, ListNode_t node)
{
    CHECK_PARAM(list != NULL, NULL);
    CHECK_PARAM(node != NULL, NULL);

	void*    p_data = NULL;
	List_st* p_list = CONVERT_2_LIST(list);

	if (p_list->type == LIST_TYPE_DOUBLE_LINK)
	{
		DBListNode_st *p_listNode = (DBListNode_st*)node;
		p_data = p_listNode->p_data;
		p_listNode->p_data = NULL;
	}
	else if (p_list->type == LIST_TYPE_SINGLE_LINK)
	{
		SGListNode_st *p_listNode = (SGListNode_st*)node;
		p_data = p_listNode->p_data;
		p_listNode->p_data = NULL;
	}
	else
	{
		LOG_E("Invalid list type:%d.\n", p_list->type);
	}

	return p_data;
}

ListNode_t List_GetFirstMatchNode(List_t list, void* p_userData)
{
    CHECK_PARAM(list != NULL, NULL);
    CHECK_PARAM(p_userData != NULL, NULL);

	void* 	p_node = NULL;
	void *	p_data = NULL;

	List_st* p_list = CONVERT_2_LIST(list);

	if (p_list->equal2KeywordFn == NULL)
	{
		LOG_E("equal2KeywordFn is NULL, pls set a valid equal2KeywordFn first.\n");
		return NULL;
	}

	List_Lock(list);
	for (p_node = List_GetHeadNL(list); p_node != NULL; p_node = List_GetNextNodeNL(list, p_node))
	{
		p_data = List_GetNodeDataNL(list, p_node);
		if (p_list->equal2KeywordFn(p_data, p_userData))
		{
			break;
		}
	}
	List_UnLock(list);

	return p_node;
}

ListNode_t List_GetNextMatchNode(List_t list, ListNode_t startNode, void* p_userData)
{
    CHECK_PARAM(list != NULL, NULL);
    CHECK_PARAM(p_userData != NULL, NULL);

	void* 	p_node = NULL;
	void *	p_data = NULL;
	List_st* p_list = CONVERT_2_LIST(list);


	if (p_list->equal2KeywordFn == NULL)
	{
		LOG_E("equal2KeywordFn is NULL, pls set a valid equal2KeywordFn first.\n");
		return NULL;
	}

	List_Lock(list);
	for (p_node = startNode; p_node != NULL; p_node = List_GetNextNodeNL(list, p_node))
	{
		p_data = List_GetNodeDataNL(list, p_node);
		if (p_list->equal2KeywordFn(p_data, p_userData))
		{
			break;
		}
	}
	List_UnLock(list);

	return p_node;
}

ListNode_t List_GetLastMatchNode(List_t list, void* p_userData)
{
    CHECK_PARAM(list != NULL, NULL);
    CHECK_PARAM(p_userData != NULL, NULL);

	void* 	p_node = NULL;
	void *	p_data = NULL;
	List_st* p_list = CONVERT_2_LIST(list);

	if (p_list->type == LIST_TYPE_SINGLE_LINK)
	{
		LOG_E("Cannot get last match node on a single link list.\n");
		return NULL;
	}

	if (p_list->equal2KeywordFn == NULL)
	{
		LOG_E("equal2KeywordFn is NULL, pls set a valid equal2KeywordFn first.\n");
		return NULL;
	}

	List_Lock(list);
	for (p_node = List_GetTailNL(list); p_node != NULL; p_node = List_GetPreNodeNL(list, p_node))
	{
		p_data = List_GetNodeDataNL(list, p_node);
		if (p_list->equal2KeywordFn(p_data, p_userData))
		{
			break;
		}
	}
	List_UnLock(list);

	return p_node;
}

ListNode_t List_GetPreMatchNode(List_t list, ListNode_t startNode, void* p_userData)
{
    CHECK_PARAM(list != NULL, NULL);
    CHECK_PARAM(p_userData != NULL, NULL);

	void* 	p_node = NULL;
	void *	p_data = NULL;
	List_st* p_list = CONVERT_2_LIST(list);

	if (p_list->type == LIST_TYPE_SINGLE_LINK)
	{
		LOG_E("Cannot get last match node on a single link list.\n");
		return NULL;
	}

	if (p_list->equal2KeywordFn == NULL)
	{
		LOG_E("equal2KeywordFn is NULL, pls set a valid equal2KeywordFn first.\n");
		return NULL;
	}

	List_Lock(list);
	for (p_node = startNode; p_node != NULL; p_node = List_GetPreNodeNL(list, p_node))
	{
		p_data = List_GetNodeDataNL(list, p_node);
		if (p_list->equal2KeywordFn(p_data, p_userData))
		{
			break;
		}
	}
	List_UnLock(list);

	return p_node;
}

ListNode_t List_GetFirstMatchNodeByCond(List_t list, void* p_userData, List_Condition_fn conditionFn)
{
    CHECK_PARAM(list != NULL, NULL);
    CHECK_PARAM(conditionFn != NULL, NULL);

	void* 	p_node = NULL;
	void *	p_data = NULL;

	List_Lock(list);
	for (p_node = List_GetHeadNL(list); p_node != NULL; p_node = List_GetNextNodeNL(list, p_node))
	{
		p_data = List_GetNodeDataNL(list, p_node);
		if (conditionFn(p_data, p_userData))
		{
			break;
		}
	}
	List_UnLock(list);

	return p_node;
}

ListNode_t List_GetNextMatchNodeByCond(List_t list, ListNode_t startNode, void* p_userData, List_Condition_fn conditionFn)
{
    CHECK_PARAM(list != NULL, NULL);
    CHECK_PARAM(conditionFn != NULL, NULL);

	void* 	p_node = NULL;
	void *	p_data = NULL;

	List_Lock(list);
	for (p_node = startNode; p_node != NULL; p_node = List_GetNextNodeNL(list, p_node))
	{
		p_data = List_GetNodeDataNL(list, p_node);
		if (conditionFn(p_data, p_userData))
		{
			break;
		}
	}
	List_UnLock(list);

	return p_node;
}

ListNode_t List_GetLastMatchNodeByCond(List_t list, void* p_userData, List_Condition_fn conditionFn)
{
    CHECK_PARAM(list != NULL, NULL);
    CHECK_PARAM(conditionFn != NULL, NULL);

	void* 	p_node = NULL;
	void *	p_data = NULL;
	List_st* p_list = CONVERT_2_LIST(list);

	if (p_list->type == LIST_TYPE_SINGLE_LINK)
	{
		LOG_E("Cannot get last match node on a single link list.\n");
		return NULL;
	}

	List_Lock(list);
	for (p_node = List_GetTailNL(list); p_node != NULL; p_node = List_GetPreNodeNL(list, p_node))
	{
		p_data = List_GetNodeDataNL(list, p_node);
		if (conditionFn(p_data, p_userData))
		{
			break;
		}
	}
	List_UnLock(list);

	return p_node;
}

ListNode_t List_GetPreMatchNodeByCond(List_t list, ListNode_t startNode, void* p_userData, List_Condition_fn conditionFn)
{
    CHECK_PARAM(list != NULL, NULL);
    CHECK_PARAM(conditionFn != NULL, NULL);

	void* 	p_node = NULL;
	void *	p_data = NULL;
	List_st* p_list = CONVERT_2_LIST(list);

	if (p_list->type == LIST_TYPE_SINGLE_LINK)
	{
		LOG_E("Cannot get last match node on a single link list.\n");
		return NULL;
	}

	List_Lock(list);
	for (p_node = startNode; p_node != NULL; p_node = List_GetPreNodeNL(list, p_node))
	{
		p_data = List_GetNodeDataNL(list, p_node);
		if (conditionFn(p_data, p_userData))
		{
			break;
		}
	}
	List_UnLock(list);

	return p_node;
}

int List_Swap(List_t list, ListNode_t firstNode, ListNode_t secondNode)
{
    CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
	CHECK_PARAM(firstNode != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(secondNode != NULL, ERR_BAD_PARAM);

	List_st* p_list = CONVERT_2_LIST(list);
	int      ret    = ERR_OK;

	List_Lock(list);
	if (p_list->type == LIST_TYPE_SINGLE_LINK)
	{
        SGListNode_st* p_firstNode = (SGListNode_st*)firstNode;
        SGListNode_st* p_secondNode = (SGListNode_st*)secondNode;
        void *p_data = NULL;

        p_data = p_secondNode->p_data;
        p_secondNode->p_data = p_firstNode->p_data;
        p_firstNode->p_data = p_data;
	}
    else if (p_list->type == LIST_TYPE_DOUBLE_LINK)
    {
        DBListNode_st* p_firstNode = (DBListNode_st*)firstNode;
        DBListNode_st* p_secondNode = (DBListNode_st*)secondNode;
        void *p_data = NULL;

        p_data = p_secondNode->p_data;
        p_secondNode->p_data = p_firstNode->p_data;
        p_firstNode->p_data = p_data;
    }
    else
    {
        LOG_E("Wrong list type:%d.\n", p_list->type);
        ret = ERR_BAD_PARAM;
    }
	List_UnLock(list);

	return ret;
}

int List_DetachNode(List_t list, ListNode_t node)
{
    CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
	CHECK_PARAM(node != NULL, ERR_BAD_PARAM);

	int ret = ERR_OK;

	List_Lock(list);
	ret = List_DetachNodeNL(list, node);
	List_UnLock(list);

	return ret;
}
int List_DetachNodeNL(List_t list, ListNode_t node)
{
    CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(node != NULL, ERR_BAD_PARAM);

	List_st* p_list = CONVERT_2_LIST(list);

	if (p_list->type == LIST_TYPE_DOUBLE_LINK)
	{
		return DBList_DetachNode(list, node);
	}
	else if (p_list->type == LIST_TYPE_SINGLE_LINK)
	{
		return SGList_DetachNode(list, node);
	}
	else
	{
		LOG_E("Invalid list type:%d.\n", p_list->type);
	}

	return ERR_BAD_PARAM;
}

ListNode_t List_DetachHead(List_t list)
{
    CHECK_PARAM(list != NULL, NULL);

	int ret = ERR_OK;
	List_st* p_list = CONVERT_2_LIST(list);
	ListNode_t node = p_list->p_head;

	if (p_list->nodeCount == 0)
	{
		return NULL;
	}

	ret = List_DetachNode(list, node);
	if (ret != ERR_OK)
	{
		LOG_E("Fail to detach node.\n");
		return NULL;
	}

	return node;
}

ListNode_t List_DetachTail(List_t list)
{
    CHECK_PARAM(list != NULL, NULL);

	int ret = ERR_OK;
	List_st* p_list = CONVERT_2_LIST(list);
	ListNode_t node = p_list->p_tail;

	if (p_list->nodeCount == 0)
	{
		return NULL;
	}

	ret = List_DetachNode(list, node);
	if (ret != ERR_OK)
	{
		LOG_E("Fail to detach node.\n");
		return NULL;
	}

	return node;
}

ListNode_t List_DetachNodeAtPos(List_t list, CdataIndex_t posIndex)
{
    CHECK_PARAM(list != NULL, NULL);

	List_st* p_list = CONVERT_2_LIST(list);
	void*    p_node = NULL;
	CdataIndex_t i = 0;

	List_Lock(list);
	for (p_node = p_list->p_head, i = 0; p_node != NULL; p_node = List_GetNextNodeNL(list, p_node), i++)
	{
		if (i == posIndex)
		{
			List_DetachNodeNL(list, p_node);
			break;
		}
	}
	List_UnLock(list);

	return p_node;
}

ListNode_t List_DetachNodeByKey(List_t list, void* p_keyword)
{
    CHECK_PARAM(list != NULL, NULL);
	CHECK_PARAM(p_keyword != NULL, NULL);

	List_st* p_list = CONVERT_2_LIST(list);
	void*    p_node = NULL;
	void*	 p_data = NULL;

	if (p_list->equal2KeywordFn == NULL)
	{
		LOG_E("equal2KeywordFn is NULL, pls set a valid equal2KeywordFn first.\n");
		return NULL;
	}

	List_Lock(list);
	for (p_node = p_list->p_head; p_node != NULL; p_node = List_GetNextNodeNL(list, p_node))
	{
		p_data = List_GetNodeDataNL(list, p_node);
		if (p_data == NULL)
		{
			continue;
		}

		if (p_list->equal2KeywordFn(p_data, p_keyword))
		{
			List_DetachNodeNL(list, p_node);
			break;
		}
	}
	List_UnLock(list);

	return p_node;
}
ListNode_t List_DetachNodeByCond(List_t list, void* p_userData, List_Condition_fn conditionFn)
{
    CHECK_PARAM(list != NULL, NULL);
	CHECK_PARAM(conditionFn != NULL, NULL);

	List_st* p_list = CONVERT_2_LIST(list);
	void*    p_node = NULL;
	void*	 p_data = NULL;

	List_Lock(list);
	for (p_node = p_list->p_head; p_node != NULL; p_node = List_GetNextNodeNL(list, p_node))
	{
		p_data = List_GetNodeDataNL(list, p_node);
		if (p_data == NULL)
		{
			continue;
		}

		if (conditionFn(p_data, p_userData))
		{
			List_DetachNodeNL(list, p_node);
			break;
		}
	}
	List_UnLock(list);

	return p_node;
}

int List_RmNode(List_t list, ListNode_t node)
{
    CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
	CHECK_PARAM(node != NULL, ERR_BAD_PARAM);

	int ret = ERR_OK;

	List_Lock(list);
	ret = List_RmNodeNL(list, node);
	List_UnLock(list);

	return ret;
}
int List_RmNodeNL(List_t list, ListNode_t node)
{
    CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(node != NULL, ERR_BAD_PARAM);

	int ret = ERR_OK;

	ret = List_DetachNodeNL(list, node);
	if (ret != ERR_OK)
	{
		LOG_E("Fail to detach node.\n");
		return ERR_FAIL;
	}

	ret = List_DestroyNode(list, node);
	if (ret != ERR_OK)
	{
		LOG_E("Fail to destroy node.\n");
		return ERR_FAIL;
	}

	return ERR_OK;
}

int List_RmHead(List_t list)
{
    CHECK_PARAM(list != NULL, ERR_BAD_PARAM);

	int ret = ERR_OK;
	ListNode_t node = NULL;


	node = List_DetachHead(list);
	if (node == NULL)
	{
		LOG_E("Fail to detach head.\n");
		return ERR_FAIL;
	}

	ret = List_DestroyNode(list, node);
	if (ret != ERR_OK)
	{
		LOG_E("Fail to destroy node.\n");
		ret = ERR_FAIL;
	}

	return ret;
}

int List_RmTail(List_t list)
{
    CHECK_PARAM(list != NULL, ERR_BAD_PARAM);

	int ret = ERR_OK;
	ListNode_t node = NULL;

	node = List_DetachTail(list);
	if (node == NULL)
	{
		LOG_E("Fail to detach tail.\n");
		return ERR_FAIL;
	}

	ret = List_DestroyNode(list, node);
	if (ret != ERR_OK)
	{
		LOG_E("Fail to destroy node.\n");
		ret = ERR_FAIL;
	}

	return ret;
}

int List_RmNodeAtPos(List_t list, CdataIndex_t posIndex)
{
    CHECK_PARAM(list != NULL, ERR_BAD_PARAM);

	int ret = ERR_OK;
	ListNode_t node = NULL;

	node = List_DetachNodeAtPos(list, posIndex);
	if (node == NULL)
	{
		LOG_E("Fail to detach node.\n");
		return ERR_FAIL;
	}

	ret = List_DestroyNode(list, node);
	if (ret != ERR_OK)
	{
		LOG_E("Fail to destroy node.\n");
		ret = ERR_FAIL;
	}

	return ret;
}

int List_RmFirstMatchNode(List_t list, void* p_userData)
{
	CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
	CHECK_PARAM(p_userData != NULL, ERR_BAD_PARAM);

	List_st* p_list = CONVERT_2_LIST(list);
	void*    p_node = NULL;
	void*    p_data = NULL;
	CdataBool found = CDATA_FALSE;

	if (p_list->equal2KeywordFn == NULL)
	{
		LOG_E("equal2KeywordFn is NULL, pls set a valid equal2KeywordFn first.\n");
		return ERR_BAD_PARAM;
	}

	List_Lock(list);
	for (p_node = p_list->p_head; p_node != NULL; p_node = List_GetNextNodeNL(list, p_node))
	{
		p_data = List_GetNodeDataNL(list, p_node);
		if (p_list->equal2KeywordFn(p_data, p_userData))
		{
			found = CDATA_TRUE;
			break;
		}
	}
	List_UnLock(list);

	if (found)
	{
		if (List_DetachNode(list, p_node) != ERR_OK)
		{
			LOG_E("Fail to detach node.\n");
			return ERR_FAIL;
		}

		if (List_DestroyNode(list, p_node) != ERR_OK)
		{
			LOG_E("Fail to destroy node.\n");
			return ERR_FAIL;
		}
	}

	return ERR_OK;
}
int List_RmFirstMatchNodeByCond(List_t list, void* p_userData, List_Condition_fn conditionFn)
{
	CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
	CHECK_PARAM(conditionFn != NULL, ERR_BAD_PARAM);

	List_st* p_list = CONVERT_2_LIST(list);
	void*    p_node = NULL;
	void*    p_data = NULL;
	CdataBool found = CDATA_FALSE;

	List_Lock(list);
	for (p_node = p_list->p_head; p_node != NULL; p_node = List_GetNextNodeNL(list, p_node))
	{
		p_data = List_GetNodeDataNL(list, p_node);
		if (conditionFn(p_data, p_userData))
		{
			found = CDATA_TRUE;
			break;
		}
	}
	List_UnLock(list);

	if (found)
	{
		if (List_DetachNode(list, p_node) != ERR_OK)
		{
			LOG_E("Fail to detach node.\n");
			return ERR_FAIL;
		}

		if (List_DestroyNode(list, p_node) != ERR_OK)
		{
			LOG_E("Fail to destroy node.\n");
			return ERR_FAIL;
		}

	}

	return ERR_OK;
}

CdataCount_t List_RmAllMatchNodes(List_t list, void* p_userData)
{
	CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
	CHECK_PARAM(p_userData != NULL, ERR_BAD_PARAM);

	List_st* p_list = CONVERT_2_LIST(list);
	void*    p_node = NULL;
	void*    p_next = NULL;
	void*    p_data = NULL;
	CdataBool found = CDATA_FALSE;
	CdataCount_t count = 0;

	if (p_list->equal2KeywordFn == NULL)
	{
		LOG_E("equal2KeywordFn is NULL, pls set a valid equal2KeywordFn first.\n");
		return ERR_BAD_PARAM;
	}

	List_Lock(list);
	count = 0;
	p_next = p_list->p_head;
	while(1)
	{
		for (p_node = p_next, found = CDATA_FALSE; p_node != NULL; p_node = List_GetNextNodeNL(list, p_node))
		{
			p_data = List_GetNodeDataNL(list, p_node);
			if (p_list->equal2KeywordFn(p_data, p_userData))
			{
				found = CDATA_TRUE;
				count++;
				break;
			}
		}

		if (found)
		{
			p_next = List_GetNextNodeNL(list, p_node);

			if (List_DetachNodeNL(list, p_node) != ERR_OK)
			{
                LOG_E("Fail to detach node.\n");
                continue;
            }
			if (List_DestroyNode(list, p_node) != ERR_OK)
			{
                LOG_E("Fail to destroy node.\n");
            }
		}
		else
		{
			break;
		}
	}
	List_UnLock(list);

	return count;
}
CdataCount_t List_RmAllMatchNodesByCond(List_t list, void* p_userData, List_Condition_fn conditionFn)
{
	CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
	CHECK_PARAM(conditionFn != NULL, ERR_BAD_PARAM);

	List_st* p_list = CONVERT_2_LIST(list);
	void*    p_node = NULL;
	void*    p_next = NULL;
	void*    p_data = NULL;
	CdataBool found = CDATA_FALSE;
	CdataCount_t count = 0;

	List_Lock(list);
	count = 0;
	p_next = p_list->p_head;
	while(1)
	{
		for (p_node = p_next, found = CDATA_FALSE; p_node != NULL; p_node = List_GetNextNodeNL(list, p_node))
		{
			p_data = List_GetNodeDataNL(list, p_node);
			if (conditionFn(p_data, p_userData))
			{
				found = CDATA_TRUE;
				count++;
				break;
			}
		}

		if (found)
		{
			p_next = List_GetNextNodeNL(list, p_node);

			if (List_DetachNodeNL(list, p_node) != ERR_OK)
			{
                LOG_E("Fail to detach node.\n");
                continue;
            }
			if (List_DestroyNode(list, p_node) != ERR_OK)
			{
                LOG_E("Fail to destroy node.\n");
            }

		}
		else
		{
			break;
		}
	}
	List_UnLock(list);

	return count;
}


/*=============================================================================*
 *                    Inner function implemention
 *============================================================================*/
List_t  CreateList(ListName_t name, ListType_e type, List_DataType_e dataType, int dataLength)
{
    OSMutex_t* guard = NULL;
    List_st* p_newList = NULL;

    size_t minNameLen = 0;
    size_t nameLen = 0;
    size_t maxLen = 0;

    guard = CreateGuard();
    if (guard == NULL)
    {
        LOG_E("Fail to create list guard.\n");
        return NULL;
    }

    p_newList = (List_st*) OS_Malloc(sizeof(List_st));
    if (NULL == p_newList)
    {
        LOG_E("Have no enough memory.\n");

        DeleteGuard(guard);
        return NULL;
    }
    memset(p_newList, 0x0, sizeof(List_st));

	p_newList->type    = type;
    p_newList->p_head  = NULL;
    p_newList->p_tail  = NULL;

    p_newList->name[0] = '\0';
    nameLen = strlen(name);
    maxLen = sizeof(ListName_t) - 1;
    minNameLen = nameLen < maxLen ? nameLen : maxLen;
    memcpy(p_newList->name, name, minNameLen);
    p_newList->name[minNameLen] = '\0';

    p_newList->dataType    = dataType;
    p_newList->dataLength  = dataLength;

    p_newList->nodeCount    = 0;
    p_newList->guard  = guard;

	p_newList->freeFn = NULL;
	p_newList->equal2KeywordFn = NULL;
	p_newList->usrLtNodeFn = NULL;
	p_newList->nodeEqualFn = NULL;

    return p_newList;
}

static OSMutex_t  CreateGuard()
{
    OSMutex_t mutex = OS_MutexCreate();
    if(mutex == NULL)
    {
        LOG_E("Fail to create a mutex!\n");
        return NULL;
    }

    return mutex;
}

static void DeleteGuard(OSMutex_t guard)
{
    ASSERT(guard != NULL);

    OS_MutexDestroy(guard);
}

static CdataBool   HasDuplicateNode(List_t list, ListNode_t node)
{
	ASSERT(list != NULL);
	ASSERT(node != NULL);

	void *p_node     = NULL;
	void *p_data     = NULL;
	void *p_userData = NULL;
	CdataBool isDuplicate = CDATA_FALSE;
	List_st* p_list  = CONVERT_2_LIST(list);

	List_Lock(list);
	p_userData = List_GetNodeDataNL(list, node);
	for (p_node = p_list->p_head; p_node != NULL; p_node = List_GetNextNodeNL(list, p_node))
	{
		p_data = List_GetNodeDataNL(list, p_node);
		if (p_list->nodeEqualFn(p_data, p_userData))
		{
			isDuplicate = CDATA_TRUE;
			break;
		}
	}
	List_UnLock(list);

	return isDuplicate;
}

/*=============================================================================*
 *                                End of file
 *============================================================================*/

