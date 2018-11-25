
/* vmedisp.c, 18/4/96 Paolo Branchini */
char Usage[] = "\
Usage: %s \
       -s <chaos server> \n\
       -n <CU name> \n\
       [-a <hetbaseaddress>] \n\
       ";
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#ifdef MOTOROLA
#include "Vme.h"
#include "Data.h" 

#include <vmedrv.h>
#endif
#include <sys/mman.h>

#include "chaos_crest.h"


static  unsigned int *vme_ptri,*org_ptr;
static  unsigned int *vme_ptr1,*org_ptr1;
static  unsigned int pno1,pno2;
static  unsigned short *vme_ptrs,*vme_ptrs1;
static  unsigned char *vme_ptrc,*vme_ptrc1;
void MyExceptionHandler(int);
 //(TYPE_VECTOR|TYPE_INT32)
#define FIFO_SIZE 1024
DEFINE_CU_DATASET(het_cu)
DEFINE_ATTRIBUTE("SR6","SR6 REGISTER",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("FIFO","HET Fifo",DIR_OUTPUT,(TYPE_BINARY),FIFO_SIZE*sizeof(int32_t))
//DEFINE_ATTRIBUTE("channel3_stringa","4 channel",DIR_OUTPUT,TYPE_STRING,128)
END_CU_DATASET;        

main(int argc,char *argv[]) {
  char chaosserver[64];
  char cuname[64];
  int cnt=0,ret=0;
  chaos_crest_handle_t handle;
  uint32_t cu0;
  int32_t fifo[FIFO_SIZE];

   
  unsigned int address;
  int r6=0;
  /* unsigned long dataint,readint,databuf;*/
  *chaosserver=0;
  *cuname=0;
  address = 0x01000000;
  
  while (cnt<argc) {
    if((!strcmp(argv[cnt],"-s"))&&((cnt+1)<argc)){
      strncpy(chaosserver,argv[++cnt],sizeof(chaosserver));
      continue;
    }
    if((!strcmp(argv[cnt],"-n"))&&((cnt+1)<argc)){
      strncpy(cuname,argv[++cnt],sizeof(cuname));
      continue;
    }
    if((!strcmp(argv[cnt],"-a"))&&((cnt+1)<argc)){
      address=strtoul(argv[++cnt]);
      continue;
    }
    if((!strcmp(argv[cnt],"-h"))){
      fprintf(stdout,Usage,argv[0]);
      return 0;
    }
	cnt++;
  }
   
   if(*chaosserver==0){
         printf("## you must provide a valid metadaserver address\n");
      return -2;
   }
   if(*cuname==0){
         printf("## you must provide a valid cuname\n");
      return -3;
   }
  
  printf("chaosserver = %s \n",chaosserver);
  printf("cuname = %s \n",cuname);
  printf("HET VME ADDRESS = %08x \n",address);
 #ifdef MOTOROLA
   VmeInit(&het_fd);
   SetDefaults();
  data.basaddr = 0x01000000; 
  data.am = VME_A32;            
  data.dtsize = VME_D32; 
  /*
  het_fd = VmeOpenChannel("het", "het");
  VmeSetExceptionHandling(Vme_EXCEPTION_EXIT);
  VmeSetProperty(het_fd, Vme_SET_DTYPE, 0);
  org_ptr =  (unsigned int *)VmeMapAddress(het_fd, 0xffff0000&address, 0x1000000, adm);
  */
 #endif
 
  handle=chaos_crest_open(chaosserver);
  if(handle==NULL){
        printf("## cannot open crest interface for url:%s\n",chaosserver);
        return -3;
    }
    cu0=ADD_CU(handle,cuname,het_cu);
    if(cu0==0){
        printf("## failed adding cu \"%s\"\n",cuname);
        return -4;
    }
    printf("* registering to %s...\n",chaosserver);
    // 0 means all defined CU
    if((ret=chaos_crest_register(handle,cu0))!=0){
      printf("## cannot register CUs, error:%d\n",ret);
      return -9;
    }
    printf("* registration average  %f ms...\n",chaos_crest_reg_time(handle));
    printf("* pushing to %s...\n",chaosserver);
    cnt=0;

    while(1){
      #ifdef MOTOROLA
        data.addr = data.basaddr + 0x00f80000 + 6*4;
        VmeRead(het_fd);
        r6=data.datum;
      #else
        r6++;
      #endif
      chaos_crest_update(handle,cu0,0,&r6);

      for(cnt=0;cnt<FIFO_SIZE;cnt++){
      #ifdef MOTOROLA
        data.addr = data.basaddr + 0x00000000;
        data.datum = 0x0;
        VmeRead(het_fd);
        fifo[cnt]=data.datum;
      #else
        fifo[cnt]=cnt;

      #endif
      }
      
      chaos_crest_update(handle,cu0,1,fifo);

        if((ret=chaos_crest_push(handle,cu0))!=0){
	        printf("## error pushing ret:%d\n",ret);
          return ret;
        }
      
    }
    chaos_crest_close(handle);
    return (0);
  
 #ifdef MOTOROLA
 VmeEnd(het_fd);
#endif
return 0;
}

