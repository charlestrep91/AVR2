///////////////////////////////////////////////////////////////////////////
/*
	hardware.c
 	ELE542 - ÉTÉ2015
 	Jonathan Lapointe LAPJ05108303
 	Charles Trépanier TREC07029107

	Contains hardware related functions.
*/
///////////////////////////////////////////////////////////////////////////

#include "hardware.h"

#define HW_OUTPUT  1
#define HW_INPUT   0

/*
	@fn  void hwInit(void)
	@des init de la direction des  ports 
*/	
void hwInit(void)
{
	//init directions leds port en sortie
	DDRB = 0xff; 
	//init des directions pwm/uart/dir
	DDRD = 0xfe;
	//init des directions pin Calib/dir/sw
	DDRA = 0x10;
	//eteint les leds
	PORTB=0xff;
}
