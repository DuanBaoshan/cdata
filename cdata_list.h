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

#ifndef _CDATA_LIST_H_
#define _CDATA_LIST_H_

#include <stdio.h>

#include "cdata_types.h"
#include "cdata_os_adapter.h"

__BEGIN_EXTERN_C_DECL__

typedef enum
{
	LIST_TYPE_DOUBLE_LINK,
	LIST_TYPE_SINGLE_LINK
}ListType_e;

typedef void (*ListFreeData_fn)(void* p_data);

typedef CdataBool (*ListEqual_fn)(void* p_nodeData, void* p_userData);

// Judge whether p_userData less than p_nodeData of a list,
// return true if less than, or else return false
typedef CdataBool (*ListUserLtNode_fn)(void* p_nodeData, void* p_userData);

typedef CdataBool (*ListIsDuplicate_fn)(void* p_leftNodeData, void* p_rightNodeData);

// Judge whether p_userData and p_nodeData satisfy the condition user sets,
// return true if satisfies, or else return false
typedef CdataBool (*ListCondition_fn)(void* p_nodeData, void* p_userData);

typedef struct
{
    CdataIndex_t index;
    ListNode_t   node;
    void*        p_data;
} ListTraverseNodeInfo_t;

typedef void (*ListTraverse_fn)(ListTraverseNodeInfo_t* p_nodeInfo, void* p_userData, CdataBool* p_needStopTraverse);

#define FOR_EACH_IN_LIST(_node_, _list_) for (_node_ = List_GetHeadNL(_list_); _node_ != NULL; _node_ = List_GetNextNodeNL(_list_, _node_))
#define FOR_EACH_IN_DBLIST_REVERSE(_node_, _list_) for (_node_ = List_GetTailNL(_list_); _node_ != NULL; _node_ = List_GetPreNodeNL(_list_, _node_))

//========================================================================================
//                    List related functions
//========================================================================================
int List_Create(ListName_t name, ListType_e type, int dataLength, List_t* p_list);
int List_CreateRef(ListName_t name, ListType_e type, List_t* p_list);

int List_SetFreeDataFunc(List_t list, ListFreeData_fn freeFn);
int List_SetEqualFunc(List_t list, ListEqual_fn equalFn);
int List_SetUserLtNodeFunc(List_t list, ListUserLtNode_fn userLtNodeFn);
int List_SetDuplicateFunc(List_t list, ListIsDuplicate_fn isDuplicateFn);

const char*   List_Name(List_t list);
CdataCount_t  List_Count(List_t list);

void List_Lock(List_t list);
void List_UnLock(List_t list);

int List_Traverse(List_t list, void *p_userData, ListTraverse_fn traverseFn);
int List_TraverseReversely(List_t list, void *p_userData, ListTraverse_fn traverseFn);

int List_Uniquefy(List_t list);

int List_Clear(List_t list);
int List_Destory(List_t list);

//========================================================================================
//                    Data related functions
//========================================================================================
ListNode_t List_InsertData(List_t list, void* p_data);
ListNode_t List_InsertData2Head(List_t list, void* p_data);

ListNode_t List_InsertDataAscently(List_t list, void* p_data);
ListNode_t List_InsertDataDescently(List_t list, void* p_data);

ListNode_t List_InsertDataBefore(List_t list, ListNode_t node, void *p_data);
ListNode_t List_InsertDataBeforeNL(List_t list, ListNode_t node, void *p_data);

ListNode_t List_InsertDataUniquely(List_t list, void* p_data);
ListNode_t List_InsertData2HeadUniquely(List_t list, void* p_data);

ListNode_t List_InsertDataAfter(List_t list, ListNode_t node, void *p_data);
ListNode_t List_InsertDataAfterNL(List_t list, ListNode_t node, void *p_data);

ListNode_t List_InsertDataAtPos(List_t list, void* p_data, CdataIndex_t posIndex);

CdataBool List_DataExists(List_t list, void* p_userData);
CdataBool List_DataExistsByCond(List_t list, void* p_userData, ListCondition_fn conditionFn);

void* List_GetHeadData(List_t list);
void* List_GetHeadDataNL(List_t list);

void* List_GetTailData(List_t list);
void* List_GetTailDataNL(List_t list);

int List_GetMachCount(List_t list, void* p_userData, CdataCount_t* p_count);
int List_GetMachCountByCond(List_t list, void* p_userData, ListCondition_fn conditionFn, CdataCount_t* p_count);

void* List_GetData(List_t list, void* p_userData);
void* List_GetDataBy(List_t list, void* p_userData, ListCondition_fn conditionFn);

void* List_GetDataAtPos(List_t list, CdataIndex_t posIndex);

void* List_DetachData(List_t list, void* p_userData);
void* List_DetachDataByCond(List_t list, void* p_userData, ListCondition_fn conditionFn);

void* List_DetachHeadData(List_t list);
void* List_DetachTailData(List_t list);
void* List_DetachDataAtPos(List_t list, CdataIndex_t posIndex);

//========================================================================================
//                    Node related functions
//========================================================================================
int List_CreateNode(List_t list, void* p_data, ListNode_t* p_node);

int List_DestroyNode(List_t list, ListNode_t node);

int List_InsertNode(List_t list, ListNode_t node);
int List_InsertNode2Head(List_t list, ListNode_t node);

int List_InsertNodeAscently(List_t list, ListNode_t node);
int List_InsertNodeDescently(List_t list, ListNode_t node);

int List_InsertNodeUniquely(List_t list, ListNode_t node);
int List_InsertNode2HeadUniquely(List_t list, ListNode_t node);

int List_InsertNodeBefore(List_t list, ListNode_t listNode, ListNode_t newNode);
int List_InsertNodeBeforeNL(List_t list, ListNode_t listNode, ListNode_t newNode);

int List_InsertNodeAfter(List_t list, ListNode_t listNode, ListNode_t newNode);
int List_InsertNodeAfterNL(List_t list, ListNode_t listNode, ListNode_t newNode);

int List_InsertNodeAtPos(List_t list, ListNode_t node, CdataIndex_t posIndex);

ListNode_t List_GetNodeAtPos(List_t list, CdataIndex_t posIndex);

ListNode_t List_GetHead(List_t list);
ListNode_t List_GetHeadNL(List_t list);

ListNode_t List_GetTail(List_t list);
ListNode_t List_GetTailNL(List_t list);

ListNode_t List_GetPreNode(List_t list, ListNode_t node);
ListNode_t List_GetPreNodeNL(List_t list, ListNode_t node);

ListNode_t List_GetNextNode(List_t list, ListNode_t node);
ListNode_t List_GetNextNodeNL(List_t list, ListNode_t node);

void*        List_GetNodeData(List_t list, ListNode_t node);
void*        List_GetNodeDataNL(List_t list, ListNode_t node);

void*        List_DetachNodeData(List_t list, ListNode_t node);
void*        List_DetachNodeDataNL(List_t list, ListNode_t node);

ListNode_t List_GetFirstMatchNode(List_t list, void* p_userData);
ListNode_t List_GetNextMatchNode(List_t list, ListNode_t startNode, void* p_userData);
ListNode_t List_GetLastMatchNode(List_t list, void* p_userData);
ListNode_t List_GetPreMatchNode(List_t list, ListNode_t startNode, void* p_userData);

ListNode_t List_GetFirstMatchNodeByCond(List_t list, void* p_userData, ListCondition_fn conditionFn);
ListNode_t List_GetNextMatchNodeByCond(List_t list, ListNode_t startNode, void* p_userData, ListCondition_fn conditionFn);
ListNode_t List_GetLastMatchNodeByCond(List_t list, void* p_userData, ListCondition_fn conditionFn);
ListNode_t List_GetPreMatchNodeByCond(List_t list, ListNode_t startNode, void* p_userData, ListCondition_fn conditionFn);

int List_Swap(List_t list, ListNode_t firstNode, ListNode_t secondNode);
int List_SwapPosNL(List_t list, ListNode_t firstNode, ListNode_t secondNode);

int List_DetachNode(List_t list, ListNode_t node);
int List_DetachNodeNL(List_t list, ListNode_t node);

ListNode_t List_DetachHead(List_t list);
ListNode_t List_DetachTail(List_t list);

ListNode_t List_DetachNodeAtPos(List_t list, CdataIndex_t posIndex);

ListNode_t List_DetachNodeByData(List_t list, void* p_userData);
ListNode_t List_DetachNodeByCond(List_t list, void* p_userData, ListCondition_fn conditionFn);

int List_RmNode(List_t list, ListNode_t node);
int List_RmNodeNL(List_t list, ListNode_t node);

int List_RmHead(List_t list);
int List_RmTail(List_t list);

int List_RmNodeAtPos(List_t list, CdataIndex_t posIndex);

int List_RmFirstMatchNode(List_t list, void* p_userData);
int List_RmFirstMatchNodeByCond(List_t list, void* p_userData, ListCondition_fn conditionFn);

CdataCount_t List_RmAllMatchNodes(List_t list, void* p_userData);
CdataCount_t List_RmAllMatchNodesByCond(List_t list, void* p_userData, ListCondition_fn conditionFn);

__END_EXTERN_C_DECL__

#endif /* _CDATA_LIST_H_ */
