///////////////////////////////////////////////////////////////////////////
/*
	hardware.h
 	ELE542 - ÉTÉ2015
 	Jonathan Lapointe LAPJ05108303
 	Charles Trépanier TREC07029107

	Contains general purpose prototypes and definitions.
*/
///////////////////////////////////////////////////////////////////////////

#ifndef HARDWARE_H
#define HARDWARE_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include "includes.h"

//Definition des type de var declarés dans le code
#define U8  unsigned char 
#define U16 unsigned int  
#define S8  signed   char 
#define S16 signed   int  
#define U32 long
#define S32 signed long

typedef union uREG08    /*8 bit register with byte and bit access*/
{
	U8  byte;           /*access whole register e.g. var.byte   = 0xFF;*/
  	struct                /*access bit at a time  e.g. var.bit._7 = 1;   */
  	{
    	U8 b0 :1;
    	U8 b1 :1;
    	U8 b2 :1;
    	U8 b3 :1;
	    U8 b4 :1;
	    U8 b5 :1;
	    U8 b6 :1;
	    U8 b7 :1;
    }bit;
}tREG08;

/*
	@fn  void hwInit(void)
	@des init de la direction des  ports 
*/	
void hwInit(void);

#endif





		  
