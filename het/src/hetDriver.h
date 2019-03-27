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

#endif
