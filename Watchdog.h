///////////////////////////////////////////////////////////////////////////
/*
	Watchdog.h
 	ELE542 - �T�2015
 	Jonathan Lapointe LAPJ05108303
 	Charles Tr�panier TREC07029107

	Contains prototypes and definitions for the system's watchdog.
*/
///////////////////////////////////////////////////////////////////////////

#ifndef WATCHDOG_H
#define WATCHDOG_H

#include "hardware.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#define WD_RESTART_WATCHDOG  WDTCR |= ((1<<WDTOE)|(1<<WDE))|(1<<WDP2)|(1<<WDP1)|(1<<WDP0)

/*
	void WdInit(void)
	@des init le watchdog 
*/
void WdInit(void);

/*
	void WdDisable(void)
	@des desactive le watchdog
*/
void WdDisable(void);

#endif

