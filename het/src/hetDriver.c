
#define VME_MAP_ADDRESS 0x01000000
#define VME_MAP_SIZE    1024*1024*32
#define VME_MAP_AM      0x9 
#define DMABUFFERSIZE   1024*60 //0x2000000

#ifdef MOTOROLA

//#include "vmelib.h"
#define R6_OFF (0x00f80000 + 6 * 4)
#include <Vme.h>

//#include "Data.h"
#include <stdio.h>
#include <time.h>

void flushLine2(void *ramptr){
  asm("dcbf 0,3");
  asm("sync");
}

void cacheinv2(void *ramptr){
  asm("dcbz 0,3");
  asm("sync");
}

#else
 void flushLine2(void *ramptr){
}

 void cacheinv2(void *ramptr){
}

#define Vme_D32WRITE(x,y,z)
#define Vme_D32READ(x,y,z)
#endif
#include "hetDriver.h"

void ResetMemoria(unsigned int* base_address, FILE *fplog){
  unsigned int offset, result, result_save;
  unsigned int *vme_ptr;
  
  printf("\nBegin ResetMemoria \n");
  fprintf(fplog,"\nBegin ResetMemoria \n");
  //Leggo lo stato di slave reg 1
  offset =  0x00f80004;
  vme_ptr = (unsigned int*)((long)base_address + offset);
  Vme_D32READ(base_address,vme_ptr,result_save);
  result = 0x80000000;
  Vme_D32WRITE(base_address,vme_ptr,result);
  printf("Valore Inizile letto su SR1 save = %08x \n",result_save);
  
  //  Abilito i RESET
  offset =  0x00f80004;
  vme_ptr = (unsigned int*)((long)base_address + offset);
  // 1 abilita reset memoria della periferica, 8 abilita reset delle machine a stati
  result = 0x98000000;
  Vme_D32WRITE(base_address,vme_ptr,result);
  //printf("Reset abilitato %08x !!\n",result);
  //Vme_D32READ(base_address,vme_ptr,result);
  printf("Reset riletto %08x !!\n",result);
  
  // Reset
  offset = 0x00f80010;
  vme_ptr = (unsigned int*)((long)base_address + offset);
  result = 0x00000006;		// 2(reset memoria) + 4(reset macchine a stati) 
  Vme_D32WRITE(base_address,vme_ptr,result);
  usleep(200);
  result = 0x00000000;		// default 
  Vme_D32WRITE(base_address,vme_ptr,result);
  printf("Reset effetuato memorie e FSM!!\n");
    
  //Disabilito il reset
  offset = 0x00f80004;
  vme_ptr = (unsigned int*)((long)base_address + offset);
  result = result_save;	        //	rimetto il valore iniziale
  Vme_D32WRITE(base_address,vme_ptr,result);
  
  //Leggo lo stato di slave reg 1
  offset = 0x00f80004;
  vme_ptr = (unsigned int*)((long)base_address + offset);
  Vme_D32READ(base_address,vme_ptr,result);
  printf("Valore letto su SR1 = %08x \n",result);
  
  //Leggo lo stato di slave reg 6
  offset = 0x00f80018;
  vme_ptr = (unsigned int*)((long)base_address + offset);
  Vme_D32READ(base_address,vme_ptr,result);
  printf("Valore letto su SR6 = %08x \n",result);
  usleep(200);
  fprintf(fplog,"End ResetMemoria \n");
  fflush(fplog);
}



 
void AbilitazioneTriggerAut(unsigned int* base_address){
  unsigned int *vme_ptr;
  unsigned int offset, result, result_save;

  //Leggo lo stato di slave reg 1
  offset =  0x00f80004;
  vme_ptr = (unsigned int*)((long)base_address + offset);
  Vme_D32READ(base_address,vme_ptr,result_save);
  //printf("Valore Iniziale letto su SR1 = %08x \n",result_save);
  
  //printf("BEGIN ABILITAZIONE \n");
  //Abilito la generazione automatica dei trigger e il consenso al T1
  //abilita T1 + e manda T1 in automatico
  offset = 0x00f80004;
  vme_ptr = (unsigned int*)((long)base_address + offset);
  // 8 = abilita trigger di primo livello 4 = abilita generazione automatica dei trigger
  result = 0x80400000;
  Vme_D32WRITE(base_address,vme_ptr,result);
  usleep(200);
  
  // abilita T1 e T2 a genero trigger 4T1 e 4T2     
  offset = 0x00f80004;
  vme_ptr = (unsigned int*)((long)base_address + offset);
  // C = abilita trigger di primo livello + abilita trigger secondo livello 4 = abilita generazione automatica dei trigger
  result = 0xC0400000;	
  Vme_D32WRITE(base_address,vme_ptr,result);
  usleep(200);
  //printf("\n\nTrigger automatici abilitati \n\n"); 

  // rimetto il valore iniziale
  offset = 0x00f80004;
  vme_ptr = (unsigned int*)((long)base_address + offset);
  result = result_save;
  Vme_D32WRITE(base_address,vme_ptr,result);
  
  usleep(200);
}
int PollingVme(unsigned int* base_address) {
  int retry = 0;
  int status = 90;
  unsigned int result = 90;
  unsigned int *vme_ptr;
  unsigned int offset = 0xF80018;
  
  int FifoOccupancy;
  
  vme_ptr = (unsigned int*)((unsigned int)base_address + offset);
  
  do {
    Vme_D32READ(base_address,vme_ptr,result);
    if (result < 1) printf("Error reading Vme\n");
    retry++;
  } while ( (retry <= 10) && (result < 1) );
  
  if (retry > 10){
    status = 99;
    return(status);
  }
  
  FifoOccupancy = result & 0xFC;
  //printf("%02x %d \n",FifoOccupancy,number_of_emptyPoll);

  switch (FifoOccupancy){
    case 0xd8: 	// Vuota 	
      status = 5;
      break;
    case 0xdc:	// < 1/4
      status = 4;
      break;
    case 0xd4:	// > 1/4 && < 1/2
      status = 3;
      break;
    case 0xe4:	// > 1/2 && < 3/4
      status = 2;
      break;
    case 0xa4:	// > 3/4 && < 1
      status = 1;
      break;
    case 0x24: 	// Piena
      status = 0;
      break;
    default:
      status = 90;
      break;
  }
  
  return(status);

}
void StatoCtrlReg(unsigned int* base_address, unsigned int*regs,FILE *fplog) {
  int numReg;
  unsigned int *vme_ptr;
  unsigned int offset, result;

  for(numReg=0;numReg<7;numReg++){
    offset =  0x00f80000 + numReg*4;;
    vme_ptr = (unsigned int*)((long)base_address + offset);
    Vme_D32READ(base_address,vme_ptr,result);
    if(fplog){
      printf("Valore letto su SR0 = %08x \n",result);
      fprintf(fplog,"Valore letto su SR1 = %08x \n",result);
    }
    if(regs){
      regs[numReg]=result;
    }
  }
}

void InitTDCV5(unsigned int *base_address, FILE *fplog) {
  int numReg;
  unsigned int *vme_ptr;
  unsigned int offset, result;

  printf("Init TDC \n");
  fprintf(fplog,"Init TDC \n");

//Leggo lo stato di slave reg 0
  numReg = 0;
  offset =  0x00f80000 + numReg*4;;
  vme_ptr = (unsigned int*)((long)base_address + offset);
  Vme_D32READ(base_address,vme_ptr,result);
  if ( result != 0xFFFFFFFF ) { // tutti i canali abilitati
    result = 0xFFFFFFFF;
    Vme_D32WRITE(base_address,vme_ptr,result);
  }

//Leggo lo stato di slave reg 1
  numReg = 1;
  offset =  0x00f80000 + numReg*4;;
  vme_ptr = (unsigned int*)((long)base_address + offset);
  Vme_D32READ(base_address,vme_ptr,result);
  if ( result != 0xC0100000 ) {
    result = 0x80100000;
    Vme_D32WRITE(base_address,vme_ptr,result);
    sleep(2);
    result = 0xC0100000;
    Vme_D32WRITE(base_address,vme_ptr,result);
  }

//Leggo lo stato di slave reg 3
  numReg = 3;  
  offset =  0x00f80000 + numReg*4;;
  vme_ptr = (unsigned int*)((long)base_address + offset);
  Vme_D32READ(base_address,vme_ptr,result);
  //if ( result != 0x00030000 ) {
  //  result = 0x00030000;
  if ( result != 0x00090000 ) {
    result = 0x00090000;
    Vme_D32WRITE(base_address,vme_ptr,result);
  }

//Leggo lo stato di slave reg 5
  numReg = 5;
  offset =  0x00f80000 + numReg*4;;
  vme_ptr = (unsigned int*)((long)base_address + offset);
  Vme_D32READ(base_address,vme_ptr,result);
  if ( result != 0x0000307a ) {
    result = 0x0000307a;
    Vme_D32WRITE(base_address,vme_ptr,result);
  }
  printf("Fine TDC \n");
  fprintf(fplog,"Fine Init TDC \n");

}

void InitTDCV5_KLOE(unsigned int *base_address, FILE *fplog) {
  int numReg;
  unsigned int *vme_ptr;
  unsigned int offset, result;


  printf("Init TDC KLOE \n");
  fprintf(fplog,"Init TDC KLOE \n");
//Leggo lo stato di slave reg 0
  numReg = 0;
  offset =  0x00f80000 + numReg*4;;
  vme_ptr = (unsigned int*)((long)base_address + offset);
  Vme_D32READ(base_address,vme_ptr,result);
  if ( result != 0xFFFFFFFF ) { // tutti i canali abilitati
    result = 0xFFFFFFFF;
    Vme_D32WRITE(base_address,vme_ptr,result);
  }

//Leggo lo stato di slave reg 1
  numReg = 1;
  offset =  0x00f80000 + numReg*4;;
  vme_ptr = (unsigned int*)((long)base_address + offset);
  Vme_D32READ(base_address,vme_ptr,result);
  if ( result != 0xC0100000 ) {
    //result = 0x80100000;
    //Vme_D32WRITE(base_address,vme_ptr,result);
    //sleep(10);
    result = 0xC0100000;
    Vme_D32WRITE(base_address,vme_ptr,result);
  }

//Leggo lo stato di slave reg 3
  numReg = 3;  
  offset =  0x00f80000 + numReg*4;;
  vme_ptr = (unsigned int*)((long)base_address + offset);
  Vme_D32READ(base_address,vme_ptr,result);
  //if ( result != 0x00030000 ) {
  //  result = 0x00030000;
  if ( result != 0x00090000 ) {
    result = 0x00090000;
    Vme_D32WRITE(base_address,vme_ptr,result);
  }

//Leggo lo stato di slave reg 5
  numReg = 5;
  offset =  0x00f80000 + numReg*4;;
  vme_ptr = (unsigned int*)((long)base_address + offset);
  Vme_D32READ(base_address,vme_ptr,result);
  if ( result != 0x0000307a ) {
    //result = 0x0000307a;
    result = 0x0000207b;
    Vme_D32WRITE(base_address,vme_ptr,result);
  }

  printf("Fine TDC KLOE \n");
  fprintf(fplog,"Fine Init TDC KLOE\n");

}

void Disabilita_Trigger_Kloe(unsigned int *base_address, FILE *fplog){
  int numReg;
  unsigned int *vme_ptr;
  unsigned int offset, result;

  printf("Disabilita FIFO \n");
  fprintf(fplog,"Disabilita FIFO \n");
//Leggo lo stato di slave reg 1
  numReg = 1;
  offset =  0x00f80000 + numReg*4;;
  vme_ptr = (unsigned int*)((long)base_address + offset);
  Vme_D32READ(base_address,vme_ptr,result);
  if ( result != 0 ) {
    result = 0x0;
    Vme_D32WRITE(base_address,vme_ptr,result);
  }
  printf("Fine Disabilita FIFO \n");
  fprintf(fplog,"Fine Disabilita FIFO \n");

}


void ResetMemoria1(unsigned int* base_address, FILE *fplog){
  unsigned int offset, result, result_save;
  unsigned int *vme_ptr;
  
  printf("\nBegin ResetMemoria \n");
  fprintf(fplog,"Begin ResetMemoria \n");
  //Leggo lo stato di slave reg 1
  offset =  0x00f80004;
  vme_ptr = (unsigned int*)((long)base_address + offset);
  Vme_D32READ(base_address,vme_ptr,result_save);
  result = 0x0;
  Vme_D32WRITE(base_address,vme_ptr,result);
  //printf("Valore Inizile letto su SR1 save = %08x \n",result_save);
  
  //  Abilito i RESET
  offset =  0x00f80004;
  vme_ptr = (unsigned int*)((long)base_address + offset);
  // 1 abilita reset memoria della periferica, 8 abilita reset delle machine a stati
  result = 0x38000000;
  Vme_D32WRITE(base_address,vme_ptr,result);
  //printf("Reset abilitato %08x !!\n",result);
  //Vme_D32READ(base_address,vme_ptr,result);
  //printf("Reset riletto %08x !!\n",result);
  
  // Reset
  offset = 0x00f80010;
  vme_ptr = (unsigned int*)((long)base_address + offset);
  result = 0x00000007;		// 2(reset memoria) + 4(reset macchine a stati) 
  Vme_D32WRITE(base_address,vme_ptr,result);
  result = 0x00000000;		// default 
  Vme_D32WRITE(base_address,vme_ptr,result);
  //printf("Reset effetuato memorie e FSM!!\n");
    
  //Disabilito il reset
  offset = 0x00f80004;
  vme_ptr = (unsigned int*)((long)base_address + offset);
  result = result_save;	        //	rimetto il valore iniziale
  Vme_D32WRITE(base_address,vme_ptr,result);
  
  offset = 0x00f80004;
  vme_ptr = (unsigned int*)((long)base_address + offset);
  result = 0;
  Vme_D32WRITE(base_address,vme_ptr,result);
  //printf("Valore letto su SR1 = %08x \n",result);

  //Leggo lo stato di slave reg 1
  offset = 0x00f80004;
  vme_ptr = (unsigned int*)((long)base_address + offset);
  Vme_D32READ(base_address,vme_ptr,result);
  //printf("Valore letto su SR1 = %08x \n",result);
  
  //Leggo lo stato di slave reg 6
  offset = 0x00f80018;
  vme_ptr = (unsigned int*)((long)base_address + offset);
  Vme_D32READ(base_address,vme_ptr,result);
  //printf("Valore letto su SR6 = %08x \n",result);
  usleep(200);
  fprintf(fplog,"End ResetMemoria \n");
  fflush(fplog);
  
}

void DisabilitaScritturaFifo(unsigned int *base_address) {
  int numReg;
  unsigned int *vme_ptr;
  unsigned int offset, result;

//Leggo lo stato di slave reg 1
  numReg = 1;
  offset =  0x00f80000 + numReg*4;;
  vme_ptr = (unsigned int*)((long)base_address + offset);
  Vme_D32READ(base_address,vme_ptr,result);
  if ( result != 0xC1800000 ) {
    result = 0xC1800000;
    Vme_D32WRITE(base_address,vme_ptr,result);
  }
}
