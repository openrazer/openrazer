/* 
 * node.c - c based data tree with various imports & exports
 * (c) 2013 by Tim Theede aka Pez2001 <pez2001@voyagerproject.de> / vp
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *
 * THIS SOFTWARE IS SUPPLIED AS IT IS WITHOUT ANY WARRANTY!
 *
 */


#ifndef LIST_H
#define LIST_H


//#define LIST_AUTOMATIC_


#include "stdio.h"
#include "stdarg.h"
#include "stdlib.h"
#include "string.h"


#ifdef USE_MEMORY_DEBUGGING
#include "memory.h"
#endif


#ifdef __cplusplus
extern "C"  {
#endif

#ifdef WIN32
//#define _WIN32_WINNT  0x501 
#include "windows.h"
#else
#define BOOL int
//#include "windef.h"
#endif

#define TRUE 1
#define FALSE 0
#define True 1
#define False 0


typedef struct
{
	void **items; 
	long num;
    long iteration_index;
	unsigned char flags;
} list;


//#define LIST_STATIC 1		// TODO add support for static lists , so indices wont change

/**
 * list_() - 
 * @l: list to use
 *
 * 
 *
 * Return: 
 */
list *list_Create(long num, unsigned char flags); /* returns item_list*/

list *list_Copy(list *l);

void list_Close(list *l);

long list_Push(list *l, void *obj);

void *list_Pop(list *l);

void *list_GetTop(list *l);

void *list_GetBottom(list *l);

BOOL list_Insert(list *l, long index, void *obj);

void *list_Remove(list *l, long index);

long list_RemoveItem(list *l, void *obj);

long list_GetIndex(list *l, void *obj);

void list_Clear(list *l);

void list_SetLen(list *l,long len);

long list_GetLen(list *l);

void *list_Get(list *l, long index);

void list_Set(list *l, long index, void *obj);

void list_Queue(list *l, void *obj);

void *list_Dequeue(list *l);

BOOL list_IsEmpty(list *l);

BOOL list_Contains(list *l,void *obj);

void list_MoveUp(list *l, long index);

void list_MoveDown(list *l, long index);

void *list_Iterate(list *l);

void *list_ReverseIterate(list *l);

void list_IterationReset(list *l);

void list_ReverseIterationReset(list *l);

int list_IterationUnfinished(list *l);

int list_ReverseIterationUnfinished(list *l);

long list_GetIterationIndex(list *l);

void list_SetIterationIndex(list *l,long iteration_index);

#ifdef __cplusplus
} 
#endif


#endif
