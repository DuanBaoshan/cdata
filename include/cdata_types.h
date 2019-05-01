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

#ifndef _CDATA_TYPES_H_
#define _CDATA_TYPES_H_

#include <stdio.h>
#include "errvalue.h"

#ifdef __cplusplus
#undef  __BEGIN_EXTERN_C_DECL__
#undef  __END_EXTERN_C_DECL__
#define  __BEGIN_EXTERN_C_DECL__ extern "C"{
#define  __END_EXTERN_C_DECL__ }
#else
#undef  __BEGIN_EXTERN_C_DECL__
#undef  __END_EXTERN_C_DECL__
#define  __BEGIN_EXTERN_C_DECL__
#define  __END_EXTERN_C_DECL__
#endif

#define CDATA_TRUE (1 == 1)
#define CDATA_FALSE !(CDATA_TRUE)


typedef unsigned long long CdataCount_t;
typedef unsigned long long CdataIndex_t;
typedef unsigned long long CdataTime_t;

typedef int CdataBool;

typedef void* List_t;
typedef void* ListNode_t;
typedef char  ListName_t[256];

typedef void* Queue_t;
typedef char  QueueName_t[256];

#endif //_CDATA_TYPES_H_