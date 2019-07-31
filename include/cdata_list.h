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

/*
1.Why use cdata_list?
#.To simplify the operation of list.
In C++, you want to remove some items from a list, so you call the remove function, but it actually only
moves the items to the end of list, you must call erase to remove them from list eventually. In cdata_list, the remove is 
actually remove, not move.C++ list is too heavy for me, I don't like to use iterator.In linux kernel, there is also list 
implementation in the queue.h, but there is so few functions can be used, and in fact, I cannot understand the design of 
linux kernel list until now, perhaps I am so stupid, :). 

#.To provide more operation about list.In cdata_list, you can insert data ascently, descently and uniquely;
you can detach data, rm data; you can operate on data and node.Both of C++ list and linux kernel list
cannot operate on the node, and sometimes user needs to operate on the node. Furthermore, if you want to lock 
a list in multi-thread,you must create an external mutex, it's so complicated.
So I decide to develop a simple and easily used(I think so at least) list.

2.When use cdata_list module, you need make some decisions first:
#.Which kind of list type you want to use, single link and double link?The most functions in cdata_list module 
are same for the both type of link list, but there are a few functions different for single and double link.
For example, when you call List_TraverseReversely on a single list, it will fail.Another example is List_DetachNode,
the time to detach node of double list is O(1), but for the single list it's O(n).

#.Which kind of data type you want to store, normal data(int, long, float, structure and so on) or pointer.
For the former, the cdata_list module will allocate a same memory in the node, then memcpy user data to the 
node data, then user can destroy data out of list safely.For the latter, the cdata_list module will store 
pointer of the user data directly, and user cannot destroy the data when the node is in use.It's like the 
two ways to send parameter to a function, value copy and value reference.And List_Create is for the former, 
List_CreateRef is for the latter.But when you want to store the C++ class object, you only can use List_CreateRef.

#.How to free the memory of the node data.If the data you stored has no internal pointer, you can let cdata_list 
module to free the node data safely, for example:
struct foo
{
    int value1;
    float value2;
    char value3[256];
};
But if there are internal pointer(s) in your data, you need free it by yourself, so you need call List_SetFreeDataFunc
to set a customer free function, for example:
struct foo
{
    int value1;
    int *p_value2;
    float *p_value3;
};
Because cdata_list module don't know the details of your data, so it cannot free the data correctly.

3.About normal and NL functions.
The functions without suffix "NL" (e.g. List_GetHead, List_InsertData) will call List_Lock and List_UnLock
internally, so they can be called safely in the multi-thread environment.
For example:

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

If you decide to call the "NL" functions(NoLock functions, e.g. List_GetHeadNL, List_GetPreNodeNL) in the multi-thread environment, 
you need call List_Lock and List_UnLock explicitly.
For example:

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

typedef void (*List_FreeData_fn)(void* p_data);

/*
 * To check if the node data is equal to keyword data.
 */
typedef CdataBool (*List_Equal2Keyword_fn)(void* p_nodeData, void* p_keyword);

/*
 * To check if the two node data are equal.
 */
typedef CdataBool (*List_NodeEqual_fn)(void* p_firstNodeData, void* p_secondNodeData);

/* 
 * To check if p_userData less than p_nodeData of a list, return true if less than, or else return false.
 */
typedef CdataBool (*List_UserLtNode_fn)(void* p_nodeData, void* p_userData);

/*
 * To check if p_userData and p_nodeData satisfy the condition user sets,
 * return true if satisfies, or else return false
 */
typedef CdataBool (*List_Condition_fn)(void* p_nodeData, void* p_userData);

typedef struct
{
    CdataIndex_t index;
    ListNode_t   node;
    void*        p_data;
} ListTraverseNodeInfo_t;

/*
 * Visit each data in the list, the p_nodeInfo contains position index, node information and data.If you 
 * want to terminate traverse before it reaches the tail, you can set p_needStopTraverse to TRUE.
 */
typedef void (*List_Traverse_fn)(ListTraverseNodeInfo_t* p_nodeInfo, void* p_userData, CdataBool* p_needStopTraverse);

#define FOR_EACH_IN_LIST(_node_, _list_) for (_node_ = List_GetHeadNL(_list_); _node_ != NULL; _node_ = List_GetNextNodeNL(_list_, _node_))
#define FOR_EACH_IN_DBLIST_REVERSE(_node_, _list_) for (_node_ = List_GetTailNL(_list_); _node_ != NULL; _node_ = List_GetPreNodeNL(_list_, _node_))

//========================================================================================
//                    List related functions
//========================================================================================

/**
 * @brief Create a new list which will store the data as value copy model.
 * @param name: List name.
 * @param type: List type, can be either LIST_TYPE_DOUBLE_LINK or LIST_TYPE_SINGLE_LINK.
 * @param dataLength: The length of data which will be stored into list node. For example,
 *  if you want to store int value in the list, so dataLength will be sizeof(int).
 * @param p_list:Output the new list handle.
 * @return Error code.
 *   @retval ERR_OK: Success
 *   @retval ERR_FAIL: Fail
 *   @retval ERR_BAD_PARAM:Param p_list is NULL. 
 */
int List_Create(ListName_t name, ListType_e type, int dataLength, List_t* p_list);

/**
 * @brief Create a new list which will store the data as value reference model.
 * @param name: List name.
 * @param type: List type, can be either LIST_TYPE_DOUBLE_LINK or LIST_TYPE_SINGLE_LINK.
 * @param p_list:Output the new list handle.
 * @return Error code. 
 *   @retval ERR_OK: Success
 *   @retval ERR_FAIL: Fail
 *   @retval ERR_BAD_PARAM:Param p_list is NULL.
 */
int List_CreateRef(ListName_t name, ListType_e type, List_t* p_list);

/**
 * @brief Set a freeFn to a list, freeFn will be used when free the node data. If not set 
 * the list will free data with free function.
 * @return Error code.
 *   @retval ERR_OK: Success
 *   @retval ERR_BAD_PARAM:Param list is NULL.
 */
int List_SetFreeDataFunc(List_t list, List_FreeData_fn freeFn);

/**
 * @brief Set a equal2KeywordFn to a list, equal2KeywordFn will be used when you call List_InsertDataBefore,
 * List_InsertNodeAfter, List_GetFirstMatchNode and so on.
 * @return Error code.
 *   @retval ERR_OK: Success
 *   @retval ERR_BAD_PARAM:Param p_list is NULL.
 */
int List_SetEqual2KeywordFunc(List_t list, List_Equal2Keyword_fn equal2KeywordFn);

/**
 * @brief Set a nodeEqualFn to a list, nodeEqualFn will be used when you call List_InsertDataUni,
 * List_InsertNodeUni and so on.
 * @return Error code.
 *   @retval ERR_OK: Success
 *   @retval ERR_BAD_PARAM:Param p_list is NULL.
 */
int List_SetNodeEqualFunc(List_t list, List_NodeEqual_fn nodeEqualFn);

/**
 * @brief Set a userLtNodeFn to a list, userLtNodeFn will be used when you call List_InsertDataAsc,
 * List_InsertNodeAsc and so on.
 * @return Error code.
 *   @retval ERR_OK: Success
 *   @retval ERR_BAD_PARAM:Param p_list is NULL.
 */
int List_SetUserLtNodeFunc(List_t list, List_UserLtNode_fn userLtNodeFn);

const char*   List_Name(List_t list);
CdataCount_t  List_Count(List_t list);

/**
 * @brief If you want to call NL functions in multi-thread, you need lock and unlock the
 * list with List_Lock and List_UnLock manually.
 */
void List_Lock(List_t list);

void List_UnLock(List_t list);

/**
 * @brief Visit each node and node data of a list, from head to tail. p_userData will be
 * passed to traverseFn as its parameter p_userData.
 */
int List_Traverse(List_t list, void *p_userData, List_Traverse_fn traverseFn);

/**
 * @brief Visit each node and node data of a list, from tail to head. It will fail if the
 * list is LIST_TYPE_SINGLE_LINK.
 */
int List_TraverseReversely(List_t list, void *p_userData, List_Traverse_fn traverseFn);

/**
 * @brief Free all the nodes and node data of a list.The node count will be 0 after clear.
 * If the node data has a pointer value, you should List_SetFreeDataFunc first, to free the
 * data by yourself in your own freeFn. After cleared, the list can insert data again.
 */
int List_Clear(List_t list);

/**
 * @brief Destroy the list, free all the nodes and node data, the list will be invalid and
 * cannot access any longer.
 */
int List_Destroy(List_t list);

//========================================================================================
//                    Data related functions
//========================================================================================
/**
 * @brief Insert p_data into list tail.
 * @param p_node:Can be NULL, if not NULL, it will return the created node which contains the p_data.
 * @return ListNode_t.
 *   @retval NoNULL: Success
 *   @retval NULL: Fail
 */
ListNode_t List_InsertData(List_t list, void* p_data);

/**
 * @brief Insert p_data into list head.
 * @return ListNode_t:The new created node which contains the p_data. 
 *   @retval New node:Insert successfully.
 *   @retval NULL:Insert failed.
 */
ListNode_t List_InsertData2Head(List_t list, void* p_data);

/**
 * @brief Insert p_data into list, after insert the order of list data is ascending.
 *  For p_data == p_nodeData, if you want p_data to be inserted after p_nodeData, you 
 *  need make p_data < p_nodeData return true in List_UserLtNode_fn; if you want p_data 
 *  to be inserted before p_nodeData, just make p_data <= p_nodeData return true.
 */
ListNode_t List_InsertDataAsc(List_t list, void* p_data);

/**
 * @brief Insert p_data into list, after insert the order of list data is descending.
 *  For p_data == p_nodeData, if you want p_data to be inserted before p_nodeData, you 
 *  need make p_data < p_nodeData return true in List_UserLtNode_fn; if you want p_data 
 *  to be inserted after p_nodeData, just make p_data <= p_nodeData return true.
 */
ListNode_t List_InsertDataDes(List_t list, void* p_data);

/**
 * @brief Insert p_data into list uniquely, if there is a node data equal to p_data, 
 * the insert will fail and return NULL. This function needs List_NodeEqual_fn
 * to check if the two node data are equal.If can insert, the new data will 
 * be inserted to the list tail.
 */
ListNode_t List_InsertDataUni(List_t list, void* p_data);

/**
 * @brief Insert p_data into list uniquely. If can insert, the new data will 
 * be inserted to the list head.
 */
ListNode_t List_InsertData2HeadUni(List_t list, void* p_data);

/**
 * @brief First find the node which data is equal to the p_keyword, if found 
 * then insert p_data before the found node, or else insert fail and return NULL.
 * This function needs List_Equal2Keyword_fn to check if the node data is equal 
 * to the p_keyword.
 */
ListNode_t List_InsertDataBefore(List_t list, void* p_keyword, void *p_data);

/**
 * @brief The no lock version of List_InsertDataBefore. User should call List_Lock and
 * List_UnLock in the multi-thread environment to avoid concurrency access.
 */
ListNode_t List_InsertDataBeforeNL(List_t list, void* p_keyword, void *p_data);

/**
 * @brief Insert the p_data after the node which data is equal to the p_keyword.
 * Also needs List_Equal2Keyword_fn.
 */
ListNode_t List_InsertDataAfter(List_t list, void* p_keyword, void *p_data);
ListNode_t List_InsertDataAfterNL(List_t list, void* p_keyword, void *p_data);

/**
 * @brief Insert the p_data into list at posIndex. If posIndex <= 0, insert to head,
 * if posIndex >= nodeCount, insert to the tail.
 */
ListNode_t List_InsertDataAtPos(List_t list, void* p_data, CdataIndex_t posIndex);

/**
 * @brief Check if there is data equal to p_keyword, needs  List_Equal2Keyword_fn.
 */
CdataBool List_DataExists(List_t list, void* p_keyword);

/**
 * @brief Check if there is data which can make conditionFn(nodeData, p_userData) be true.
 */
CdataBool List_DataExistsByCond(List_t list, void* p_userData, List_Condition_fn conditionFn);

void* List_GetHeadData(List_t list);
void* List_GetHeadDataNL(List_t list);

void* List_GetTailData(List_t list);
void* List_GetTailDataNL(List_t list);

/**
 * @brief Get the count of all data which is equal to p_keyword.Need List_Equal2Keyword_fn.
 */
CdataCount_t List_GetMachCount(List_t list, void* p_keyword);

/**
 * @brief Get the count of all data which make conditionFn(nodeData, p_userData) be true.
 */
CdataCount_t List_GetMachCountByCond(List_t list, void* p_userData, List_Condition_fn conditionFn);

/**
 * @brief Get the data which is equal to p_keyword.If there are multi data equal to p_keyword, only return 
 * the first data.
 */
void* List_GetData(List_t list, void* p_keyword);

/**
 * @brief Get the data which make conditionFn(nodeData, p_userData) be true.If there are multi data 
 *  equal to p_keyword, only return the first data.
 */
void* List_GetDataByCond(List_t list, void* p_userData, List_Condition_fn conditionFn);

void* List_GetDataAtPos(List_t list, CdataIndex_t posIndex);

/**
 * @brief Detach the node which data is equal to p_keyword from list, destroy the node, but
 * return the node data to user.
 */
void* List_DetachData(List_t list, void* p_keyword);

void* List_DetachDataByCond(List_t list, void* p_userData, List_Condition_fn conditionFn);

/**
 * @brief Detach head node from list and destroy it, then return the head data to user.
 */
void* List_DetachHeadData(List_t list);

void* List_DetachTailData(List_t list);
void* List_DetachDataAtPos(List_t list, CdataIndex_t posIndex);

/**
 * @brief Detach head node from list, then destroy both the node and data. If the data contains
 * pointer, it needs custom List_FreeData_fn.
 */
int List_RmHead(List_t list);
int List_RmTail(List_t list);

int List_RmNodeAtPos(List_t list, CdataIndex_t posIndex);

int List_RmFirstMatchNode(List_t list, void* p_keyword);
int List_RmFirstMatchNodeByCond(List_t list, void* p_userData, List_Condition_fn conditionFn);

CdataCount_t List_RmAllMatchNodes(List_t list, void* p_keyword);
CdataCount_t List_RmAllMatchNodesByCond(List_t list, void* p_userData, List_Condition_fn conditionFn);

//========================================================================================
//                    Node related functions
//========================================================================================
/**
 * @brief Create a node which will contain the p_data.
 */
int List_CreateNode(List_t list, void* p_data, ListNode_t* p_node);

/**
 * @brief Destroy the node both with its data, if there is pointer in the data, it needs
 * custom List_FreeData_fn.
 */
int List_DestroyNode(List_t list, ListNode_t node);

/**
 * @brief Insert node to the tail.
 */
int List_InsertNode(List_t list, ListNode_t node);

int List_InsertNode2Head(List_t list, ListNode_t node);

int List_InsertNodeAsc(List_t list, ListNode_t node);
int List_InsertNodeDes(List_t list, ListNode_t node);

int List_InsertNodeUni(List_t list, ListNode_t node);
int List_InsertNode2HeadUni(List_t list, ListNode_t node);

/**
 * @brief Insert newNode before listNode. If list is LIST_TYPE_SINGLE_LINK, the time
 * will be O(n), but for LIST_TYPE_SINGLE_LINK it's O(1).
 */
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

/**
 * @brief Return pre node of the node.It will return NULL if the
 * list is LIST_TYPE_SINGLE_LINK.
 */
ListNode_t List_GetPreNode(List_t list, ListNode_t node);
ListNode_t List_GetPreNodeNL(List_t list, ListNode_t node);

ListNode_t List_GetNextNode(List_t list, ListNode_t node);
ListNode_t List_GetNextNodeNL(List_t list, ListNode_t node);

void*      List_GetNodeData(List_t list, ListNode_t node);
void*      List_GetNodeDataNL(List_t list, ListNode_t node);

/**
 * @brief Detach the data from node, and return the data to user.Then there
 * is nothing in the node.
 */
void*      List_DetachNodeData(List_t list, ListNode_t node);

void*      List_DetachNodeDataNL(List_t list, ListNode_t node);

/**
 * @brief Using List_GetFirstMatchNode and List_GetNextMatchNode, you can visit
 * all the nodes which data is equal to p_keyword from head to tail.
 */
ListNode_t List_GetFirstMatchNode(List_t list, void* p_keyword);
ListNode_t List_GetNextMatchNode(List_t list, ListNode_t startNode, void* p_keyword);

/**
 * @brief Using List_GetFirstMatchNode and List_GetNextMatchNode, you can visit
 * all the nodes which data is equal to p_keyword from tail to head. It will return NULL 
 * if the list is LIST_TYPE_SINGLE_LINK.
 */
ListNode_t List_GetLastMatchNode(List_t list, void* p_keyword);

/**
 *@brief Get the pre matched node from startNode. It will return NULL if the
 * list is LIST_TYPE_SINGLE_LINK.
 */
ListNode_t List_GetPreMatchNode(List_t list, ListNode_t startNode, void* p_keyword);

ListNode_t List_GetFirstMatchNodeByCond(List_t list, void* p_userData, List_Condition_fn conditionFn);
ListNode_t List_GetNextMatchNodeByCond(List_t list, ListNode_t startNode, void* p_userData, List_Condition_fn conditionFn);

/**
 *@brief It will return NULL if the list is LIST_TYPE_SINGLE_LINK.
 */
ListNode_t List_GetLastMatchNodeByCond(List_t list, void* p_userData, List_Condition_fn conditionFn);

/**
 *@brief It will return NULL if the list is LIST_TYPE_SINGLE_LINK.
 */
ListNode_t List_GetPreMatchNodeByCond(List_t list, ListNode_t startNode, void* p_userData, List_Condition_fn conditionFn);

/**
 * @brief Swap the data between firstNode and secondNode.
 */
int List_Swap(List_t list, ListNode_t firstNode, ListNode_t secondNode);

/**
 * @brief Detach node from the list, so the node will not belong to the list any longer.
 * The time is O(n) if the list is LIST_TYPE_SINGLE_LINK, for the LIST_TYPE_DOUBLE_LINK
 * it is O(1).
 */
int List_DetachNode(List_t list, ListNode_t node);
int List_DetachNodeNL(List_t list, ListNode_t node);

/**
 * @brief Detach the head node from list then return it to user.
 */
ListNode_t List_DetachHead(List_t list);

ListNode_t List_DetachTail(List_t list);

ListNode_t List_DetachNodeAtPos(List_t list, CdataIndex_t posIndex);

ListNode_t List_DetachNodeByKey(List_t list, void* p_keyword);
ListNode_t List_DetachNodeByCond(List_t list, void* p_userData, List_Condition_fn conditionFn);

/**
 * @brief Detach node from the list and destroy it. If there is pointer in the node data, it needs
 * custom List_FreeData_fn.The time is O(n) if the list is LIST_TYPE_SINGLE_LINK, for the LIST_TYPE_DOUBLE_LINK
 * it is O(1).
 */
int List_RmNode(List_t list, ListNode_t node);

int List_RmNodeNL(List_t list, ListNode_t node);

__END_EXTERN_C_DECL__

#endif /* _CDATA_LIST_H_ */
