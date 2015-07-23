/**
	@file moteur.h
	@brief Header pour le fichier moteur.c
	@author Bruno De Kelper et Louis-Bernard Lagueux
	@date 13 Octobre 2006
*/

/*
 ELE542 
 Jonathan Lapointe LAPJ05108303
 Charles Trépanier 
*/
#ifndef __MOTOR_H_
#define __MOTOR_H_
#include "hardware.h"

#define Pi      (3.1415926535897932384626433832795)
#define RAYON   (9.525)
#define TS      (0.005)
#define Vmax    (80.0)
#define Tau     (0.5)

#define H11		(3.90148347975678) 
#define H12		(4.90148347975678)

#define H21		(1.1613504)
#define H22		(0.5806746734)

#define Fpwm (10000)

#define M_MARCHE 0
#define M_NEUTRE 1
#define M_ARRET  2

#define VITESSEMAX 	200
#define	ANGLEMAX	180

/*
	void CalculPWM(float Vitesse_D, float Angle_D, float Vg, float Vd, float *Duty_G, float *Duty_D)
	@des calcul de l'asservisement 
*/
void CalculPWM(float Vitesse_D, float Angle_D, float Vg, float Vd, float *Duty_G, float *Duty_D);


/*
	U8 moteurControl(U8 vitesse,U8 angle,U8 mode)
	@des fn qui est appeler par la fn de controle update les valeurs recues
*/
U8 moteurControl(U8 vitesse,U8 angle);


/*
	void CalculMoteur(void)
	@des Calcul des valeurs envoyé au moteur et asservissement 
*/
void CalculMoteur(void);


/*
	void moteurUpdateDutys(void)
	@des leve le drapeau d'update du calcul d'asservissement 
*/
void moteurUpdateDutys(void);


/*
	void moteurSetMode(U8 mode)
	@des fn qui met le robot en mode :M_ARRET,M_MARCHE,M_NEUTRE
*/
void moteurSetMode(U8 mode);

#endif
