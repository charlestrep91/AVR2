///////////////////////////////////////////////////////////////////////////
/*
	robot2.c
 	ELE542 - ÉTÉ2015
 	Jonathan Lapointe LAPJ05108303
 	Charles Trépanier TREC07029107

	Contains system's main function, tasks and declarations, initializations, 
	definitions	and ISRs.
*/
///////////////////////////////////////////////////////////////////////////

#include "includes.h"
#include <stdio.h>
#include <avr/io.h>
#include "pwm.h"
#include "adc.h"
#include "moteur.h"
#include "hardware.h"
#include "SwNLed.h"
#include "Watchdog.h"

#define Pi (3.1415926535897932384626433832795)

//variables angles et vitesse
INT8U AngleD	= 0;
INT8U VitesseD	= 0;

INT8U tpRx = 0;

//Definition des stack size
#define STANDARD_STACK_SIZE 150
#define MOTEUR_STACK_SIZE   200

//definition Led qui clignote a la reception Uart
#define UART_RX_LED   0x02

//definition des etats du bouton start
#define CHECK_FOR_START_BUTTON_STATE 0
#define CHECK_FOR_STOP_BUTTON_STATE  1

//definition stacks des taches
OS_STK CmdTele_Stk[STANDARD_STACK_SIZE];
OS_STK MoteurTask_Stk[MOTEUR_STACK_SIZE];
OS_STK IOTask_Stk[STANDARD_STACK_SIZE];

//declaration des OS_EVENT
OS_EVENT	*UARTRxBox;
OS_EVENT	*UARTTxSem;
OS_EVENT	*UARTEchoSem;
OS_EVENT	*MoteurUpdateSem;
OS_EVENT    *CmdTeleSem;
OS_EVENT    *InRunModeSem;

extern void InitOSTimer(void);

int  main(void);

//declaration prototypes des taches       
static  void  CmdTeleTask(void *p_arg);
static  void  MoteurTask(void *p_arg);
static  void  IOTask(void *p_arg);

/*
	ISR(USART_RXC_vect,ISR_NAKED)
	@des Isr de reception des données 
	provenant du uart et les places 
	dans la mailBox UartRxBox
*/
ISR(USART_RXC_vect,ISR_NAKED) 
{
	OS_INT_ENTER();
	tpRx = UDR;
	PORTB&=~UART_RX_LED;
	OSMboxPost(UARTRxBox, (void*)&tpRx);
	OS_INT_EXIT();
}

/*
	ISR(USART_TXC_vect,ISR_NAKED) 
	@des Isr d'envoi Uart fait 
	que posté UARTTxSem
*/
ISR(USART_TXC_vect,ISR_NAKED) 
{
	OS_INT_ENTER();
	PORTB|=UART_RX_LED;
	OSSemPost(UARTTxSem);
	OS_INT_EXIT();
}


/*
	void InitUART(void)
	@des initialise le uart
*/
void InitUART(void)
{
	UCSRB = 0x00;
	UBRRH = 0;
	//9600Kbps@16Mhz (ATMega32)
	UBRRL = 103;	
	UCSRA = 0x00;
	UCSRC = 0x86;
	UCSRB = 0xD8;
}

/*
	int main(void) 
	@des main
*/
int main(void) 
{
	cli();
	//initialisations
	OSInit();
	InitUART();	
	pwmInit(); 	
	hwInit();
	adcInit();
	adcStartConversion();
	SLInit();
	WdInit();

	//creation des taches 
    OSTaskCreate(MoteurTask,    NULL, (OS_STK *)&MoteurTask_Stk[MOTEUR_STACK_SIZE-1], 1);  
    OSTaskCreate(CmdTeleTask,   NULL, (OS_STK *)&CmdTele_Stk[STANDARD_STACK_SIZE-1] , 2); 	  
    OSTaskCreate(IOTask, NULL, (OS_STK *)&IOTask_Stk[STANDARD_STACK_SIZE-1] , 3); 
	   
    //creation des semaphores
	UARTTxSem 		= OSSemCreate(1);
	UARTEchoSem 	= OSSemCreate(1);
	MoteurUpdateSem	= OSSemCreate(1);
	CmdTeleSem		= OSSemCreate(1);
	InRunModeSem    = OSSemCreate(0);
	//demarage de l'o.s.
	OSStart();
}

/*
	static  void  CmdTeleTask(void *p_arg)
	@des tache de traitement des bytes recus et renvoi des bytes recus
*/
static  void  CmdTeleTask(void *p_arg) 
{
 	INT8U	err;
	INT8U	tp;
	INT8U	Etat = 0;

    (void)p_arg;          // Prevent compiler warnings
	
    while (TRUE) 
	{    // Task body, always written as an infinite loop.
        tp = *((INT8U *) OSMboxPend(UARTRxBox, 0, &err));
		//Kick le watchdog
		WdDisable();
		WD_RESTART_WATCHDOG;

		//verifie l'integrité du Mbox
		if (err == OS_NO_ERR) 
		{		
			//Echo data
			if (OSSemAccept(UARTEchoSem))
			{		
				OSSemPend(UARTTxSem, 0, &err);
				UDR = tp ;					
				OSSemPost(UARTEchoSem);
			}
			//traite les données si est en mode roulement
			if (OSSemAccept(InRunModeSem))
			{
				OSSemPost(InRunModeSem);
				switch(Etat) 
				{
					case 0 :	
						if (tp  == 0xF0) 
						{
							VitesseD = 0;
							moteurSetMode(M_ARRET);
						}
						else if (tp == 0xF1) 
						{
							Etat = 1;
						
						}
					break;

					case 1 :	
						VitesseD =tp;
						Etat = 2;
					break;

					case 2 :	
						AngleD   =tp;
						Etat = 0;	
						//assignation des valeurs au controle Moteur					
						moteurControl(VitesseD,AngleD);
						moteurSetMode(M_MARCHE);
					break;
				}
			}
		}
    }
}

/*
	static  void  IOTask(void *p_arg)
	@des tache qui vérifie l'état des switchs
	d'arrret et de mise en marche
	post et pend InRunModeSe
*/
static  void  IOTask(void *p_arg) 
{

	INT8U   state=CHECK_FOR_START_BUTTON_STATE;
	INT8U	err;
	
    (void)p_arg;          // Prevent compiler warnings

    while (TRUE) 
	{    // Task body, always written as an infinite loop.

		switch(state)
		{
			case CHECK_FOR_START_BUTTON_STATE:
				if(SLCheckSwStatusStart())
				{
					OSSemPost(InRunModeSem);
					state=CHECK_FOR_STOP_BUTTON_STATE;
					moteurSetMode(M_MARCHE);
				}
			break;

			case CHECK_FOR_STOP_BUTTON_STATE:
				if(SLCheckSwStatusStop())
				{
					OSSemPend(InRunModeSem, 0, &err);
					state=CHECK_FOR_START_BUTTON_STATE;
					moteurSetMode(M_ARRET);
					AngleD	    = 0;
				    VitesseD	= 0;
				}
			break;

			default:
				state=CHECK_FOR_START_BUTTON_STATE;
			break;
		}
    }
}

/*
	static  void  MoteurTask(void *p_arg) 
	@des tache prioritaire affectue des inits et le 
	calculPWM lorsque que la semaphore MoteurUpdateSem 
	est posté
*/
static  void  MoteurTask(void *p_arg) 
{

	INT8U	err;	
    (void)p_arg; // Prevent compiler warnings

	cli();//OS_CRITICAL_INT
	InitOSTimer();
	UARTRxBox = OSMboxCreate(NULL);	
    sei();//EXIT_OS_CRITICAL
	adcCalibSeq();
	//kick le watchdog
	WdDisable();
	WD_RESTART_WATCHDOG;
	//met le robot en mode arret au demarrage
	moteurSetMode(M_ARRET);

    while (TRUE) 
	{   
	   // Task body, always written as an infinite loop.
	   	OSSemPend(InRunModeSem, 0, &err);
		OSSemPost(InRunModeSem);
	    OSSemPend(MoteurUpdateSem, 0, &err);
		//appel de la fonction d'asservissement moteur
		CalculMoteur();		
    }
}


