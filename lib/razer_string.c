/* 
 * razer_chroma_drivers - a driver/tools collection for razer chroma devices
 * (c) 2015 by Tim Theede aka Pez2001 <pez2001@voyagerproject.de> / vp
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

/*some string routines from node.c*/
#include "razer_string.h"


char *str_Sub(char *a,long start,long len)
{
  if(a==NULL || (long)strlen(a)<=start)
  {
    return(str_CreateEmpty());
  }
  long l=0;
  if(len>0)
  {
    long e = start + len;
    l=len;
    if(e>(long)strlen(a))
      l = strlen(a) - start;
  }
  else
  {
    l = strlen(a) - start;
  }
  char *tmp = (char*)malloc(l + 1);
  memset(tmp+l, 0, 1);
  memcpy(tmp, a+start, l);
  return(tmp);
}

long str_Tokenize(char *string,char *splitter,list **tokens)
{
  long tokens_num=0;
  long offset = 0;
  long splitter_len = strlen(splitter);
  long string_len = strlen(string);
  *tokens=list_Create(0,0);
  char *pos=NULL;
  while((pos = strstr(string+offset,splitter)))
  {
    long index = (long)(pos-(string+offset));
    if(index)
    {
      char *sub = str_Sub(string+offset,0,index);
      list_Push(*tokens,sub);
      tokens_num++;
    }
    offset+=index+splitter_len;
  } 
  if(offset<string_len)
  {
    char *sub = str_Sub(string+offset,0,string_len-offset);
    list_Push(*tokens,sub);
    tokens_num++;
  }

  return(tokens_num);
}

void str_FreeTokens(list *tokens)
{
  list_IterationReset(tokens);
  while(list_IterationUnfinished(tokens))
  {
    char *token = list_Iterate(tokens);
    free(token);
  }
  list_Close(tokens);
}
char *str_CreateEmpty(void)
{
    char *string = (char*)malloc(1);
    string[0] = 0;
    return(string);
}

char *str_Copy(char *src)
{
  if(src==NULL)
  	return(str_CreateEmpty());
  char *a = (char*)malloc(strlen(src)+1);
  memcpy(a, src, strlen(src)+1);
  return(a);
}

char *str_Cat(char *a,char *b)
{
  if(a == NULL && b != NULL)
    return(str_Copy(b));
  else
    if(a != NULL && b == NULL)
      return(str_Copy(a));
  else
    if(a == NULL && b == NULL)
      return(str_CreateEmpty());
  char *tmp = (char*)malloc(strlen(a) + strlen(b) + 1);
  memcpy(tmp, a, strlen(a));
  memcpy(tmp + strlen(a), b, strlen(b)+1);
  return(tmp);
}

char *str_CatFree(char *a,char *b)
{
  if(a == NULL && b != NULL)
    return(str_Copy(b));
  else
    if(a != NULL && b == NULL)
      return(a);
  else
    if(a == NULL && b == NULL)
      return(str_CreateEmpty());
  char *tmp = (char*)malloc(strlen(a) + strlen(b) + 1);
  memcpy(tmp, a, strlen(a));
  memcpy(tmp + strlen(a), b, strlen(b)+1);
  free(a);
  return(tmp);
}

char *str_FromLong(long i)
{
  char *ret=NULL;
  long len = snprintf(NULL,0,"%ld",i);
  if(len)
  {
    ret = (char*)malloc(len+1);
    snprintf(ret,len+1,"%ld",i);
  }
  else
    ret=str_CreateEmpty();
  return(ret);
} 

char *str_FromDouble(double d)
{
  char *ret=NULL;
  long len = snprintf(NULL,0,"%f",d);
  if(len)
  {
    ret = (char*)malloc(len+1);
    snprintf(ret,len+1,"%f",d);
  }
  else
    ret=str_CreateEmpty();
  return(ret);
} 
