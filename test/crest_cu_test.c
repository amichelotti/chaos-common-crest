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
printf("%s <wan proxy url> [iterations]\n",argv[0]);\
exit(1)

DEFINE_CU_DATASET(test_cu)
DEFINE_ATTRIBUTE("config","configuration channel",DIR_INPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("channel0","1 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("channel1","2 channel",DIR_OUTPUT,TYPE_INT64,sizeof(int64_t))
DEFINE_ATTRIBUTE("channel2_double","3 channel",DIR_OUTPUT,TYPE_DOUBLE,sizeof(double))
//DEFINE_ATTRIBUTE("channel3_stringa","4 channel",DIR_OUTPUT,TYPE_STRING,128)
END_CU_DATASET;        

DEFINE_CU_DATASET(test_cu2)
DEFINE_ATTRIBUTE("config","configuration channel",DIR_INPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("channel00","first channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("channel10","2 channel",DIR_OUTPUT,TYPE_INT64,sizeof(int64_t))
DEFINE_ATTRIBUTE("channel20_double","second channel",DIR_OUTPUT,TYPE_DOUBLE,sizeof(double))
//DEFINE_ATTRIBUTE("channel30_stringa","thierd channel",DIR_OUTPUT,TYPE_STRING,128)
END_CU_DATASET;   
     
        
int main(int argc, char** argv) {
    char *url=0;
    int iterations=10;
    chaos_crest_handle_t handle;
    uint32_t cu0,cu1;
    // data variables
    int32_t idata32=0;
    int64_t idata64=0;
    double fdata=0;
    //    char sdata[128];
    int ret,cnt;
    if(argc<2){
        USAGE;
    }
    url = argv[1];
    printf("* opening %s\n",url);
    handle=chaos_crest_open(url);
    if(handle==NULL){
        printf("## cannot open crest interface for url:%s\n",url);
        exit(1);
    }
    if(argc==3){
        iterations=atoi(argv[2]);
    }
    
    cu0=ADD_CU(handle,test_cu);
    if(cu0==0){
        printf("## failed adding cu \"test_cu\"\n");
        exit(1);
    }
    cu1=ADD_CU(handle,test_cu2);
    if(cu1==0){
        printf("## failed adding cu \"test_cu2\"\n");
        exit(1);
    }
    printf("* connecting to %s...\n",url);
    if((ret=chaos_crest_connect(handle))!=0){
      printf("## cannot connect to %s, error:%d\n",url,ret);
        exit(1);
    }
    printf("* registering to %s...\n",url);
    // 0 means all defined CU
    if((ret=chaos_crest_register(handle,0))!=0){
      printf("## cannot register CUs, error:%d\n",ret);
        exit(1);
    }
    printf("* registration average  %f ms...\n",chaos_crest_reg_time(handle));
    printf("* pushing to %s...\n",url);
    for(cnt=0;cnt<iterations;cnt++){
        idata32++;
        idata64+=2;
        fdata=3.14*iterations;
        //positional attribute, 0 is the first output attribute of cu0
        chaos_crest_update(handle,cu0,0,&idata32);
	
        chaos_crest_update(handle,cu0,1,(void*)&idata64);
        chaos_crest_update(handle,cu0,2,&fdata);
	//        sprintf(sdata,"test stringa %d",idata32);
	//        chaos_crest_update(handle,cu0,3,sdata);
        idata32++;
        idata64+=2;
        fdata=3.14*iterations;
        chaos_crest_update(handle,cu1,0,&idata32);
        chaos_crest_update(handle,cu1,1,&idata64);
        chaos_crest_update(handle,cu1,2,&fdata);
	//        sprintf(sdata,"test2 stringa %d",idata32);
	//        chaos_crest_update(handle,cu1,3,sdata);

	if((ret=chaos_crest_push(handle,0))!=0){
	  printf("## error pushing ret:%d\n",ret);
	  exit(1);
	}

    }
    printf("* %d iteration average %f ms per iteration...\n",iterations,chaos_crest_push_time(handle));
    return (EXIT_SUCCESS);
}

