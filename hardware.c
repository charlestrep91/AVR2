/*
 ELE542 

 Jonathan Lapointe LAPJ05108303
 Charles Trépanier 
*/
#include "hardware.h"


#define HW_OUTPUT  1
#define HW_INPUT   0
/*
	@fn  void hwInit(void)
	@des init de la direction des  ports 
*/	
void hwInit(void)
{
	//init leds port
	DDRB = 0xff; 
	DDRD = 0xfe;
	DDRA = 0x10;
	PORTB=0xff;
}
