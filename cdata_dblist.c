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

static CdataBool IsNeighbour(DBListNode_st *p_firstNode, DBListNode_st *p_secondNode);
static void        SwapNeighbour(List_st *p_list, DBListNode_st *p_firstNode, DBListNode_st *p_secondNode);

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

int DBList_SwapPos(List_t list, ListNode_t firstNode, ListNode_t secondNode)
{
    CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(firstNode != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(secondNode != NULL, ERR_BAD_PARAM);

    List_st*     p_list = CONVERT_2_LIST(list);

    DBListNode_st* p_first  = CONVERT_2_DBLIST_NODE(firstNode);
    DBListNode_st* p_second = CONVERT_2_DBLIST_NODE(secondNode);

    DBListNode_st* p_firstPreNode  = p_first->p_pre;
    DBListNode_st* p_firstNextNode = p_first->p_next;

    DBListNode_st* p_secondPreNode  = p_second->p_pre;
    DBListNode_st* p_secondNextNode = p_second->p_next;

    if (firstNode == secondNode)
    {
        return ERR_OK;
    }

    if (IsNeighbour(p_first, p_second))
    {
        SwapNeighbour(p_list, p_first, p_second);
        return ERR_OK;
    }

    p_first->p_next = p_secondNextNode;
    p_first->p_pre = p_secondPreNode;

    p_second->p_next = p_firstNextNode;
    p_second->p_pre = p_firstPreNode;

    //firstNode is head
    if (p_firstPreNode == NULL)
    {
        p_list->p_head = p_second;
    }
    else
    {
        p_firstPreNode->p_next = p_second;
    }

    //firstNode is tail
    if (p_firstNextNode == NULL)
    {
        p_list->p_tail = p_second;
    }
    else
    {
        p_firstNextNode->p_pre = p_second;
    }

    //secondNode is head
    if (p_secondPreNode == NULL)
    {
        p_list->p_head = p_first;
    }
    else
    {
        p_secondPreNode->p_next = p_first;
    }

    //secondNode is tail
    if (p_secondNextNode == NULL)
    {
        p_list->p_tail = p_first;
    }
    else
    {
        p_secondNextNode->p_pre = p_first;
    }

    return ERR_OK;
}

int DBList_DetachNode(List_t list, ListNode_t node)
{
    CHECK_PARAM(list != NULL, ERR_BAD_PARAM);
    CHECK_PARAM(node != NULL, ERR_BAD_PARAM);

    List_st *       p_list = CONVERT_2_LIST(list);
    DBListNode_st * p_node = CONVERT_2_DBLIST_NODE(node);

	LOG_A("Enter.\n");

    if (0 == p_list->nodeCount)
    {
        return ERR_OK;
    }

    if (p_node == p_list->p_head)
    {
		LOG_A("Detach head.\n");
        p_list->p_head = p_node->p_next;
        if (p_list->p_head != NULL)
        {
            ((DBListNode_st*)p_list->p_head)->p_pre = NULL;
        }
    }
    else if (p_node == p_list->p_tail)
    {
		LOG_A("Detach tail.\n");
        p_list->p_tail = p_node->p_pre;
        if (p_list->p_tail != NULL)
        {
            ((DBListNode_st*)p_list->p_tail)->p_next = NULL;
        }
    }
    else
    {
		LOG_A("Detach other.\n");
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

static CdataBool IsNeighbour(DBListNode_st *p_firstNode, DBListNode_st *p_secondNode)
{
    ASSERT(p_firstNode != NULL);
    ASSERT(p_secondNode != NULL);

	return (p_firstNode->p_next == p_secondNode) || (p_firstNode->p_pre == p_secondNode) || (p_secondNode->p_next == p_firstNode) || (p_secondNode->p_pre == p_firstNode);
}

static void  SwapNeighbour(List_st *p_list, DBListNode_st *p_firstNode, DBListNode_st *p_secondNode)
{
    ASSERT(p_list != NULL);
    ASSERT(p_firstNode != NULL);
    ASSERT(p_secondNode != NULL);

    DBListNode_st *p_actualFirst = NULL;
    DBListNode_st *p_actualSecond = NULL;

    DBListNode_st *p_firstPre = NULL;
    DBListNode_st *p_secondNext = NULL;

    if (p_firstNode->p_next == p_secondNode)
    {
        p_actualFirst = p_firstNode;
        p_actualSecond = p_secondNode;
    }
    else
    {
        p_actualFirst = p_secondNode;
        p_actualSecond = p_firstNode;
    }

    p_firstPre = p_actualFirst->p_pre;
    p_secondNext = p_actualSecond->p_next;

    p_actualFirst->p_pre = p_actualSecond;
    p_actualFirst->p_next = p_secondNext;

    p_actualSecond->p_pre = p_firstPre;
    p_actualSecond->p_next = p_actualFirst;

    //p_actualFirst is head
    if (p_firstPre == NULL)
    {
        p_list->p_head = p_actualSecond;
    }
    else
    {
        p_firstPre->p_next = p_actualSecond;
    }

    //p_actualSecond is tail
    if (p_secondNext == NULL)
    {
        p_list->p_tail = p_actualFirst;
    }
    else
    {
        p_secondNext->p_pre = p_actualFirst;
    }

    return;
}

/*=============================================================================*
 *                                End of file
 *============================================================================*/
