///////////////////////////////////////////////////////////////////////////
/*
	SwNLed.c
 	ELE542 - ÉTÉ2015
 	Jonathan Lapointe LAPJ05108303
 	Charles Trépanier TREC07029107

	Contains functions related to the STK500's switches and LEDs.
*/
///////////////////////////////////////////////////////////////////////////

#include "hardware.h"
#include "SwNLed.h"
#include "Watchdog.h"
#include "moteur.h"

#define   SW_STOP_MSK  		 0x80 
#define   SW_START_MSK 		 0x40 
#define   LED_WAIT_LED 	     0x40
#define   LED_RUN_LED  		 0x80
#define   LED_COM_STATUS  	 0x02
#define   SW_STOP_THE_MOTORS 0xCC

/*
	void SLInit(void)
	@des initialise les dels d'affichage d'état du robot
*/
void SLInit(void)
{
	
	PORTB|=LED_RUN_LED;
	PORTB&=~LED_WAIT_LED;;
	PORTD|=SW_STOP_THE_MOTORS;		
}


/*
	U8 SLCheckSwStatusStop(void)
	@des verifie le bouton stop
	@return 1 si bouton appuyer sinon 0 
*/
U8 SLCheckSwStatusStop(void)
{
	if((PINA&SW_STOP_MSK)==0)
	{
		PORTB|=LED_RUN_LED;
		PORTB|=LED_COM_STATUS;
		PORTB&=~LED_WAIT_LED;	
		PORTD|=SW_STOP_THE_MOTORS;
		moteurSetMode(M_ARRET);
		return 1;
	}
	else 
		return 0;
}


/*
	U8 SLCheckSwStatusStart(void)
	@des verifie le bouton start
	@return 1 si bouton appuyer sinon 0
*/
U8 SLCheckSwStatusStart(void)
{
	if((PINA&SW_START_MSK)==0)
	{

		PORTB|=LED_WAIT_LED;
		PORTB&=~LED_RUN_LED;
		moteurSetMode(M_MARCHE);
		return 1;
	}
	else
		return 0;
}
