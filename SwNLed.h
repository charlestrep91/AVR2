/*
 ELE542 
 Jonathan Lapointe LAPJ05108303
 Charles Trépanier 
*/


#ifndef SW_N_LED
#define SW_N_LED
#include "hardware.h"

/*
	void SLWaitForTheStartSw(void)
	@des attends pour la switch de démarrage
*/
void SLCheckSwStatus(void);

/*
	void SLWaitForTheStartSw(void)
	@des verifie letat de la switch d'arret d'urgence
	et bloque le programme et desactive les interruptions si 
	la switch est appuyer
*/
void SLWaitForTheStartSw(void);




#endif




