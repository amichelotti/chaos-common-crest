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

DEFINE_CU_DATASET(qdc0)
DEFINE_ATTRIBUTE("hi0","965 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("hi1","965 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("hi2","965 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("hi3","965 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("hi4","965 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("hi5","965 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("hi6","965 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("hi7","965 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("hi8","965 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("hi9","965 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("hi10","965 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("hi11","965 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("hi12","965 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("hi13","965 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("hi14","965 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("hi15","965 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("lo0","965 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("lo1","965 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("lo2","965 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("lo3","965 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("lo4","965 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("lo5","965 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("lo6","965 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("lo7","965 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("lo8","965 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("lo9","965 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("lo10","965 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("lo11","965 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("lo12","965 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("lo13","965 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("lo14","965 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("lo15","965 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
END_CU_DATASET;       

DEFINE_CU_DATASET(qdc1)
DEFINE_ATTRIBUTE("ch0","792 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("ch1","792 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("ch2","792 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("ch3","792 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("ch4","792 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("ch5","792 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("ch6","792 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("ch7","792 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("ch8","792 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("ch9","792 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("ch10","792 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("ch11","792 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("ch12","792 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("ch13","792 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("ch14","792 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("ch15","792 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
/*DEFINE_ATTRIBUTE("ch16","792 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("ch17","792 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("ch18","792 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("ch19","792 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("ch20","792 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("ch21","792 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("ch22","792 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("ch23","792 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("ch24","792 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("ch25","792 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("ch26","792 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("ch27","792 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("ch28","792 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("ch29","792 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("ch30","792 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("ch31","792 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
*/
END_CU_DATASET;       

DEFINE_CU_DATASET(scaler)
DEFINE_ATTRIBUTE("ch0","scaler channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("ch1","scaler channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
END_CU_DATASET;       
 

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
    uint32_t cu0,cu1,qdc0,qdc1,scaler;
    // data variables
    int32_t idata32=0;
    int64_t idata64=0;
    double fdata=0;
    uint32_t max,min;
    //    char sdata[128];
    int ret,cnt,cntt;
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
    
    cu0=ADD_CU(handle,"TEST/TEST_CU",test_cu);
    if(cu0==0){
        printf("## failed adding cu \"test_cu\"\n");
        exit(1);
    }
    cu1=ADD_CU(handle,"TEST/TEST_CU2",test_cu2);
    if(cu1==0){
        printf("## failed adding cu \"test_cu2\"\n");
        exit(1);
    }
    
    qdc0=ADD_CU(handle,"BTF_SIM/QDC0",qdc0);
    qdc1=ADD_CU(handle,"BTF_SIM/QDC1",qdc1);
    scaler=ADD_CU(handle,"BTF_SIM/SCALER",scaler);

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
	for(cntt=0;cntt<32;cntt++){
	  chaos_crest_update(handle,qdc0,cntt,&cntt);
	}
	for(cntt=0;cntt<16;cntt++){
	  chaos_crest_update(handle,qdc1,cntt,&cntt);
	}
	chaos_crest_update(handle,scaler,0,&idata32);
	chaos_crest_update(handle,scaler,1,&idata32);

	if((ret=chaos_crest_push(handle,0))!=0){
	  printf("## error pushing ret:%d\n",ret);
	  exit(1);
	}

    }
    {
      float t=chaos_crest_push_time(handle,&max,&min);
      printf("* %d iteration average %f ms per iteration (max:%u ms, min:%u ms) ...\n",iterations,t,max,min);
    }
    chaos_crest_close(handle);
    return (EXIT_SUCCESS);
}

