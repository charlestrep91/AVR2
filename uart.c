/*
 ELE542 
 Jonathan Lapointe LAPJ05108303
 Charles Trépanier 
*/



#include "hardware.h"
#include "includes.h"
#include <stdio.h>
#include "uart.h"

#define Pi (3.1415926535897932384626433832795)

//UART COMMUNICATION SPEED CONFICGURATION
#ifndef F_CPU
#define F_CPU            16000000UL      /* Crystal 16.000 Mhz */
#endif

#define UART_BAUD_RATE      9600      /* 9600 baud */
#define UART_BAUD_SELECT (F_CPU/(UART_BAUD_RATE*16l)-1)
#define UART_RX_LED   0x02



//OS_EVENTs DEFINITIONS
OS_EVENT	*UARTRxBox;
OS_EVENT	*UARTTxSem;
OS_EVENT	*UARTEchoSem;
OS_EVENT	*DebugMessageQ;

//Variables pour les buffers
U8 rxByte;
U8 uartTxBuffer[UART_TX_BUFFER_SIZE];

//pointeurs et compteurs de données FIFOS
volatile U8 uartTxInPtr;
volatile U8 uartTxOutPtr;
volatile U8 uartTxSize;

float AngleD	= 0.0;
float VitesseD	= 0.0;


extern void InitOSTimer(void);

SIGNAL (SIG_UART_RECV) {
	OS_INT_ENTER();
	rxByte = UDR;
	OSMboxPost(UARTRxBox, &rxByte);
	OS_INT_EXIT();
}

SIGNAL (SIG_UART_TRANS) {
	OS_INT_ENTER();
	OSSemPost(UARTTxSem);
	OS_INT_EXIT();
}

void uartPutDbgStr(U8 *str)
{
	while((*str!=0)&&(uartTxSize<UART_TX_BUFFER_SIZE))
	{
		uartTxBuffer[uartTxInPtr]=*str;
		uartTxInPtr++;
		str++;
		uartTxSize++;
		if(uartTxInPtr>=UART_TX_BUFFER_SIZE)
			uartTxInPtr=0;
		
	}
}
/*
 Initialisation du Uart
*/
void uartInit(void)
{
    /* configure asynchronous operation, no parity, 1 stop bit, 8 data bits, Tx on rising edge */
    UCSRC = (1<<URSEL)|(0<<UMSEL)|(0<<UPM1)|(0<<UPM0)|(0<<USBS)|(1<<UCSZ1)|(1<<UCSZ0)|(0<<UCPOL);       
    /* enable RxD/TxD and ints */
    UCSRB = (1<<RXCIE)|(1<<TXCIE)|(1<<RXEN)|(1<<TXEN)|(0<<UCSZ2);       
    /* set baud rate */
    UBRRH = (U8)(UART_BAUD_SELECT >> 8);          
    UBRRL = (U8)(UART_BAUD_SELECT & 0x00FF);   
}

static  void  CmdTeleTask(void *p_arg) 
{
	U8	err;
	short	tp;
	U8	Etat = 0;

    (void)p_arg;          // Prevent compiler warnings

    cli();
	InitOSTimer();
	UARTRxBox = OSMboxCreate(NULL);
    sei();

    while (TRUE) {               // Task body, always written as an infinite loop.
        tp = *((short *) OSMboxPend(UARTRxBox, 0, &err));
		if (OSSemAccept(UARTEchoSem)) {
			OSSemPend(UARTTxSem, 0, &err);
			UDR = tp;
			OSSemPost(UARTEchoSem);
		}
		switch(Etat) {
		case 0 :	if (tp == 0xF0) {
						VitesseD = 0.0;
					} else if (tp == 0xF1) {
						Etat = 1;
					}
					break;
		case 1 :	VitesseD = ((float) tp) - 100.0;
					Etat = 2;
					break;
		case 2 :	AngleD = ((float) tp)*Pi/90.0;
					Etat = 0;
					break;
		}
    }
}





/*
 envoi une chaine de caractère avec caractere de fin
*/ 
static  void  sendDbgCmd(void *p_arg)
{ 
INT8U	err;
	char	*p;
	
    (void)p_arg;          // Prevent compiler warnings

    while (TRUE) {               // Task body, always written as an infinite loop.
        p = (char *) OSQPend(DebugMessageQ, 0, &err);
		OSSemPend(UARTEchoSem, 0, &err);
		OSSemPend(UARTTxSem, 0, &err);
		UDR = 0xFE;
		while (*p != 0) {
			OSSemPend(UARTTxSem, 0, &err);
			UDR = *p++;
		}
		OSSemPend(UARTTxSem, 0, &err);
		UDR = 0xFF;
		OSSemPost(UARTEchoSem);
    }
}





