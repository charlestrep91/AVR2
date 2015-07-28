///////////////////////////////////////////////////////////////////////////
/*
	pwm.h
 	ELE542 - ÉTÉ2015
 	Jonathan Lapointe LAPJ05108303
 	Charles Trépanier TREC07029107

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
	@des update les valeurs des variable qui seront passé dans l'interruption 
*/
void pwmSetDutyValue(U16 valueD,U16 valueG,U8 portValue);

#endif
