/* 
 * File:   crest_cu_test.c
 * Author: michelo
 *
 * Created on February 27, 2015, 9:49 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include "chaos_crest.h"
/*
 * 
 */
#define USAGE   \
printf("%s <ui url> <CUNAME> [-d channel] [-c <command>  <args>]\n",argv[0]);\
exit(1)

        
int main(int argc, char** argv) {
    char *url=0;
    chaos_crest_handle_t handle;
    uint32_t cu0,cu1;
    uint64_t tm;
    int ret= 0;
    // data variables
    int cnt;
    char *cuname;
    char*channame=0;
    char*command=0;
    char*cmd_args=0;
    char buffer[4096];
    if(argc<3){
        USAGE;
    }
    url = argv[1];
    
    handle=chaos_crest_open(url);
    if(handle==NULL){
        printf("## cannot open crest interface for url:%s\n",url);
        exit(1);
    }
    cuname = argv[2];
    
    if(argc==4){
      channame=argv[3];
    }
    for(cnt=0;cnt<argc;cnt++){
      if(!strcmp("-d",argv[cnt])){
	channame = argv[cnt+1];
	continue;
      }
      if(!strcmp("-c",argv[cnt])){
	command = argv[cnt+1];
	cmd_args = argv[cnt+2];
	continue;
      }
    }
    chaos_crest_connect(handle);
    printf("* opening %s CU:%s\n",url,cuname);
    if(channame){
      printf("* getting %s/%s\n",cuname,channame);
      tm =chaos_crest_cu_get_channel(handle,cuname,channame,buffer,sizeof(buffer));
    } else {
      tm =chaos_crest_cu_get(handle,cuname,buffer,sizeof(buffer));
    }

    if(command){
      if(chaos_crest_cu_cmd(handle,cuname,command,cmd_args)==0){
	printf("* command ok");
      }
    }
    if(tm>0){
      printf("%s returned at %llu:\n\"%s\"\n",cuname,tm,buffer);
      
    } else {
      printf("## error getting %s\n",cuname);
      ret=-1;
    }

    //    chaos_crest_close(handle);
    return ret;
}

