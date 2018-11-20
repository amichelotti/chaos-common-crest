/* vmedisp.c, 18/4/96 Paolo Branchini */
char Usage[] = "\
Usage: ./provalib.exe [options] address data ... \n\
       -w##    write to address \n\
       -r##    read from address \n\
       -b##    byte access type \n\
       -s##    shortword access type \n\
       -l##    longword access type \n\
       -n##    number of words to read/write \n\
Example: ./provalib.exe '-w30000000' '-l12345678' '-z9' '-n40' \n\
";
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <vmedrv.h>
#include <sys/mman.h>
#include "Vme.h"

static  unsigned int *vme_ptri,*org_ptr;
static  unsigned int *vme_ptr1,*org_ptr1;
static  unsigned int pno1,pno2;
static  unsigned short *vme_ptrs,*vme_ptrs1;
static  unsigned char *vme_ptrc,*vme_ptrc1;
void MyExceptionHandler(int);
 
main(int argc,char *argv[]) {
  unsigned int address,address1;
  int setvar, setacc, prova_check=0, status,err;
  /* unsigned long dataint,readint,databuf;*/
  unsigned int dataint,readint,databuf;
  unsigned short datashort,readword;
  unsigned char datachar,readchar;
  int ii, number_of_words,size,cid_pio, cid_pio1,signal_nr, adm;
  
  address = 0;
  datachar = 0;
  datashort = 0;
  dataint = 0;
  number_of_words = 0;
  adm = 0;
  
  if (argc < 3) {
    fprintf(stderr,Usage);
    exit(-1);
  }
  
  argv++; argc--;
  while (argc>0&&argv[0][0] =='-') {
    switch (argv[0][1]){
    case 'r':
      address = strtoul(&argv[0][2], (char**)NULL, 16);
      printf("opzione r : address = %08x \n",address);
      setvar = 0;
      break;
    case 'w':
      address = strtoul(&argv[0][2], (char**)NULL, 16);
      printf("opzione w : address = %08x \n",address);
      setvar = 1;
      break;
    case 'b':
      setacc = 1;
      if(setvar == 1) {
	datachar = (unsigned char)strtoul(&argv[0][2], (char**)NULL, 16);
      }
      printf("opzione b : datachar = %02x \n",datachar);
      break;
    case 's':
      setacc = 2;
      if(setvar == 1) {
	datashort = (unsigned short)strtoul(&argv[0][2], (char**)NULL, 16);
      }
      printf("opzione s : datashort = %04x \n",datashort);
      break;
    case 'l':
      setacc = 3;
      if(setvar == 1) {
	dataint = strtoul(&argv[0][2],(char**) NULL,16);
      }
      printf("opzione l : dataint = %08x \n",dataint);
      break;
    case 'n':
      number_of_words = strtoul (&argv[0][2],(char**) NULL,16);
      printf("opzione n : number_of_words = %d \n",number_of_words);
      break;
    case 'z':
      adm =             strtoul (&argv[0][2],(char**) NULL,16);
      printf("opzione z : adc = %08x \n",adm);
      break;
    }
    argv++;argc--;
  }
  
  printf("address = %08x \n",address);
  printf("datashort = %04x \n",datashort);
  printf("datachar = %02x \n",datachar);
  printf("dataint = %08x \n",dataint);
  printf("number_of_words = %08x \n",number_of_words);
  printf("adm = %08x \n",adm); 
  /* 
     adm = 0x9  : A32 
           0x39 : A24
           0x29 : A16    
  */
  cid_pio = VmeOpenChannel("prova", "pio");
  VmeSetExceptionHandling(Vme_EXCEPTION_EXIT);
  VmeSetProperty(cid_pio, Vme_SET_DTYPE, 0);
  org_ptr =  (unsigned int *)VmeMapAddress(cid_pio, 0xffff0000&address, 0x1000000, adm);
  vme_ptri = (unsigned int *) ((long)org_ptr + (address&0xffff));  /* D32 */
  vme_ptrs = (unsigned short*)((long)org_ptr + (address&0xffff));  /* D16 */
  vme_ptrc = (unsigned char*) ((unsigned int)org_ptr + ((address)&0xffff));  /* D8 */
 
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
  
  VmeCloseChannel(cid_pio);
  exit(0);
}

