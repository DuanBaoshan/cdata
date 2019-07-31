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
#ifndef _CDATA_SGLIST_H_
#define _CDATA_SGLIST_H_

#include "cdata_types.h"

__BEGIN_EXTERN_C_DECL__

int SGList_CreateNode(List_t list, void* p_data, ListNode_t* p_node);

int SGList_InsertDataBefore(List_t list, void* p_keyword, ListNode_t newNode);
int SGList_InsertDataAfter(List_t list,  void* p_keyword, ListNode_t newNode);

int SGList_InsertNode(List_t list, ListNode_t node);
int SGList_InsertNode2Head(List_t list, ListNode_t node);

int SGList_InsertNodeAsc(List_t list, ListNode_t node);
int SGList_InsertNodeDes(List_t list, ListNode_t node);

int SGList_InsertNodeBefore(List_t list, ListNode_t listNode, ListNode_t newNode);
int SGList_InsertNodeAfter(List_t list, ListNode_t listNode, ListNode_t newNode);

int SGList_DetachNode(List_t list, ListNode_t node);

__END_EXTERN_C_DECL__

#endif //_CDATA_SGLIST_H_
