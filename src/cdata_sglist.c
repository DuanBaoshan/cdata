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
Date:2019.4.1
*/

#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "cdata_types.h"
#include "list_internal.h"
#include "cdata_sglist.h"

#ifndef _DEBUG_LEVEL_
#define _DEBUG_LEVEL_  2
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
 *                    Static variable declaration
 *============================================================================*/


/*=============================================================================*
 *                    Inner function declaration
 *============================================================================*/
static void InsertFirstNode(List_st *p_list, SGListNode_st *p_node);

/*=============================================================================*
 *                    Outer function implemention
 *============================================================================*/
int SGList_CreateNode(List_t list, void* p_data, ListNode_t* p_node)
{
	CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(p_data != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(p_node != NULL, ERR_BAD_PARAM);

    SGListNode_st* p_newNode = NULL;
    List_st*       p_list    = CONVERT_2_LIST(list);

    p_newNode = (SGListNode_st*) OS_Malloc(sizeof(SGListNode_st));
    if (NULL == p_newNode)
    {
        LOG_E("Not enough memory 1\n");
        return ERR_OUT_MEM;
    }
    memset(p_newNode, 0x0, sizeof(SGListNode_st));

    p_newNode->p_next = NULL;

    if (p_list->dataType == LIST_DATA_TYPE_VALUE_COPY)
    {
        p_newNode->p_data = (void*) OS_Malloc(p_list->dataLength);
        if (p_newNode->p_data == NULL)
        {
            LOG_E("Not enough memory.\n");

            OS_Free(p_newNode);
            return ERR_OUT_MEM;
        }
        else
        {
            memcpy(p_newNode->p_data, p_data, p_list->dataLength);
        }
    }
    else if (p_list->dataType == LIST_DATA_TYPE_VALUE_REFERENCE)
    {
        p_newNode->p_data = p_data;
    }

    *p_node = p_newNode;
    return ERR_OK;
}

int SGList_InsertNode(List_t list, ListNode_t node)
{
	CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(node != NULL, ERR_BAD_PARAM);

    List_st*       p_list = CONVERT_2_LIST(list);
	SGListNode_st* p_node = CONVERT_2_SGLIST_NODE(node);

	if (p_list->nodeCount == 0)
	{
		InsertFirstNode(p_list, p_node);
		return ERR_OK;
	}

	((SGListNode_st*)p_list->p_tail)->p_next = (ListNode_t)p_node;
	p_node->p_next = NULL;
	p_list->p_tail = p_node;

	p_list->nodeCount++;

	return ERR_OK;
}

int SGList_InsertNode2Head(List_t list, ListNode_t node)
{
	CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(node != NULL, ERR_BAD_PARAM);

    List_st*       p_list = CONVERT_2_LIST(list);
	SGListNode_st* p_node = CONVERT_2_SGLIST_NODE(node);

	if (p_list->nodeCount == 0)
	{
		InsertFirstNode(p_list, p_node);
		return ERR_OK;
	}

	p_node->p_next = p_list->p_head;
	p_list->p_head = p_node;
	p_list->nodeCount++;

	return ERR_OK;
}

int SGList_InsertNodeAscently(List_t list, ListNode_t node)
{
	CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(node != NULL, ERR_BAD_PARAM);

    List_st*       p_list = CONVERT_2_LIST(list);
	SGListNode_st* p_node = CONVERT_2_SGLIST_NODE(node);
	SGListNode_st* p_head = NULL;
	void*		   p_userData = NULL;

	SGListNode_st* p_pre = NULL;
	SGListNode_st* p_cur = NULL;

    if (p_list->nodeCount == 0)
    {
        return SGList_InsertNode(list, node);
    }

	if (p_list->usrLtNodeFn == NULL)
	{
		LOG_E("usrLtNodeFn is NULL.\n");
		return ERR_FAIL;
	}

	p_userData = p_node->p_data;
	p_head = CONVERT_2_SGLIST_NODE(p_list->p_head);

	if (p_list->usrLtNodeFn(p_head->p_data, p_userData))
	{
		return SGList_InsertNode2Head(list, node);
	}

	if (p_list->nodeCount == 1)
	{
		return SGList_InsertNode(list, node);
	}

	for (p_pre = p_head, p_cur = (SGListNode_st*)p_head->p_next; p_cur != NULL; p_pre = p_cur, p_cur = (SGListNode_st*)p_cur->p_next)
	{
		if (p_list->usrLtNodeFn(p_cur->p_data, p_userData))
		{
			return SGList_InsertNodeAfter(list, p_pre, node);
		}
	}

	return SGList_InsertNode(list, node);
}

int SGList_InsertNodeDescently(List_t list, ListNode_t node)
{
	CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(node != NULL, ERR_BAD_PARAM);

    List_st*       p_list = CONVERT_2_LIST(list);
	SGListNode_st* p_node = CONVERT_2_SGLIST_NODE(node);
	SGListNode_st* p_head = NULL;
	void*		   p_userData = NULL;

	SGListNode_st* p_pre = NULL;
	SGListNode_st* p_cur = NULL;

    if (p_list->nodeCount == 0)
    {
        return SGList_InsertNode(list, node);
    }

	if (p_list->usrLtNodeFn == NULL)
	{
		LOG_E("usrLtNodeFn is NULL.\n");
		return ERR_FAIL;
	}

	p_userData = p_node->p_data;
	p_head = CONVERT_2_SGLIST_NODE(p_list->p_head);

	if (!p_list->usrLtNodeFn(p_head->p_data, p_userData))
	{
		return SGList_InsertNode2Head(list, node);
	}

	if (p_list->nodeCount == 1)
	{
		return SGList_InsertNode(list, node);
	}

	for (p_pre = p_head, p_cur = (SGListNode_st*)p_head->p_next; p_cur != NULL; p_pre = p_cur, p_cur = (SGListNode_st*)p_cur->p_next)
	{
		if (!p_list->usrLtNodeFn(p_cur->p_data, p_userData))
		{
			return SGList_InsertNodeAfter(list, p_pre, node);
		}
	}

	return SGList_InsertNode(list, node);
}

int SGList_InsertNodeBefore(List_t list, ListNode_t listNode, ListNode_t newNode)
{
	CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(listNode != NULL, ERR_BAD_PARAM);
	CHECK_PARAM(newNode != NULL, ERR_BAD_PARAM);

    List_st*       p_list = CONVERT_2_LIST(list);
	SGListNode_st* p_head = NULL;
	SGListNode_st* p_pre = NULL;
	SGListNode_st* p_cur = NULL;

	if (listNode == p_list->p_head)
	{
		return SGList_InsertNode2Head(list, newNode);
	}

	p_head = CONVERT_2_SGLIST_NODE(p_list->p_head);
	for (p_pre = p_head, p_cur = (SGListNode_st*)p_pre->p_next; p_cur != NULL; p_pre = p_cur, p_cur = (SGListNode_st*)p_cur->p_next)
	{
		if (p_cur == listNode)
		{
			return SGList_InsertNodeAfter(list, p_pre, newNode);
		}
	}

	LOG_E("Not find list node to insert before.\n");

	return ERR_BAD_PARAM;
}

int SGList_InsertNodeAfter(List_t list, ListNode_t listNode, ListNode_t newNode)
{
	CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(listNode != NULL, ERR_BAD_PARAM);
	CHECK_PARAM(newNode != NULL, ERR_BAD_PARAM);

	List_st*       p_list = CONVERT_2_LIST(list);
	SGListNode_st* p_listNode = CONVERT_2_SGLIST_NODE(listNode);
	SGListNode_st* p_newNode = CONVERT_2_SGLIST_NODE(newNode);

	p_newNode->p_next  = p_listNode->p_next;
	p_listNode->p_next = (ListNode_t)p_newNode;

	if (p_newNode->p_next == NULL)
	{
		p_list->p_tail = p_newNode;
	}

	p_list->nodeCount++;

	return ERR_OK;
}

int SGList_DetachNode(List_t list, ListNode_t node)
{
	CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(node != NULL, ERR_BAD_PARAM);

	List_st*       p_list = CONVERT_2_LIST(list);
	SGListNode_st* p_node = NULL;
	SGListNode_st* p_pre  = NULL;
	SGListNode_st* p_cur  = NULL;

	if (p_list->nodeCount <= 1)
	{
		if (p_list->p_head == node)
		{
			p_list->p_head = NULL;
			p_list->p_tail = NULL;
			p_list->nodeCount = 0;

			return ERR_OK;
		}
	}

	if (p_list->p_head == node)
	{
		p_node = (SGListNode_st*)node;
		p_list->p_head = p_node->p_next;
		p_list->nodeCount--;

		return ERR_OK;
	}

	for (p_pre = (SGListNode_st*)p_list->p_head, p_cur = (SGListNode_st*)p_pre->p_next; p_cur != NULL; p_pre = p_cur, p_cur = (SGListNode_st*)p_cur->p_next)
	{
		if (p_cur == node)
		{
			p_pre->p_next = p_cur->p_next;
			if (p_pre->p_next == NULL)
			{
				p_list->p_tail = p_pre;
			}

			p_list->nodeCount--;
			break;
		}
	}

	return ERR_OK;
}

/*=============================================================================*
 *                    Inner function implemention
 *============================================================================*/
static void InsertFirstNode(List_st *p_list, SGListNode_st *p_node)
{
	ASSERT(p_list != NULL);
	ASSERT(p_node != NULL);

	p_list->p_head = p_node;
	p_list->p_tail = p_node;
	p_node->p_next = NULL;

	p_list->nodeCount++;

	return;
}

/*=============================================================================*
 *                                End of file
 *============================================================================*/

