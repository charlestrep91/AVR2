/*
 ELE542 
 Jonathan Lapointe LAPJ05108303
 Charles Trépanier 
*/


#ifndef SW_N_LED
#define SW_N_LED
#include "hardware.h"

/*
	U8 SLCheckSwStatusStop(void)
	@des verifie le bouton stop
	@return 1 si bouton appuyer sinon 0 
*/
U8 SLCheckSwStatusStop(void);


/*
	U8 SLCheckSwStatusStart(void)
	@des verifie le bouton start
	@return 1 si bouton appuyer sinon 0
*/
U8 SLCheckSwStatusStart(void);


/*
	void SLInit(void)
	@des initialise les dels d'affichage
	d'état du robot
*/
void SLInit(void);




#endif




