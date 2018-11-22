
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
 

DEFINE_CU_DATASET(het_cu)
DEFINE_ATTRIBUTE("config","configuration channel",DIR_INPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("channel0","1 channel",DIR_OUTPUT,TYPE_INT32,sizeof(int32_t))
DEFINE_ATTRIBUTE("channel1","2 channel",DIR_OUTPUT,TYPE_INT64,sizeof(int64_t))
DEFINE_ATTRIBUTE("channel2_double","3 channel",DIR_OUTPUT,TYPE_DOUBLE,sizeof(double))
//DEFINE_ATTRIBUTE("channel3_stringa","4 channel",DIR_OUTPUT,TYPE_STRING,128)
END_CU_DATASET;        

main(int argc,char *argv[]) {
  char chaosserver[64];
  char cuname[64];
  int cnt=0,ret=0;
  chaos_crest_handle_t handle;
  uint32_t cu0;
  int32_t idata32=0;
  int64_t idata64=0;
  double fdata=0;
   
  unsigned int address;
  int setvar, setacc, prova_check=0, status,err;
  /* unsigned long dataint,readint,databuf;*/
  unsigned int dataint,readint,databuf;
  unsigned short datashort,readword;
  unsigned char datachar,readchar;
  int ii, number_of_words,size,het_fd, het_fd1,signal_nr, adm;
  *chaosserver=0;
  *cuname=0;
  address = 0;
  datachar = 0;
  datashort = 0;
  dataint = 0;
  number_of_words = 0;
  adm = 0;
  
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
  het_fd = VmeOpenChannel("het", "het");
  VmeSetExceptionHandling(Vme_EXCEPTION_EXIT);
  VmeSetProperty(het_fd, Vme_SET_DTYPE, 0);
  org_ptr =  (unsigned int *)VmeMapAddress(het_fd, 0xffff0000&address, 0x1000000, adm);
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
       idata32++;
        idata64+=2;
        fdata=3.14*(++cnt);
        //positional attribute, 0 is the first output attribute of cu0
        chaos_crest_update(handle,cu0,0,&idata32);
	
        chaos_crest_update(handle,cu0,1,(void*)&idata64);
        chaos_crest_update(handle,cu0,2,&fdata);
	//        sprintf(sdata,"test stringa %d",idata32);
	//        chaos_crest_update(handle,cu0,3,sdata);
        idata32++;
        idata64+=2;
        fdata=3.14*cnt;
        if((ret=chaos_crest_push(handle,cu0))!=0){
	        printf("## error pushing ret:%d\n",ret);
          //return ret;
        }
    }
    chaos_crest_close(handle);
    return (0);
  /* 
     adm = 0x9  : A32 
           0x39 : A24
           0x29 : A16    
  */
 
  //vme_ptri = (unsigned int *) ((long)org_ptr + (address&0xffff));  /* D32 */
  //vme_ptrs = (unsigned short*)((long)org_ptr + (address&0xffff));  /* D16 */
  //vme_ptrc = (unsigned char*) ((unsigned int)org_ptr + ((address)&0xffff));  /* D8 */
 /*
  switch (setvar) {
  case 0:
    switch (setacc) {
    case 1:
      printf("leggo 8 bit\n");
      for(ii=0;ii<number_of_words;ii++){
	Vme_D08READ(org_ptr, vme_ptrc, readchar);
	printf("Address %8x Data %8x \n",address,readchar);
      }
      break;
    case 2:
      printf("leggo 16 bit\n");
      for(ii=0;ii<number_of_words;ii++){
	Vme_D16READ(org_ptr, vme_ptrs, readword);
	printf("Address %8x Data %8x \n",address,readword);
      }
      break;
    case 3:
      printf("leggo 32 bit\n");
      for(ii=0;ii<number_of_words;ii++){
	Vme_D32READ(org_ptr, vme_ptri, readint);
	printf("Address %8x Data[%d] %8x : ",address,ii,readint);
        PrintEvent_1290A(readint);
      }
    }
    break;
  case 1:
    switch (setacc) {
    case 1:
      printf("scrivo 8 bit\n");
      for(ii=0;ii<number_of_words;ii++){
	Vme_D08WRITE(org_ptr, vme_ptrc, datachar);
      }
      break;
    case 2:
      printf("scrivo 16 bit\n");
      for(ii=0;ii<number_of_words;ii++){
	Vme_D16WRITE(org_ptr, vme_ptrs, datashort);
      }
      break;
    case 3:
      printf("scrivo 32 bit\n");
      for(ii=0;ii<number_of_words;ii++){
	Vme_D32WRITE(org_ptr, vme_ptri, dataint);
        vme_ptri += 4;
        address += 4;
        dataint += 1;
      }
      break;
    }
  }
  */
 #ifdef MOTOROLA
  VmeCloseChannel(het_fd);
#endif
return 0;
}

