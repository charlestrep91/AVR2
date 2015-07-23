#include "includes.h"
#include <stdio.h>
#include <avr/io.h>
#include "pwm.h"
#include "adc.h"
#include "moteur.h"
#include "hardware.h"



#define Pi (3.1415926535897932384626433832795)

U8 AngleD	= 0;
U8 VitesseD	= 0;
volatile U8 Data=0;


INT8U tpRx = 0;

#define STANDARD_STACK_SIZE 150
#define MOTEUR_STACK_SIZE   200
#define DEBUG_STACK_SIZE    150

OS_STK CmdTele_Stk[STANDARD_STACK_SIZE];
OS_STK MoteurTask_Stk[MOTEUR_STACK_SIZE];
OS_STK IOTask_Stk[STANDARD_STACK_SIZE];
OS_STK SendDbg_Stk[STANDARD_STACK_SIZE];
OS_STK MaTache_Stk[DEBUG_STACK_SIZE];

OS_EVENT	*UARTRxBox;
OS_EVENT	*UARTTxSem;
OS_EVENT	*UARTEchoSem;
OS_EVENT	*DebugMessageQ;
OS_EVENT	*MoteurUpdateSem;
OS_EVENT    *CmdTeleSem;

void *DebugMessage[10];

extern void InitOSTimer(void);

int  main(void);
       
static  void  CmdTeleTask(void *p_arg);
static  void  MoteurTask(void *p_arg);
//static  void  IOTask(void *p_arg);
//static  void  SendDbgTask(void *p_arg);
//static  void  MaTacheTask(void *p_arg);
ISR(USART_RXC_vect,ISR_NAKED) {
	OS_INT_ENTER();
	tpRx = UDR;
	OSMboxPost(UARTRxBox, &tpRx);
	OS_INT_EXIT();
}

ISR(USART_TXC_vect,ISR_NAKED) {
	OS_INT_ENTER();
	OSSemPost(UARTTxSem);
	OS_INT_EXIT();
}



void InitUART(void) {
	UCSRB = 0x00;
	UBRRH = 0;
	//9600Kbps@16Mhz (ATMega32)
	UBRRL = 103;	
	UCSRA = 0x00;
	UCSRC = 0x86;
	UCSRB = 0xD8;
}
int main(void) {
	cli();
	//inits
	OSInit();
	InitUART();	
	pwmInit(); 
	moteurSetMode(M_MARCHE);///juste pour tester
	hwInit();
	adcInit();
	adcStartConversion();

	//creation des taches 
      OSTaskCreate(MoteurTask,    NULL, (OS_STK *)&MoteurTask_Stk[MOTEUR_STACK_SIZE-1], 1);  
      OSTaskCreate(CmdTeleTask,   NULL, (OS_STK *)&CmdTele_Stk[STANDARD_STACK_SIZE-1] , 2); 	  
     // OSTaskCreate(SendDbgTask, NULL, (OS_STK *)&SendDbg_Stk[STANDARD_STACK_SIZE-1] , 3); 
      //OSTaskCreate(MaTacheTask, NULL, (OS_STK *)&MaTache_Stk[DEBUG_STACK_SIZE-1]    , 4);
	   
    //creation des semaphores
	UARTTxSem 		= OSSemCreate(1);
	UARTEchoSem 	= OSSemCreate(1);
	MoteurUpdateSem	= OSSemCreate(1);
	CmdTeleSem		= OSSemCreate(1);
	DebugMessageQ = OSQCreate(&DebugMessage[0],10);
	
	OSStart();
}


static  void  CmdTeleTask(void *p_arg) {
	INT8U	err;
	short	tp;
	INT8U	Etat = 0;
//	U8   etatRobot;

    (void)p_arg;          // Prevent compiler warnings


    while (TRUE) 
	{               // Task body, always written as an infinite loop.
		//OSSemPend(OSSemPost,0,&err);
        tp = *((short *) OSMboxPend(UARTRxBox, 0, &err));
		if (OSSemAccept(UARTEchoSem))
		{
			OSSemPend(UARTTxSem, 0, &err);
			UDR = tpRx;			
			OSSemPost(UARTEchoSem);
		}
		switch(Etat) 
		{
		case 0 :	if (tpRx == 0xF0) 
						VitesseD = 0;
					else if (tpRx == 0xF1) 
						Etat = 1;

					break;

		case 1 :	VitesseD =(U8)tpRx;
					Etat = 2;
					break;

		case 2 :	AngleD   =(U8)tpRx;
					Etat = 0;	
					//assignation des valeurs au controle Moteur					
					moteurControl(VitesseD,AngleD);
					break;
		}
    }
}


/*static  void  SendDbgTask(void *p_arg) {
	INT8U	err;
	char	*p;
	
    (void)p_arg;          // Prevent compiler warnings

    while (TRUE) 
	{   
	    // Task body, always written as an infinite loop.
        p = (char *) OSQPend(DebugMessageQ, 0, &err);
		OSSemPend(UARTEchoSem, 0, &err);
		OSSemPend(UARTTxSem, 0, &err);
		UDR = 0xFE;
		while (*p != 0) 
		{
			OSSemPend(UARTTxSem, 0, &err);
			UDR = *p++;
		}
		OSSemPend(UARTTxSem, 0, &err);
		UDR = 0xFF;
		OSSemPost(UARTEchoSem);
    }
}*/


/*static  void  MaTacheTask(void *p_arg) {
//	INT8U	err;
	char	message[20];
	INT8U	i = 0;
	
    (void)p_arg;          // Prevent compiler warnings

    while (TRUE) 
	{    // Task body, always written as an infinite loop.
		sprintf(message, "mon message %u",i++);
		OSQPost(DebugMessageQ, (void *) message);
		OSTimeDly(1000);
    }
}*/

static  void  MoteurTask(void *p_arg) {
	INT8U	err;


	
    (void)p_arg;          // Prevent compiler warnings

	cli();
	InitOSTimer();	
    sei();
	adcCalibSeq();

    while (TRUE) 
	{   
	   // Task body, always written as an infinite loop.
	    OSSemPend(MoteurUpdateSem, 0, &err);
		CalculMoteur();
    }
}


