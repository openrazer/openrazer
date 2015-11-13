/*some string routines from node.c*/
#include "razer_string.h"

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
