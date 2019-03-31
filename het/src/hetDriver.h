#ifndef _HETDRIVER_
#define _HETDRIVER_
#include <stdio.h>

void ResetMemoria(unsigned int* base_address, FILE *fplog);
void AbilitazioneTriggerAut(unsigned int* base_address);
int PollingVme(unsigned int* base_address,unsigned int*reg);
void StatoCtrlReg(unsigned int* base_address, unsigned int*regs,FILE *fplog);
void InitTDCV5(unsigned int *base_address, FILE *fplog);
void InitTDCV5_KLOE(unsigned int *base_address, FILE *fplog);
void Disabilita_Trigger_Kloe(unsigned int *base_address, FILE *fplog);
void ResetMemoria1(unsigned int* base_address, FILE *fplog);
void DisabilitaScritturaFifo(unsigned int *base_address);
void flushLine2(void *ramptr);
void cacheinv2(void *ramptr);
#define HET_REG_OFF(n) (0x00f80000 + n*4)
#define READ32(base,off) *(unsigned int*)((long)base+off)
#define WRITE32(base,off,data) *(unsigned int*)((long)base+off)=data;
// register 1
#define ENABLE_T1 (1<<31)
#define ENABLE_T2 (1<<30)
#define GBL_RESET_ENABLE (1<<29)
#define MEM_RESET_ENABLE (1<<28)
#define FSM_RESET_ENABLE (1<<27)
#define PLL_RESET_ENABLE (1<<26)
#define CTRL_RESET_ENABLE (1<<25)
#define FIFO_NORMAL (0<<23)
#define FIFO_TEST_WPATTERN (1<<23)
#define FIFO_TEST_WVME (2<<23)
#define FIFO_TEST_WNONE (2<<23)
#define FIFO_TRG_AUTO (1<<22)
#define FIFO_TRG_AUTO_DATA (1<<21)
#define FIFO_TRG_UKN (1<<20)

// register 3
#define TRIGGER_TEST_REG (1<<31)
#define FIDUCIAL_TURN(n) ((n&0x1F)<<16)
#define ENABLE_ZEROSUPPRESSION ((9)<<16)
#define TRIGGER_DELAY(n) ((n&0x7F)<<8)
#define OFFSET_FIDUCIAL(n) ((n&0xFF))
// register 4
#define GBLRESET_GLOBAL 1
#define GBLRESET_MEM 2
#define GBLRESET_FSM 4
#define GBLRESET_PLL 8
#define GBLREST_CTRL 0x10


// register 5
#define GOLDEN_NUMBER(x) (x&0xFFF)
#define SYNCF_INVERT (1<<12)
#define SYNCR_INVERT (1<<13)
#define T2_COUNTER(x) ((x&0xFFFF)<<16)

// status register 6
#define GET_T2_DONE(x) ((x>>20)&0xFFF)
#define GET_T2_NEXT(x) ((x>>8)&0xFFF)
#define GET_FIFO_STAT(x) (x&0xFF)

//
#endif
