/* Author: Christoph Gille   Licence: GNU */
#include "time.h"
#include "sys/stat.h"
#include "sys/param.h"
#include "bash/loadables.h"
#include "cg_bashbuiltin.h"
#include "cg_normalizePath.c"
#define DEBUG 0

#define I(id) ID_##id
enum builtin_type{I(cg_fileLastModified),I(cg_fileSize),I(cg_fileSizeCommaLastModified),I(cg_fileUnmodifiedSeconds),I(cg_areFilesUnmodifiedSeconds),SEPARATOR_NEEDS_STAT,
                  I(cg_normalizePath),
                  I(cg_currentTimeSeconds),I(cg_lowercase_global_variables)};



struct cg_list_of_strings{ char **ss; int ss_l,ss_capacity; };
#define cg_array_contains(v,s) (!cg_add_to_array(true,v,s))
static bool cg_add_to_array(const bool dryrun, struct cg_list_of_strings *v,const char *s){
  if (!v->ss_l || !v->ss)  v->ss=calloc(sizeof(void*),v->ss_capacity=16);
  for(int i=0;i<v->ss_l;i++){
    const char *t=v->ss[i];
    if (*s==*t && !strcmp(s,t)) return false;
  }
  if (!dryrun){
    if (v->ss_l>=v->ss_capacity) v->ss=realloc(v->ss,sizeof(void*)*(v->ss_capacity*=2));
    v->ss[v->ss_l++]=strdup(s);
  }
  return true;
}

static int print_global_lowercase_vars(const bool verbose){
  static struct cg_list_of_strings v;
  static bool report;
  //  fprintf(stderr,"Entered print_global_lowercase_vars ss_l: %d  report: %d\n",v.ss_l,report);
  int res=EXECUTION_SUCCESS;
  SHELL_VAR **vars=all_shell_variables();
  if (!vars){ report_error("all_shell_variables() returns NULL\n"); EXIT_SHELL_RETURN();}
  for(int i=0; vars[i]; i++){
    SHELL_VAR *var=vars[i];
    if (!invisible_p(var) && !local_p(var)){
      const char *n=var->name;
      if (n && 'a'<=*n && *n<='z' && cg_add_to_array(report,&v,n) && report){
        print_assignment(var);
        if (report) res=EXECUTION_FAILURE;
      }
    }
  }
  free(vars);
  if (!report && verbose) fprintf(stderr,"Note: Stored %d names of global lower case variables.\n"
                       "To check whether there are more due to forgotten keyword "ANSI_FG_BLUE"local"ANSI_RESET" in functions, run\n"
                       ANSI_FG_BLUE"   print_global_lowercase_vars\n"ANSI_RESET
                       "At the end of your script.\n"
                       ,v.ss_l);
  report=true;
  return res;
}
int CONCAT(cg_lowercase_global_variables,_builtin_load)(char *s){
  //   print_global_lowercase_vars(false);
  return true;
}
static int common(const char *cmd,const int id, WORD_LIST *list){
  if (id==I(cg_lowercase_global_variables)) return  print_global_lowercase_vars(false);
  bool retvar=false;
  reset_internal_getopt();
  for (int opt; (opt=internal_getopt(list,"h$"))!=-1;){
    switch(opt){
    case '$': retvar=true;break;
    case 'h':
      CASE_HELPOPT;
    default: builtin_usage(); return EX_USAGE;
    }
  }
  list=loptend;
  int resAll=EXECUTION_SUCCESS;
  char s[PATH_MAX+1]; *s=0;
  if (id==I(cg_currentTimeSeconds)){
    sprintf(s,"%ld",time(NULL));
  }else{
    long seconds=0,numresult=0,now=0;
    if (id==I(cg_areFilesUnmodifiedSeconds) || id==I(cg_fileUnmodifiedSeconds)) now=time(NULL);
    if (id==I(cg_areFilesUnmodifiedSeconds)){
      seconds=atol(list->word->word);
      list=list->next;
    }
    for(int iArg=0;list!=NULL;list=list->next,iArg++){
      if (iArg && (id==I(cg_normalizePath) || id==I(cg_fileSizeCommaLastModified))){
        report_error(RED_ERROR"The builtin "ANSI_FG_BLUE"%s"ANSI_RESET" takes only one argument.",cmd);
        EXIT_SHELL_RETURN();
      }
      struct stat st;
      int res=0;
      char *file=list->word->word;
      if (id<SEPARATOR_NEEDS_STAT && (res=stat(file,&st))){ resAll=res;  report_error(RED_WARNING" '%s'",file); perror("");}
      switch(id){
      case I(cg_fileSizeCommaLastModified):  if (res) strcpy(s,"0,0"); else sprintf(s,"%ld,%ld",st.st_size,st.st_mtime);break;
      case I(cg_normalizePath):cg_normalizePath(s,sizeof(s),file);break;
      case I(cg_fileLastModified): if (!res) numresult=MAX(numresult,st.st_mtime); break;
      case I(cg_fileSize): if (!res) numresult+=st.st_size; break;
      case I(cg_fileUnmodifiedSeconds):if (!res) {
        const long t=now-st.st_mtime;
        numresult=!numresult?t: MIN(t,numresult);
      }
        break;
      case I(cg_areFilesUnmodifiedSeconds):
        if (now-st.st_mtime<=seconds){
          strcpy(s,file);
          resAll=1;
          goto output;
        }
        break;
      }
    } /*for*/
    if (id!=I(cg_areFilesUnmodifiedSeconds) && id!=I(cg_normalizePath) && id!=I(cg_fileSizeCommaLastModified)) sprintf(s,"%ld",numresult);
  }/* with args */
 output:
  //fprintf(stderr,"retvar=%d  s=%s   is cg_normalizePath: %d\n",retvar,s,id==I(cg_normalizePath));
  if (retvar){
    //if (variable_context) make_local_variable(VARNAME_RETVAL,0);
    bind_global_variable(VARNAME_RETVAL,s,0);
  }else{
    if (*s) fputs(s,stdout);
  }
  return resAll;
}
#undef I

#define DOC_OPT_STORE "Options:  -$  Store result in the variable '"VARNAME_RETVAL"'"
#define DOC_COMMON  "",DOC_OPT_STORE,"","Exit status:  Exit value of the stat() call"
#define D(id,...) *id##_doc[]={__VA_ARGS__,(char*)NULL}
static char
D(cg_fileLastModified,"Return max last modified time [s].",DOC_COMMON),
  D(cg_fileSize,"Return sum of file sizes  [Bytes].",DOC_COMMON),
  D(cg_fileSizeCommaLastModified,"Return size,lastModi.",DOC_COMMON),
  D(cg_fileUnmodifiedSeconds,"Return minimal seconds the file has not been modified  [Bytes].",DOC_COMMON),
  D(cg_areFilesUnmodifiedSeconds,"Return EXIT_SUCCESS if all given files exist and have been modified before seconds.",DOC_COMMON),
  D(cg_lowercase_global_variables,""),
  D(cg_currentTimeSeconds,"Return time since 1970 in seconds.","",DOC_OPT_STORE),
  D(cg_normalizePath,"Remove double slash and /./. Converts relative in absolute paths.  Some specific conversion for Charite.","",DOC_OPT_STORE);
#undef D
#undef DOC_OPT_STORE
#undef DOC_COMMON
/* ================================================================================ */
#define MY_USAGE(N) (#N" file  or  "#N" -$ file")
#define MY_BASHBUILTIN(N,usage)  static int N##_builtin(WORD_LIST *list){ return common(#N,ID_##N,list);}; struct builtin N##_struct={#N,N##_builtin,BUILTIN_ENABLED,N##_doc,usage,0};
MY_BASHBUILTIN(cg_fileUnmodifiedSeconds,MY_USAGE(cg_fileUnmodifiedSeconds));
MY_BASHBUILTIN(cg_fileSize,MY_USAGE(cg_fileSize));
MY_BASHBUILTIN(cg_fileSizeCommaLastModified,MY_USAGE(cg_fileSizeCommaLastModified));
MY_BASHBUILTIN(cg_fileLastModified,MY_USAGE(cg_fileLastModified));
MY_BASHBUILTIN(cg_areFilesUnmodifiedSeconds,"cg_areFilesUnmodifiedSeconds   seconds  file1 file2 file3");
MY_BASHBUILTIN(cg_currentTimeSeconds,"cg_currentTimeSeconds or cg_currentTimeSeconds -$ ");
MY_BASHBUILTIN(cg_normalizePath,"cg_normalizePath path ");
MY_BASHBUILTIN(cg_lowercase_global_variables,"cg_lowercase_global_variables");
#undef MY_BASHBUILTIN
#undef MY_USAGE
