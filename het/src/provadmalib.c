
/* VME Linux driver test/demo code */
/* 
 * Include files
 */
char Usage[] = "\
Usage: ./provadmalib.exe [options] address data ... \n\
       -w##    write to address \n\
       -r##    read from address \n\
       -b##    blt type (2 = VME_BLT, 0x4 = VME_MBLT, 8 = VME_2eVME, 0x10 = VME_2eSST, 0x20 = VME_2eSSTB) \n\
       -a##    address space (1 = VME_A16, 2 = VME_A24, 3 = VME_A32 4 = VME_A64\n\
       -z##    size in byte \n\
Example: ./provadmalib.exe '-w30000000' '-b2' '-a1' '-z100000' \n\
";

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <unistd.h> 
#include <stdio.h> 
#include <string.h> 
#include <errno.h> 
#include <sys/ioctl.h> 
#include <malloc.h>
#include "vmedrv.h"
#include "Vme.h"

#define MIN(x,y) ((x)>(y)?(y):(x))
#define SYNC asm("sync")
#define DCBF asm("dcbf 0,3")
#define DCBZ asm("dcbz 0,3")

static char *nomedma[] = {
  "/dev/vme_dma0",
  "/dev/vme_dma1"
};

vmeInfoCfg_t myVmeInfo;

#define	DMABUFFERSIZE	0x2000000
int *genpatt;
int *testbuf;
#define LINESIZE 0x20

int getMyInfo() {
  int	fd, status;
  fd = open("/dev/vme_ctl", 0);
  if (fd < 0) {
    return(1);
  }
  
  memset(&myVmeInfo, 0, sizeof(myVmeInfo));
  status = ioctl(fd, VME_IOCTL_GET_SLOT_VME_INFO, &myVmeInfo);
  if (status < 0) {
    return(1);
  }
  close(fd);
  return(0);
}


#define	NUMDMATYPES	3

dmaData_t dmasrctypes[NUMDMATYPES] = {
  VME_DMA_USER,
  VME_DMA_VME, 
  VME_DMA_VME
};

dmaData_t dmadsttypes[NUMDMATYPES] = {
  VME_DMA_VME,
  VME_DMA_USER,
  VME_DMA_VME
};

int main(int argc, char *argv[]){
  int channel;
  int maxchannel;
  int address;
  int setvar;
  int blttype;
  int addressspace;
  int sizeinbyte;
  int addrs;
  int btype;
  int ii;
  int jj;
  int fd;
  int cid_dma;
  int cid_pio;
  unsigned int *org_dma, *org_ptr, *vme_ptr;

  if(argc < 4){
    fprintf(stderr,Usage);
    _exit(-1);
  }

  argv++; argc--;
  while (argc>0&&argv[0][0] =='-') {
    switch (argv[0][1]){
    case 'r':
      address = strtoul(&argv[0][2], (char**)NULL, 16);
      printf("Opzione r : address = %08x \n",address);
      setvar = 0;
      break;
    case 'w':
      address = strtoul(&argv[0][2], (char**)NULL, 16);
      printf("Opzione w : address = %08x \n",address);
      setvar = 1;
      break;
    case 'b':
      blttype = strtoul(&argv[0][2], (char**)NULL, 16);
      printf("Opzione b : address = %08x \n",blttype);
      break;
    case 'a':
      addressspace = strtoul(&argv[0][2], (char**)NULL, 16);
      printf("Opzione a : addrressspace = %08x \n",addressspace);
      break;
    case 'z':
      sizeinbyte = strtoul(&argv[0][2], (char**)NULL, 16);
      printf("Opzione z : sizeinbyte = %08x \n",sizeinbyte);
      break;
    }
    argv++;argc--;
  }

  printf("address = %x \n",address);
  printf("blttype= %x \n",blttype);
  printf("addressspace = %x \n",addressspace);
  printf("sizeinbyte = %x \n",sizeinbyte);

  if(addressspace == 1) {
    addrs = VME_A16;
  } else if(addressspace == 2) {
    addrs = VME_A24;
  } else if(addressspace == 3) {
    addrs = VME_A32;
  } else if(addressspace == 4) {
    addrs = VME_A64;
  } else {
    printf("not valid address space\n");
    _exit(-1);
  }
        
  if(blttype == 0x2) {
    btype = VME_BLT;
  } else if(blttype == 0x4) {
    btype = VME_MBLT;
  } else if(blttype == 0x8) {
    btype = VME_2eVME;
  } else if(blttype == 0x10) {
    btype = VME_2eSST;
  } else if(blttype == 0x20) { 
    btype = VME_2eSSTB;
  } else {
    printf("not valid blttype \n");
    _exit(-1);
  } 

  genpatt = (int *)malloc(sizeinbyte);
  testbuf = (int *)malloc(sizeinbyte);
  if ( genpatt == NULL || testbuf == NULL ) {
    printf("malloc problem\n");
    _exit(-1);
  }

  printf("testbuf = %x\n",testbuf);
  printf("genpatt = %x\n",genpatt);

  if (getMyInfo()) {
    printf("%s: getMyInfo failed.  Errno = %d\n", argv[0], errno);
    _exit(1);
  }
  
  if (myVmeInfo.vmeControllerID != 0x014810e3) {
    maxchannel = 1;
  } else {
    maxchannel = 2;
  }

  cid_dma = VmeOpenChannel("prova","dma");
  cid_pio = VmeOpenChannel("prova","pio");
  VmeSetExceptionHandling(Vme_EXCEPTION_EXIT);
  VmeSetProperty(cid_pio, Vme_SET_DTYPE, 0);
  VmeSetProperty(cid_dma, 10, 8);
  org_dma = (unsigned int*)VmeMapAddress(cid_dma, 0xFFFF0000&address, sizeinbyte, 0x9);
  org_ptr = (unsigned int*)VmeMapAddress(cid_pio, 0xFFFF0000&address, 0x1000000, 0x9);
  vme_ptr = (unsigned int*)((long)org_ptr + (address&0xFFFF));

  for (ii=0;ii<sizeinbyte/4;ii++) {
    testbuf[ii] = -99; 
    genpatt[ii] = ii;
    if(ii%2)genpatt[ii] = 0xdeadbeef;
  }
  for (ii=0;ii<sizeinbyte/4;ii++) {
    flushLine2(&testbuf[ii]);
    flushLine2(&genpatt[ii]);
  }
  for (ii=0;ii<sizeinbyte/4;ii++) {
    cacheinv2(&testbuf[ii]);
    cacheinv2(&genpatt[ii]);
  }

  if (setvar == 1) {
    VmeDmaWrite(cid_dma, 0x0, (char *)genpatt, sizeinbyte);
  }

  if (setvar == 0) {
    VmeDmaRead(cid_dma, 0x0, (char *)testbuf, sizeinbyte);
    for(ii=0;ii<sizeinbyte/4;ii++){
      if(genpatt[ii] != testbuf[ii]){
	printf("i= %d genpatt = %x dato= % x\n",ii,genpatt[ii],testbuf[ii]);
      }
    }
  }
  
  (void)VmeCloseChannel(cid_pio);       
  (void)VmeCloseChannel(cid_dma);       
  return(0);
}

flushLine2(void *ramptr){
  DCBF;
  SYNC;
}

cacheinv2(void *ramptr){
  DCBZ;
  SYNC;
}

