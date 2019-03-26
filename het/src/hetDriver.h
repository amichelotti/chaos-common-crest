#ifndef _HETDRIVER_
#define _HETDRIVER_
#include <stdio.h>

void ResetMemoria(unsigned int* base_address, FILE *fplog);
void AbilitazioneTriggerAut(unsigned int* base_address);
int PollingVme(unsigned int* base_address);
void StatoCtrlReg(unsigned int* base_address, unsigned int*regs,FILE *fplog);
void InitTDCV5(unsigned int *base_address, FILE *fplog);
void InitTDCV5_KLOE(unsigned int *base_address, FILE *fplog);
void Disabilita_Trigger_Kloe(unsigned int *base_address, FILE *fplog);
void ResetMemoria1(unsigned int* base_address, FILE *fplog);
void DisabilitaScritturaFifo(unsigned int *base_address);
void flushLine2(void *ramptr);
void cacheinv2(void *ramptr);

#endif
