/*
 ELE542 
 Jonathan Lapointe LAPJ05108303
 Charles Trépanier 
*/


#ifndef UART_H
#define UART_H

#include "hardware.h"
#include "includes.h"
#include <stdio.h>

//grandeur des FIFO (RX/TX)													
#define UART_RX_BUFFER_SIZE 60
#define UART_TX_BUFFER_SIZE 60

//definition utilisée pour la fonction void uartSendString(U8 *buf);	
#define UART_MAX_SIZE_STRING 	UART_TX_BUFFER_SIZE
#define UART_END_OF_STRING_CHAR  0


//Variables locales

//UART TASKS
OS_STK CmdTele_Stk[STANDARD_STACK_SIZE];
OS_STK SendDbg_Stk[STANDARD_STACK_SIZE];
OS_STK MaTache_Stk[DEBUG_STACK_SIZE];

/*
 initialise le UART
*/
void uartInit(void);

/*
 envoi une chaine de caractère qui doit se terminer 
 par NULL 
 La grandeur MAX est UART_MAX_SIZE_STRING
*/
void uartSendString(U8 *buf);


//static  void  sendDbgCmd(void *p_arg);
//static  void  CmdTeleTask(void *p_arg);


#endif
