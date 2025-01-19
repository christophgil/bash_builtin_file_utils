#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
static void cg_removePartOfString(char *f, char *t){
  assert(f>t);
  while((*t++=*f++));
}
static int cg_normalizePath(char * const to, const int size,const char * const from){
  *to=0;
  if (!from || !*from) return 0;
  if (strlen(from)>=size) return ENAMETOOLONG;
  char *t=to,*f=(char*)from;
  if (*f=='~' && f[1]=='/'){ /* ~/ */
    static char *h;
    if (!h && !(h=getenv("HOME"))){
      h="~/";
      fprintf(stderr,"HOME empty \n");
    }
    t=stpcpy(t,h);
    f++;
  }else if (*f!='/'){ /* relative path */
    if (!getcwd(t,size)){
      fprintf(stderr,"getcwd too large \n");
      return ENAMETOOLONG;
    }
    (t+=strlen(t)+1)[-1]='/';
  }
  for(;*f;f++){
    if (t>to && t[-1]=='/'){
      if (*f=='/') continue; /* double slash */
      if (*f=='.' && f[1]=='/'){ f++; continue;} /* /./ */
    }
    *t++=*f;
  }
  *t=0;
  /* Now specific path conversions */
  /*  /s-mcpb-ms03/fularchiv01/1/  or  /s-mcpb-ms03/fularchiv01.d/EquiValent/151/1/   =>  /s-mcpb-ms03/store/ */
  if ((f=strstr(to,".charite.de/"))) cg_removePartOfString(f+11,f);
  if (!strncmp("/s-mcpb-ms03/fularchiv01",to,24) && (f=strstr(to+24,"/1/")) && f<to+42){
    cg_removePartOfString(f+2,(char*)stpcpy(to+13,"store"));
  }
  return 0;
}


#if defined(__INCLUDE_LEVEL__) && __INCLUDE_LEVEL__==0

int main(int argc, char *argv[]){
  char t[PATH_MAX+1];
  for(int i=1;i<argc;i++){
    cg_normalizePath(t,sizeof(t),argv[i]);
    printf("\n%s -> %s\n",argv[i],t);


  }
  return 0;
}
#endif
