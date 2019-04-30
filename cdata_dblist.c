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
Date:2019.3.28
*/

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <semaphore.h>

#include "cdata_types.h"
#include "list_internal.h"
#include "cdata_dblist.h"

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
static void        InsertBefore(List_st *p_list, DBListNode_st *p_listNode, DBListNode_st *p_newNode);

static void        InsertFirstNode(List_st *p_list, DBListNode_st *p_node);
static void        Insert2Tail(List_st *p_list, DBListNode_st *p_node);

/*=============================================================================*
 *                    Outer function implemention
 *============================================================================*/
int DBList_CreateNode(List_t list, void* p_data, ListNode_t *p_node)
{
	CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(p_data != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(p_node != NULL, ERR_BAD_PARAM);

    DBListNode_st* p_newNode = NULL;
    List_st*     p_list    = CONVERT_2_LIST(list);

    p_newNode = (DBListNode_st*) OS_Malloc(sizeof(DBListNode_st));
    if (NULL == p_newNode)
    {
        LOG_E("DBList_CreateNode() : Not enough memory 1\n");
        return ERR_OUT_MEM;
    }
    memset(p_newNode, 0x0, sizeof(ListNode_t));

    p_newNode->p_pre = NULL;
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

int DBList_InsertNode(List_t list, ListNode_t node)
{
    CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(node != NULL, ERR_BAD_PARAM);

    List_st*     p_list = CONVERT_2_LIST(list);
    DBListNode_st* p_node = CONVERT_2_DBLIST_NODE(node);

    Insert2Tail(p_list, p_node);

    return ERR_OK;
}

int DBList_InsertNode2Head(List_t list, ListNode_t node)
{
    CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(node != NULL, ERR_BAD_PARAM);

    List_st*     p_list = CONVERT_2_LIST(list);
    DBListNode_st* p_node = CONVERT_2_DBLIST_NODE(node);

    if (p_list->nodeCount == 0)
    {
        InsertFirstNode(p_list, p_node);
        return ERR_OK;
    }

    p_node->p_next = p_list->p_head;
    p_node->p_pre = NULL;

    ((DBListNode_st*)p_list->p_head)->p_pre = p_node;
    p_list->p_head = p_node;

    p_list->nodeCount++;

    return ERR_OK;
}

int   DBList_InsertNodeAscently(List_t list, ListNode_t node)
{
    CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(node != NULL, ERR_BAD_PARAM);

    List_st*        p_list    = CONVERT_2_LIST(list);
    DBListNode_st*  p_newNode = CONVERT_2_DBLIST_NODE(node);
	DBListNode_st*  p_node    = NULL;

    for (p_node = p_list->p_head; p_node != NULL; p_node = p_node->p_next)
    {
        if (p_list->usrLtNodeFn(p_node->p_data, p_newNode->p_data))
        {
            InsertBefore(p_list, p_node, p_newNode);
            return ERR_OK;
        }
    }

    Insert2Tail(p_list, p_newNode);

    return ERR_OK;
}

int   DBList_InsertNodeDescently(List_t list, ListNode_t node)
{
    CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(node != NULL, ERR_BAD_PARAM);

    List_st*        p_list    = CONVERT_2_LIST(list);
    DBListNode_st*  p_newNode = CONVERT_2_DBLIST_NODE(node);
	DBListNode_st*  p_node    = NULL;

    for (p_node = p_list->p_head; p_node != NULL; p_node = p_node->p_next)
    {
        if (!p_list->usrLtNodeFn(p_node->p_data, p_newNode->p_data))
        {
            InsertBefore(p_list, p_node, p_newNode);
            return ERR_OK;
        }
    }

    Insert2Tail(p_list, p_newNode);

    return ERR_OK;
}

int DBList_InsertNodeBefore(List_t list, ListNode_t listNode, ListNode_t newNode)
{
    CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(listNode != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(newNode != NULL, ERR_BAD_PARAM);

    List_st*      p_list      = CONVERT_2_LIST(list);
    DBListNode_st*  p_listNode   = CONVERT_2_DBLIST_NODE(listNode);
	DBListNode_st*  p_newNode   = CONVERT_2_DBLIST_NODE(newNode);

	InsertBefore(p_list, p_listNode, p_newNode);

	return ERR_OK;
}

int DBList_InsertNodeAfter(List_t list, ListNode_t listNode, ListNode_t newNode)
{
    CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(listNode != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(newNode != NULL, ERR_BAD_PARAM);

    List_st*      p_list      = CONVERT_2_LIST(list);
    DBListNode_st*  p_listNode   = CONVERT_2_DBLIST_NODE(listNode);
	DBListNode_st*  p_newNode   = CONVERT_2_DBLIST_NODE(newNode);

	p_newNode->p_next = p_listNode->p_next;
	p_newNode->p_pre = p_listNode;
	p_listNode->p_next = p_newNode;

	if (p_newNode->p_next == NULL)
	{
		p_list->p_tail = p_newNode;
	}
	else
	{
		p_newNode->p_next->p_pre = p_newNode;
	}

    p_list->nodeCount++;

	return ERR_OK;
}

int DBList_DetachNode(List_t list, ListNode_t node)
{
    CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(node != NULL, ERR_BAD_PARAM);

    List_st *       p_list = CONVERT_2_LIST(list);
    DBListNode_st * p_node = CONVERT_2_DBLIST_NODE(node);

    if (0 == p_list->nodeCount)
    {
        return ERR_OK;
    }

    if (p_node == p_list->p_head)
    {
        p_list->p_head = p_node->p_next;
        if (p_list->p_head != NULL)
        {
            ((DBListNode_st*)p_list->p_head)->p_pre = NULL;
        }
    }
    else if (p_node == p_list->p_tail)
    {
        p_list->p_tail = p_node->p_pre;
        if (p_list->p_tail != NULL)
        {
            ((DBListNode_st*)p_list->p_tail)->p_next = NULL;
        }
    }
    else
    {
        if (p_node->p_pre != NULL)
        {
            p_node->p_pre->p_next = p_node->p_next;
        }

        if (p_node->p_next != NULL)
        {
            p_node->p_next->p_pre = p_node->p_pre;
        }
    }

    p_list->nodeCount--;

    return ERR_OK;
}
/*=============================================================================*
 *                    Inner function implemention
 *============================================================================*/
static void InsertBefore(List_st* p_list, DBListNode_st* p_listNode, DBListNode_st* p_newNode)
{
    ASSERT(p_list != NULL);
    ASSERT(p_listNode != NULL);
    ASSERT(p_newNode != NULL);

	p_newNode->p_pre = p_listNode->p_pre;

	p_listNode->p_pre = p_newNode;
	p_newNode->p_next = p_listNode;

	if (p_newNode->p_pre == NULL)
	{
		p_list->p_head = p_newNode;
	}
	else
	{
		p_newNode->p_pre->p_next = p_newNode;
	}

    p_list->nodeCount++;

    return;
}

static void InsertFirstNode(List_st *p_list, DBListNode_st *p_node)
{
    ASSERT(p_list != NULL);
    ASSERT(p_node != NULL);

    p_list->p_head = p_node;
    p_list->p_tail = p_node;

    p_node->p_next = NULL;
    p_node->p_pre = NULL;

    p_list->nodeCount++;

    return;
}

static void Insert2Tail(List_st* p_list, DBListNode_st* p_node)
{
    ASSERT(p_list != NULL);
    ASSERT(p_node != NULL);

    if (p_list->nodeCount == 0)
    {
        InsertFirstNode(p_list, p_node);
        return ;
    }

    p_node->p_pre = p_list->p_tail;
    p_node->p_next = NULL;
    ((DBListNode_st*)p_list->p_tail)->p_next = p_node;
    p_list->p_tail = p_node;

    p_list->nodeCount++;

    return;
}

/*=============================================================================*
 *                                End of file
 *============================================================================*/
