/**
 * Acquisition through !CHAOS
 * Andrea Michelotti November 2018
 */
char Usage[] =
    "\
usage: %s \
       -s <chaos server> \n\
       -n <CU name> \n\
       [-a <hetbaseaddress>] \n\
       [-c <cycles to store 9=zero suppression>][%d] \n\
       [-of <offset fiducial>][%d] \n\
       [-td <trigger delay (10ns units)>][%d] \n\
       [-t] (test mode trigger autogenerated) [%d]\n\
       ";
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

#include <unistd.h>
#define VME_MAP_ADDRESS 0x01000000
#define VME_MAP_SIZE 1024 * 1024 * 32
#define VME_MAP_AM 0x9
//#define DMABUFFERSIZE 1024 * 60 //0x2000000
#define FULLFIFOSIZE (256) * 1024 //256K 0x40000
#define HOWTOREAD 3               // 3=1/4 2=1/2 1=3/4 0 tutta.

#define DMABUFFERSIZE (FULLFIFOSIZE * (4 - HOWTOREAD) / 4)
#include "hetDriver.h"
#include <stdlib.h>
#ifdef MOTOROLA

//#include "vmelib.h"
#define R6_OFF (0x00f80000 + 6 * 4)
#include <Vme.h>

//#include "Data.h"
#include <stdio.h>
#include <time.h>
#else
#define Vme_D32WRITE(x, y, z)
#define Vme_D32READ(x, y, z)
#define VmeDmaRead(ciddma, offset, evtbuf, sizedma) 0
#define VmeCloseChannel(ciddma) ;
#define PollingVme(s, r) 3
#endif

#include <sys/mman.h>

#include "chaos_crest.h"

static unsigned int *vme_ptri, *org_ptr;
static unsigned int *vme_ptr1, *org_ptr1;
static unsigned int pno1, pno2;
static unsigned short *vme_ptrs, *vme_ptrs1;
static unsigned char *vme_ptrc, *vme_ptrc1;
void MyExceptionHandler(int);
static unsigned endian_swap(unsigned int x)
{
  return (x >> 24) | ((x >> 8) & 0x0000ff00) | ((x << 8) & 0x00ff0000) |
         (x << 24);
}
//(CREST_TYPE_VECTOR|CREST_TYPE_INT32)
//#define FIFO_SIZE 1024
DEFINE_CU_DATASET(het_cu)
DEFINE_ATTRIBUTE("STATUS", "STATUS REGISTER", CREST_DIR_OUTPUT, CREST_TYPE_INT32, sizeof(int32_t))
DEFINE_ATTRIBUTE("EVENTS", "Events Found", CREST_DIR_OUTPUT, CREST_TYPE_INT64, sizeof(int64_t))

DEFINE_ATTRIBUTE("Config", "Config register0", CREST_DIR_OUTPUT, (CREST_TYPE_INT32 | CREST_TYPE_VECTOR), 7)

// DEFINE_ATTRIBUTE("SR7","SR7 REGISTER",CREST_DIR_OUTPUT,CREST_TYPE_INT32,sizeof(int32_t))
// DEFINE_ATTRIBUTE("SR_STRING","SR7 REGISTER",CREST_DIR_OUTPUT,CREST_TYPE_STRING,10)

DEFINE_ATTRIBUTE("FIFO", "HET Fifo", CREST_DIR_OUTPUT, (CREST_TYPE_INT32 | CREST_TYPE_VECTOR),
                 DMABUFFERSIZE / 4)
// DEFINE_ATTRIBUTE("channel3_stringa","4 channel",CREST_DIR_OUTPUT,CREST_TYPE_STRING,128)
END_CU_DATASET;

static int open_file_data(FILE **fpdata, FILE **fplog)
{
  char file_name[200];
  char data[100];
  FILE *fp1;
  int ret = 0;

  system("rm -f lista");
  system("date | awk '{print $1$3$2$6\"_\"$4}' | sed 's/://g' > lista");
  fp1 = fopen("lista", "r");
  fscanf(fp1, "%s", &data[0]);
  fclose(fp1);

  strcpy(file_name, "./data/run_");
  strcat(file_name, data);
  strcat(file_name, ".dat");
  printf("file name = %s \n", file_name);
  *fpdata = fopen(file_name, "w");
  if (*fpdata == NULL)
  {
    printf("Errore apertura file data = %s \n", file_name);
    ret = -1;
  }

  strcpy(file_name, "./data/run_");
  strcat(file_name, data);
  strcat(file_name, ".log");
  printf("file name = %s \n", file_name);
  *fplog = fopen(file_name, "w");
  if (*fplog == NULL)
  {
    printf("Errore apertura file log = %s\n", file_name);
    ret = -1;
  }

  system("rm -f lista");

  return (ret);
}
enum
{
  SEARCH_HEADER,
  HEADER1_FOUND,
  HEADER2_FOUND,
  DATA_FOUND,
  FOOTER1_FOUND,
  FOOTER2_FOUND
};
int main(int argc, char *argv[])
{
  unsigned long vme_address;
  unsigned int offset, result, nread;
  int ciddma, cidvme;
  unsigned int *evtbuf, *vme_dma, *vme_base;
  int cycles = 3;
  int testmode = 0;
  int triggerDelay = 0;
  int fiducialOffset = 0;
  unsigned int loop = 0;
  uint32_t* fifo;

  unsigned int *vme_ptr;
  int status, sizedma;
  FILE *fpdata, *fplog;
  int trig_old, trig_work, delta_trig, nn;

  char chaosserver[64];
  char cuname[64];
  int shm_state = SEARCH_HEADER;
  int cnt = 0, ret = 0;
  chaos_crest_handle_t handle;
  int het_fd;
  uint32_t cu0;
  uint32_t readint;
  struct timeval mytime;
  uint64_t eventn = 0;
  unsigned int address, t2 = 0;
  unsigned int r6 = 0;
  fifo=calloc(1,DMABUFFERSIZE);
  /* unsigned long dataint,readint,databuf;*/
  *chaosserver = 0;
  *cuname = 0;
  address = 0x01000000;
  
  while (cnt < argc)
  {
    if ((!strcmp(argv[cnt], "-s")) && ((cnt + 1) < argc))
    {
      strncpy(chaosserver, argv[++cnt], sizeof(chaosserver));
      continue;
    }
    if ((!strcmp(argv[cnt], "-n")) && ((cnt + 1) < argc))
    {
      strncpy(cuname, argv[++cnt], sizeof(cuname));
      continue;
    }
    if ((!strcmp(argv[cnt], "-a")) && ((cnt + 1) < argc))
    {
      address = strtoul(argv[++cnt], 0, 0);
      continue;
    }
    if ((!strcmp(argv[cnt], "-c")) && ((cnt + 1) < argc))
    {
      cycles = strtoul(argv[++cnt], 0, 0);
      continue;
    }
    if ((!strcmp(argv[cnt], "-of")) && ((cnt + 1) < argc))
    {
      fiducialOffset = strtoul(argv[++cnt], 0, 0);
      continue;
    }
    if ((!strcmp(argv[cnt], "-td")) && ((cnt + 1) < argc))
    {
      triggerDelay = strtoul(argv[++cnt], 0, 0);
      continue;
    }
    if ((!strcmp(argv[cnt], "-h")))
    {
      fprintf(stdout, Usage, argv[0]);
      return 0;
    }
    if ((!strcmp(argv[cnt], "-t")))
    {
      testmode = 1;
      printf("* enabling auto-tigger\n");
    }
    cnt++;
  }
  if (cycles > 9 || cycles < 0)
  {
    printf("## invalid cycles %d (1:9)\n", cycles);
    return -1;
  }
  if (*chaosserver == 0)
  {
    printf("## you must provide a valid metadaserver address\n");
    return -2;
  }
  if (*cuname == 0)
  {
    printf("## you must provide a valid cuname\n");
    return -3;
  }
  ret = open_file_data(&fpdata, &fplog);
  if (ret != 0)
  {
    printf("Apertura file data NOT OK \n");
    fflush(stdout);
    exit(-1);
  }
  gettimeofday(&mytime, NULL);
  printf("chaosserver = %s %llu ms \n", chaosserver, (unsigned long long)mytime.tv_sec * 1000 + ((unsigned long long)mytime.tv_usec / 1000));
  printf("cuname = %s \n", cuname);
  printf("HET VME ADDRESS = %08x \n", address);
#ifdef MOTOROLA
#warning "HW ACCESS ENABLED"

  /*
   * Init Vme
   */
  vme_address = (unsigned long)VME_MAP_ADDRESS;
  ciddma = VmeOpenChannel("data", "dma");
  cidvme = VmeOpenChannel("data", "pio");
  printf("ciddma is now %d\n", ciddma);
  printf("cidvme is now %d\n", cidvme);
  VmeSetExceptionHandling(Vme_EXCEPTION_EXIT);
  VmeSetProperty(cidvme, Vme_SET_DTYPE, 0);
  VmeSetProperty(ciddma, 10, 8);
  vme_dma = (unsigned int *)VmeMapAddress(ciddma, vme_address, VME_MAP_SIZE, VME_MAP_AM);
  vme_base = (unsigned int *)VmeMapAddress(cidvme, vme_address, VME_MAP_SIZE, VME_MAP_AM);
  printf("Fine del map address %x\n", vme_base);

#endif
  evtbuf = (unsigned int *)malloc(DMABUFFERSIZE);
  for (cnt = 0; cnt < DMABUFFERSIZE / 4; cnt++)
  {
    evtbuf[cnt] = 0xdeaddead;
  }
  /* for (cnt = 0; cnt < DMABUFFERSIZE / 4; cnt++)
  {
    flushLine2(&evtbuf[cnt]);
  }
  for (cnt = 0; cnt < DMABUFFERSIZE / 4; cnt++)
  {
    cacheinv2(&evtbuf[cnt]);
  }
*/
  ResetMemoria(vme_base, fplog);
  StatoCtrlReg(vme_base, 0, fplog);
  if (testmode)
  {
    InitTDCV5(vme_base, fplog);
  }
  else
  {
    printf("* reset ALL\n");
    //enable reset
    //  WRITE32(vme_base, HET_REG_OFF(1), CTRL_RESET_ENABLE|GBL_RESET_ENABLE|MEM_RESET_ENABLE|FSM_RESET_ENABLE|PLL_RESET_ENABLE|ENABLE_T1 | ENABLE_T2 | FIFO_NORMAL | FIFO_TRG_UKN);
    // do reset
    WRITE32(vme_base, HET_REG_OFF(4), 0xFFFFFFFF);
    sleep(1);
    WRITE32(vme_base, HET_REG_OFF(4), 0);

    // enable all channels
    WRITE32(vme_base, HET_REG_OFF(0), 0xFFFFFFFF);
    WRITE32(vme_base, HET_REG_OFF(1), ENABLE_T1 | ENABLE_T2 | FIFO_NORMAL | FIFO_TRG_UKN);
    WRITE32(vme_base, HET_REG_OFF(3), FIDUCIAL_TURN(cycles) | TRIGGER_DELAY(triggerDelay) | OFFSET_FIDUCIAL(fiducialOffset));
    WRITE32(vme_base, HET_REG_OFF(5), T2_COUNTER(0) | SYNCR_INVERT);
  }
  // WRITE32(vme_base,HET_REG_OFF(3),(cycles<<16)|(fiducialOffset&0xFF)|((triggerDelay&0x3F)<<8));
  StatoCtrlReg(vme_base, 0, fplog);

  handle = chaos_crest_open(chaosserver);
  if (handle == NULL)
  {
    printf("## cannot open crest interface for url:%s\n", chaosserver);
    return -3;
  }
  cu0 = ADD_CU(handle, cuname, het_cu);
  if (cu0 == 0)
  {
    printf("## failed adding cu \"%s\"\n", cuname);
    return -4;
  }
  printf("* registering to %s...\n", chaosserver);
  // 0 means all defined CU
  chaos_crest_update(handle, cu0, 1, &eventn);
  chaos_crest_update(handle, cu0, 3, fifo);

  if ((ret = chaos_crest_register(handle, cu0)) != 0)
  {
    printf("## cannot register CUs, error:%d\n", ret);
    return -9;
  }
  printf("* registration average  %f ms...\n", chaos_crest_reg_time(handle));
  cnt = 0;
  trig_old = 0;

  // state of the board;
  sleep(10);

  {
    unsigned int regs[7];

    for (cnt = 0; cnt < 7; cnt++)
    {
      regs[cnt] = READ32(vme_base, HET_REG_OFF(cnt));
      printf("* read regs[%d]=0x%x\n", cnt, regs[cnt]);
      regs[cnt] = endian_swap(regs[cnt]);
    }
    chaos_crest_update(handle, cu0, 0, &regs[6]);

    chaos_crest_update(handle, cu0, 1, &eventn);
    //     chaos_crest_update(handle, cu0, 2, regs);
    chaos_crest_update(handle, cu0, 2, regs);

    chaos_crest_update(handle, cu0, 3, fifo);
    if ((ret = chaos_crest_push(handle, cu0, 0)) != 0)
    {
      printf("## error pushing ret:%d\n", ret);
      //  return ret;
    }
    if ((ret = chaos_crest_push(handle, cu0, 0)) != 0)
    {
      printf("## error pushing ret:%d\n", ret);
      //  return ret;
    }
    sleep(1);
  }

  printf("* pushing to %s...\nStarting..\n", chaosserver);

  while (1)
  {

    if (testmode)
    {
      AbilitazioneTriggerAut(vme_base);
      nn = 0;
      offset = 0xf80018;
      vme_ptr = (unsigned int *)((long)vme_base + offset);
      do
      {
        Vme_D32READ(base_address, vme_ptr, result);
        trig_work = (result & 0xfff00000) >> 20;
        delta_trig = (trig_old + 4) & 0x00000FFF;
        ;
        //printf("trig_old = %08x trig_work = %08x \n",trig_old,trig_work);
        nn++;
        if (nn == 1000)
        {
          printf("Errore Abilitazione Trigger Automatici \n");
          fflush(stdout);
          fprintf(fplog, "Errore Abilitazione Trigger Automatici \n");
          fflush(fplog);
          StatoCtrlReg(vme_base, 0, fplog);
          exit(-3);
        }
      } while (delta_trig != trig_work);
      trig_old = trig_work;
    }
    //  VmeRead(het_fd, R6_OFF, &readint, sizeof(readint));

    status = PollingVme(vme_base, &r6);
    //status = 2;
    chaos_crest_update(handle, cu0, 0, &r6);

    if (status <= HOWTOREAD)
    {
      printf("%u] status = %d 0x%x T2 DONE:%d, NEXT:%d\n", loop++, status, r6, GET_T2_DONE(r6), GET_T2_NEXT(r6));

      offset = 0x0;
      ret = VmeDmaRead(ciddma, offset, (char *)evtbuf, DMABUFFERSIZE); // deve leggere un quarto della fifo
                                                                       //ret = 0;
      if (ret < 0)
      {
        printf("Errore VmeDmaRead \n");
        fflush(stdout);
        fprintf(fplog, "Errore VmeDmaRead \n");
        fflush(fplog);
        StatoCtrlReg(vme_base, 0, fplog);
        //exit(-4);
        ResetMemoria(vme_base, fplog);
        StatoCtrlReg(vme_base, 0, fplog);
      }
      else
      {
        shm_state = SEARCH_HEADER;
        //for (cnt = (DMABUFFERSIZE / 4) - 1; cnt >= 0; cnt--)
        for (cnt = 0; cnt < (DMABUFFERSIZE / 4); cnt++)
        {

#ifdef MOTOROLA

          if (evtbuf[cnt] == 0xEFFFFFFF)
          {
            shm_state = HEADER1_FOUND;
#ifdef DEBUG
            fprintf(fplog, "[ev:%llu,pos:%d] HEADER1  0x%x\n", eventn, cnt, evtbuf[cnt]);
            fflush(fplog);
#endif
          }
          else if ((shm_state == HEADER1_FOUND) && ((evtbuf[cnt] & 0xFFFFF000) == 0xFD000000))
          {
            shm_state = HEADER2_FOUND;
            t2 = evtbuf[cnt] & 0xFFF;
#ifdef DEBUG

            fprintf(fplog, "==[ev:%llu,pos:%d] HEADER data=0x%x T2=%d==\n", eventn, cnt, evtbuf[cnt], evtbuf[cnt] & 0xFFF);
            fflush(fplog);
#endif
          }
          else if (((shm_state == HEADER2_FOUND) || (shm_state == DATA_FOUND)) && ((evtbuf[cnt] & 0x80000000) == 0x0))
          {
            shm_state = DATA_FOUND;
#ifdef DEBUG
            fprintf(fplog, "[ev:%llu,pos:%d] DATA 0x%x\n", eventn, cnt, evtbuf[cnt]);
            fflush(fplog);
#endif
          }
          else if ((shm_state == DATA_FOUND) && (evtbuf[cnt] & 0x80000000))
          {
            shm_state = FOOTER1_FOUND;
          }
          else if ((shm_state == FOOTER1_FOUND) && (evtbuf[cnt] & 0x80000000))
          {
            shm_state = FOOTER2_FOUND;
          }
          else if ((shm_state == FOOTER2_FOUND) && ((evtbuf[cnt] & 0xFFF00000) == 0xF7F00000))
          {
            eventn++;

#ifdef DEBUG

            fprintf(fplog, "[ev:%llu,pos:%d,orig:%d] END WORDS:%d\n", eventn, fifo_cnt, cnt, evtbuf[cnt] & 0x7FFF);
            fflush(fplog);
#endif
            //  fifo[fifo_cnt++] = endian_swap(evtbuf[cnt]& 0xFFF00000|t2);
         
            shm_state = SEARCH_HEADER;
          }
          fifo[cnt] = endian_swap(evtbuf[cnt]);

#else
          fifo[cnt] = evtbuf[cnt];

#endif
          /*if(evtbuf[cnt] == 0xdeaddead) {
            kprint++; 
            if(kprint == 1){ 
	            printf("Errore sui valori %d Vme Dma Read \n",cnt); fflush(stdout);
	            fprintf(fplog,"Errore sui valori %d Vme Dma Read \n",cnt); fflush(fplog);
              StatoCtrlReg(vme_base, fplog);
              //exit(-5);
            }
          } else {
            //printf("Letto da DMA[%d] = %08x \n",cnt,evtbuf[cnt]);
            fprintf(fpdata, "DMA : %08x \n", evtbuf[cnt]);
            fflush(fpdata);
          }
          evtbuf[cnt] = 0xdeaddead;
        }*/
        }
        chaos_crest_update(handle, cu0, 1, &eventn);
        //     chaos_crest_update(handle, cu0, 2, regs);

        chaos_crest_update(handle, cu0, 3, fifo);
      }
      if ((ret = chaos_crest_push(handle, cu0, 0)) != 0)
      {
        printf("## error pushing ret:%d\n", ret);
        //  return ret;
      }
    }
  }
  chaos_crest_close(handle);
  free(evtbuf);
  if (testmode == 0)
  {
    Disabilita_Trigger_Kloe(vme_base, fplog);
  }
  /*
   * Read last part of Memory with VME A32/D32
   */
  status = PollingVme(vme_base, &r6);
  nread = 0;
  offset = 0x0;
  vme_ptr = (unsigned int *)((long)vme_base + offset);
  if (status != 5)
  {
    do
    {
      Vme_D32READ(base_address, vme_ptr, result);
      if (result != 0xdeaddead)
      {
        fprintf(fpdata, "A32/D32 : %08x \n", result);
        fflush(fpdata);
        nread++;
      }

      Vme_D32READ(base_address, vme_ptr, result);
      if (result != 0xdeaddead)
      {
        fprintf(fpdata, "A32/D32 : %08x \n", result);
        fflush(fpdata);
        nread++;
      }

      status = PollingVme(vme_base, NULL);
    } while (status != 5);
  }
  if (nread > 0)
  {
    fflush(fpdata);
    printf("Letture D32 necessarie per vuotare la memoria = %d \n", nread);
    fprintf(fplog, "Letture D32 necessarie per vuotare la memoria = %d \n", nread);
  }

  DisabilitaScritturaFifo(vme_base);

  /*
    Close
  */
  //ResetMemoria1(vme_base, fplog);
  ResetMemoria(vme_base, fplog);
  printf("Fine Run \n");
  fprintf(fplog, "Fine Run \n");
  StatoCtrlReg(vme_base, 0, fplog);
  VmeCloseChannel(cidvme);
  VmeCloseChannel(ciddma);
  fflush(fpdata);
  fclose(fpdata);
  fflush(fplog);
  fclose(fplog);
  return 0;
}
