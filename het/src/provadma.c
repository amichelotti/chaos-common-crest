
/* VME Linux driver test/demo code */
/* 
 * Include files
 */
char Usage[] = "\
Usage: ./provadma.exe [options] address data ... \n\
       -w##    write to address \n\
       -r##    read from address \n\
       -b##    blt type (2 = VME_BLT, 0x4 = VME_MBLT, 8 = VME_2eVME, 0x10 = VME_2eSST, 0x20 = VME_2eSSTB) \n\
       -a##    address space (1 = VME_A16, 2 = VME_A24, 3 = VME_A32 4 = VME_A64\n\
       -z##    size in byte \n\
Example: ./provadma.exe '-w30000000' '-b2' '-a1' '-z100000' \n\
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
#include <vmedrv.h>

#define DCBF asm("dcbf 0,3")
#define SYNC asm("sync")

void CloseDma(int);
//void flushLine(void*);

vmeDmaPacket_t vmeDma;
vmeInfoCfg_t myVmeInfo;

int getMyInfo(){
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

#define	DMABUFFERSIZE	0x2000000
int	*dmadstbuffer;
int	*dmasrcbuffer;
int     *testbuf;

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
  int fd;
  int ii;

  if(argc < 4){
    fprintf(stderr,Usage);
    _exit(-1);
  }

  argv++; argc--;
  while (argc>0&&argv[0][0] =='-'){
    switch (argv[0][1]){
    case 'r':
      address = strtoul(&argv[0][2], (char**)NULL, 16);
      printf("Opzione r : address = %08x \n",address);
      setvar = 0;
      break;
    case 'w':
      address = strtoul (&argv[0][2], (char**)NULL, 16);
      printf("Opzione w : address = %08x \n",address);
      setvar = 1;
      break;
    case 'b':
      blttype = strtoul (&argv[0][2], (char**)NULL, 16);
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

  if (addressspace == 1) {
    addrs = VME_A16;
  }else if (addressspace == 2) {
    addrs = VME_A24;
  }else if (addressspace == 3) {
    addrs = VME_A32;
  }else if (addressspace == 4) {
    addrs = VME_A64;
  }else{
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
  
  dmadstbuffer = malloc(sizeinbyte);
  dmasrcbuffer = malloc(sizeinbyte);
  testbuf = malloc(sizeinbyte);
  if ( dmadstbuffer == NULL || dmasrcbuffer == NULL || testbuf == NULL ) {
    printf("malloc problem\n");
    _exit(-1);
  }
  
  for(ii=0;ii<sizeinbyte/4;ii++){
    dmadstbuffer[ii]=-99;
  }
  for(ii=0;ii<sizeinbyte/4;ii++){
    dmasrcbuffer[ii]=(ii<<8);
    if(ii%2)dmasrcbuffer[ii]=0xbeef123;
  }
  
  if (getMyInfo()) {
    printf("%s: getMyInfo failed.  Errno = %d\n", argv[0], errno);
    _exit(1);
  }

  if (myVmeInfo.vmeControllerID != 0x014810e3) {
    maxchannel = 1;
  } else {
    maxchannel = 2;
  }

  fd = InitDma(VME_A32,blttype,VME_A32,blttype);
  if(setvar==1) {
    for(ii=0;ii<=sizeinbyte/32;ii++){
      flushLine(&dmasrcbuffer[ii*8]);
    }
    SYNC;
    DoDma(fd, sizeinbyte, dmasrcbuffer, address, VME_DMA_USER, VME_DMA_VME);
  }

  if(setvar==0) {
    for(ii=0;ii<=sizeinbyte/32;ii++){
      flushLine(&dmadstbuffer[ii*8]);
    }
    SYNC;
    DoDma(fd, sizeinbyte, address, dmadstbuffer, VME_DMA_VME, VME_DMA_USER);
    for(ii=0;ii<sizeinbyte/4;ii++){
      if(dmasrcbuffer[ii] != dmadstbuffer[ii]){
	printf("ii = %d,src = %x dest = %x\n",ii,dmasrcbuffer[ii],dmadstbuffer[ii]);
      }
    }
  }

  CloseDma(fd); 
  return(0);
}
/*
int InitDma(addressMode_t srcmode, int srcprotocol, addressMode_t dstmode, int dstprotocol) {
  int fd;
  int status;
  char devnode[20];
  int channel;

  channel = 0;
  sprintf(devnode, "/dev/vme_dma%d", channel);
  fd = open(devnode, 0);
  if (fd < 0) {
    printf("%s: Open failed.  Errno = %d\n", devnode, errno);
    _exit(1);  
  }

  memset(&vmeDma, 0, sizeof(vmeDma));
  vmeDma.maxPciBlockSize = 4096;
  vmeDma.maxVmeBlockSize = 4096;
  vmeDma.srcVmeAttr.maxDataWidth = VME_D32;
  vmeDma.srcVmeAttr.userAccessType = VME_SUPER;
  vmeDma.srcVmeAttr.dataAccessType = VME_DATA;
  vmeDma.srcVmeAttr.xferProtocol = srcprotocol;
  vmeDma.dstVmeAttr.maxDataWidth = VME_D32;
  vmeDma.dstVmeAttr.userAccessType = VME_SUPER;
  vmeDma.dstVmeAttr.dataAccessType = VME_DATA;
  vmeDma.srcVmeAttr.addrSpace = srcmode;
  vmeDma.dstVmeAttr.addrSpace = dstmode;
  vmeDma.dstVmeAttr.xferProtocol = dstprotocol;
  return (fd);
}

int DoDma(int fd, int bytecount, unsigned int srcaddress, unsigned int dstaddress,
           dmaData_t srcbus, dmaData_t dstbus){
  
  int status;
  vmeDma.byteCount = bytecount;
  vmeDma.srcBus = srcbus;
  vmeDma.dstBus = dstbus;
  vmeDma.srcAddr = srcaddress;
  vmeDma.dstAddr = dstaddress;
  status = ioctl(fd, VME_IOCTL_START_DMA, &vmeDma);
  if (status < 0) {
    printf("VME_IOCTL_START_DMA failed.  Errno = %d\n", errno);
    _exit(1);
  }
}
*/
void CloseDma(int fd){
  close(fd);
}
/*
void flushLine(void *ramptr){
  DCBF;
}
*/
