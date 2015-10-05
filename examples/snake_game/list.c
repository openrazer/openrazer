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

#include "list.h"


list *list_Create(long num, unsigned char flags)
{
	list *l = (list*)malloc(sizeof(list));
	if(num)
	{
		l->items = (void**)malloc(num * sizeof(void*));
		memset(l->items,0,num*sizeof(void*));
	}
	else
		l->items = NULL;
	l->num = num;
        l->iteration_index = 0;
	l->flags = flags;
	return(l);
}

void list_Close(list *l)
{
	if(l->num > 0)
		free(l->items);
	free(l);
}

list *list_Copy(list *l)
{
       list *r = list_Create(l->num,l->flags);
       memcpy(r->items,l->items,sizeof(void*)*l->num);
       return(r);
}

long list_Push(list *l, void *obj)
{
	if(!l->num)
	{
		l->num = 1;
		l->items = (void**)malloc(l->num * sizeof(void*));
		l->items[0] = obj;
		return(0);
	}
	else
	{
		l->items = (void**)realloc(l->items,(l->num + 1) * sizeof(void*));
		l->items[l->num] = obj;
		l->num++;
		return(l->num-1);
	}
}

void *list_Pop(list *l)
{
	if(l->num)
	{
		void *tmp = list_Remove(l, l->num - 1);
		return(tmp);
	}
	return(0);
}

void *list_GetTop(list *l)
{
	void *r = NULL;
    if(l->num)
    {
		r = l->items[l->num-1];
	}
	return(r);
}

void *list_GetBottom(list *l)
{
	void *r = NULL;
    if(l->num)
    {
		r = l->items[0];
	}
	return(r);
}

void list_SetLen(list *l,long len)
{
	if(len!=l->num)
	{
		l->items = (void**)realloc(l->items, len * sizeof(void*));
		l->num = len;
	}
}

BOOL list_Insert(list *l, long index, void *obj)
{
	if(index == 0 && !l->num)
	{
		list_Push(l, obj);
	}
	else if(index <= l->num - 1)
	{
		l->num++;
		l->items = (void**)realloc(l->items, (l->num) * sizeof(void*));
		for(long i = l->num - 2; i >= index; i--)
		{
			list_MoveDown(l, i);
		}
		l->items[index] = obj;
		return(1);
	}
	else if(index > l->num - 1)
		list_Push(l, obj);
	return(0);
}

void list_MoveUp(list *l, long index)
{
	if(!l->num)
	{
		return;
	}
	if(index < l->num && index > 0)
	{
		l->items[index - 1] = l->items[index];
	}
}

void list_MoveDown(list *l, long index)
{
	if(!l->num)
	{
		return;
	}
	if(index < l->num - 1 && index >= 0)
	{
		l->items[index + 1] = l->items[index];
	}
}

void *list_Remove(list *l, long index)
{
	if(!l->num)
	{
		return(0);
	}
	if(index < l->num)
	{
		void *tmp = l->items[index];
		long i=index+1;
		while(i<l->num)
		{
			list_MoveUp(l,i);
			i++;
		}	
		if((l->num - 1) == 0)
		{
			free(l->items);
			l->items = 0;
			l->num = 0;
		}
		else
		{
			l->items = (void**)realloc(l->items, (l->num - 1) * sizeof(void*));
			l->num--;
		}
		return(tmp);
	}
	return(0);
}

void list_Clear(list *l)
{
	if(l->num)
	{
		free(l->items);
		l->items = NULL;
	}
	l->num = 0;
}

long list_GetLen(list *l)
{
	long r = l->num;
	return(r);
}

void *list_Get(list *l,long index)
{
	void *r = NULL;
	if(index < l->num)
	{
		r = l->items[index];
	}
	return(r);
}

void list_Set(list *l,long index, void *obj)
{
	if(index < l->num)
	{
		l->items[index] = obj;
	}
}

void list_Queue(list *l, void *obj)
{
	list_Insert(l, 0, obj);
}

void *list_Dequeue(list *l)
{
	return(list_Remove(l, 0));
}

BOOL list_Contains(list *l,void *obj)
{
	for(int i=0;i<l->num;i++)
	{	
		if(l->items[i] == obj)
		{
			return(1);
		}
	}
	return(0);
}

long list_GetIndex(list *l, void *obj)
{
	for(int i=0;i<l->num;i++)
	{	
		if(l->items[i] == obj)
		{
			return(i);
		}
	}
	return(-1);
}

long list_RemoveItem(list *l, void *obj)
{
	long index = list_GetIndex(l,obj);
	if(index != -1)
		list_Remove(l,index);
	return(index);
}

BOOL list_IsEmpty(list *l)
{
	BOOL r = !l->num;
	return(r);
}

//TODO iteration and removal/insertion handling

void *list_Iterate(list *l)
{
  void *r = l->items[l->iteration_index];
  l->iteration_index++;
  return(r);
}

void *list_ReverseIterate(list *l)
{
  void *r = l->items[l->iteration_index];
  l->iteration_index--;
  return(r);
}

int list_IterationUnfinished(list *l)
{
  return(l->iteration_index < l->num);
}

int list_ReverseIterationUnfinished(list *l)
{
  return(l->iteration_index >= 0);
}

void list_IterationReset(list *l)
{
  l->iteration_index = 0;
}

void list_ReverseIterationReset(list *l)
{
  l->iteration_index = l->num-1;
}

long list_GetIterationIndex(list *l)
{
  return(l->iteration_index);
}

void list_SetIterationIndex(list *l,long iteration_index)
{
  l->iteration_index = iteration_index;
}

