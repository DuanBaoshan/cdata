#ifndef _LIST_INTERNAL_H_
#define _LIST_INTERNAL_H_

#include "cdata_types.h"
#include "cdata_list.h"

typedef enum
{
    //List will allocate the same size memory for node data as user data, and memcpy usr data to node data.
    LIST_DATA_TYPE_VALUE_COPY = 0,
	
    //List don't allocate memory for the data of the node.
    LIST_DATA_TYPE_VALUE_REFERENCE,
} List_DataType_e;

typedef struct _List_
{
	ListType_e	   		type;
	
    void* 				p_head;
    void* 				p_tail;

    List_DataType_e 	dataType;
    int 				dataLength;

    OSMutex_t* 			p_listGuard;	
	ListEqual_fn		equalFn;
	ListFreeData_fn 	freeFn; 
	ListUserLtNode_fn   usrLtNodeFn;
	ListIsDuplicate_fn	isDuplicateFn;
	
    CdataCount_t 		nodeCount;
    ListName_t 		name;
}List_st;

typedef struct _DBListNode_s
{
    struct _DBListNode_s* p_next;
    struct _DBListNode_s* p_pre;
    void* p_data;
}DBListNode_st;

typedef struct _SGListNode_s
{
    struct _DBListNode_s* p_next;
    void* p_data;
}SGListNode_st;

#define CONVERT_2_LIST(_list_) (List_st*)(_list_)
#define CONVERT_2_DBLIST_NODE(node) (struct _DBListNode_s*)(node)
#define CONVERT_2_SGLIST_NODE(node) (struct _SGListNode_s*)(node)

#endif //_LIST_INTERNAL_H_