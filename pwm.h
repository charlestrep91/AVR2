///////////////////////////////////////////////////////////////////////////
/*
	pwm.h
 	ELE542 - �T�2015
 	Jonathan Lapointe LAPJ05108303
 	Charles Tr�panier TREC07029107

	Contains prototypes and definitions for pwm.c.
*/
///////////////////////////////////////////////////////////////////////////

#ifndef PWM_H
#define PWM_H
#include "hardware.h"

/*
	void pwmInit(void)
	@des init du pwm 
*/
void pwmInit(void);

/*
	void pwmSetDutyValue(U16 valueD,U16 valueG,U8 portValue)
	@des update les valeurs des variable qui seront pass� dans l'interruption 
*/
void pwmSetDutyValue(U16 valueD,U16 valueG,U8 portValue);

#endif
