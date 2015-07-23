/*
 ELE542 
 Jonathan Lapointe LAPJ05108303
 Charles Trépanier 
*/
#include "moteur.h"
#include "hardware.h"
#include "pwm.h"
#include "adc.h"
#include "includes.h"

#define Pi (3.1415926535897932384626433832795)


U8 lastVitesse=0;	
U8 lastAngle=0;
S16 lastVitG=0;
S16 lastVitD=0;
U8 mMode=0;

U16 dutyValD;
U16 dutyValG;


float mVitesse_D=0;		//vitesse formattee en float pour envoyer a la fonction CalculPWM
float mAngle_D=0;		//angle formatte en float pour envoyer a la fonction CalculPWM
float mVg=0;			//valeur ADC formatte en float pour envoyer a la fonction CalculPWM
float mVd=0;			//valeur ADC formatte en float pour envoyer a la fonction CalculPWM
float mDuty_G=0;		//duty cycle retourne par la fonction CalculPWM
float mDuty_D=0;		//duty cycle retourne par la fonction CalculPWM


/*
	void CalculPWM(float Vitesse_D, float Angle_D, float Vg, float Vd, float *Duty_G, float *Duty_D)
	@des calcul de l'asservisement 
*/
void CalculPWM(float Vitesse_D, float Angle_D, float Vg, float Vd, float *Duty_G, float *Duty_D)
{

	/**
		Dans cette fonction, la valeur des duty cycle pour chaque moteur est calculé.  
		Ce calcul est effectué à l`aide de la vitesse désirée, de l`angle désriré ainsi 
		que de la vitesse (mesurée avec CalculVitesses) et l`angle actuel.
	*/
	static float Angle = 0.0;
	static float ErreurAngle = 0.0;
	static float W = 0.0;
	static float Old_W = 0.0;
	static float Vt     = 0.0;
	static float Ut     = 0.0;
	static float Ua     = 0.0;
	static int 	 Signe_Ua = 0;
	static int 	 Signe_Ut = 0;


	Vg = (Vg > 1.0) ? 1.0 : ((Vg < -1.0) ? -1.0 : Vg);  /* Regarde les limites (-1.0 à 1.0) */
	Vd = (Vd > 1.0) ? 1.0 : ((Vd < -1.0) ? -1.0 : Vd);  /* Regarde les limites (-1.0 à 1.0) */
		
	Old_W = W;
	W     = 0.5*(Vmax/RAYON)*(Vd-Vg);
	Vt    = 0.5*(Vd+Vg);

	Angle =  Angle + (0.5)*TS*(W+Old_W);
	Angle = (Angle > 2.0*Pi) ? Angle - 2*Pi : ((Angle < 0.0) ? Angle + 2*Pi : Angle); /* Angle entre 0 et 2 pi */
	ErreurAngle = ((Angle_D >= Pi + Angle) ? Angle_D - 2*Pi : ((Angle_D <= -Pi + Angle) ? Angle_D + 2*Pi : Angle_D))-Angle;

	Ut = -H11*Vt + H12*Vitesse_D;
	Ua = H21*ErreurAngle - H22*W;
			
	Signe_Ua = (Ua >= 0.0) ? 1 : -1;
	Signe_Ut = (Ut >= 0.0) ? 1 : -1;
	
	Ua = (Signe_Ua*Ua > 1.0) ? Signe_Ua*1.0 : ((Signe_Ua*Ua <= 0.05) ? 0.0 : Ua);
	Ut = (Signe_Ut*Ut > 1.0) ? Signe_Ut*1.0 : Ut;
	Ut = ((Signe_Ut*Ut) > (1.0 - Signe_Ua*Ua)) ? Signe_Ut*(1.0 - Signe_Ua*Ua) : Ut;


	*Duty_D = (Ut+Ua);
	*Duty_G = (Ut-Ua);

	*Duty_D = (*Duty_D > 0.99) ? 0.99 : ((*Duty_D < -0.99) ? -0.99 : *Duty_D);
	*Duty_G = (*Duty_G > 0.99) ? 0.99 : ((*Duty_G < -0.99) ? -0.99 : *Duty_G);	
}
/*
	U8 moteurControl(U8 vitesse,U8 angle,U8 mode)
	@des fn qui est appeler par la fn de controle update les valeurs recues
*/
U8 moteurControl(U8 vitesse,U8 angle)
{	
	if(vitesse>VITESSEMAX || angle>ANGLEMAX)	//verifies if received value is not within accepted range
	{
		return 1;
	}
	if(vitesse!=lastVitesse || angle!=lastAngle)
	{ 
		if(vitesse!=lastVitesse)
		{
			mVitesse_D=(float)(vitesse-100)/100;
			
		}

		if(angle!=lastAngle)
		{
			mAngle_D=((float) angle)*Pi/90.0;;
		}

		lastVitesse=vitesse;
		lastAngle=angle;
	}
	return 0;
}

void moteurSetMode(U8 mode)
{
	mMode=mode;
}
/*
	void CalculMoteur(void)
	@des Calcul des valeurs envoyé au moteur et asservissement 
*/
void CalculMoteur(void)
{
tREG08 mPortDREG;		
#define M_DIR_G1 mPortDREG.bit.b2
#define M_DIR_G2 mPortDREG.bit.b3
#define M_DIR_D1 mPortDREG.bit.b6
#define M_DIR_D2 mPortDREG.bit.b7	  
 

		//MODE AVANT
		if(mMode!=M_MARCHE)
		{	
			switch(mMode)
			{
				case M_NEUTRE:
				M_DIR_G1=0;
				M_DIR_G2=0;
				M_DIR_D1=0;
				M_DIR_D2=0;

				break;

				//M_ARRET
				default:  
				M_DIR_G1=1;
				M_DIR_G2=1;
				M_DIR_D1=1;
				M_DIR_D2=1;
				break;
				
			}
			mDuty_G=0;
			mDuty_D=0;
			dutyValD=0;
			dutyValG=0;
		}
		else
		{
			adcCalculateAvg(&mVd,&mVg);	
			CalculPWM(mVitesse_D,mAngle_D,mVg,mVd,&mDuty_G,&mDuty_D);
			//MODE ARRIERE
			if(mDuty_G<0)
			{
				M_DIR_G1=0;
				M_DIR_G2=1;
				mDuty_G=mDuty_G*(-1);
			}
			//MODE AVANT
			else
			{
				M_DIR_G1=1;
				M_DIR_G2=0;	
			}	

			//MODE ARRIERE
			if(mDuty_D<0)
			{
				M_DIR_D1=0;
				M_DIR_D2=1;
				mDuty_D=mDuty_D*(-1);
			}
			//MODE AVANT
			else
			{
				M_DIR_D1=1;
				M_DIR_D2=0;	
			}
			
			dutyValD=(U16)(mDuty_D*10000);
			dutyValG=(U16)(mDuty_G*10000);		
		}
		pwmSetDutyValue(dutyValD,dutyValG,mPortDREG.byte);	

}



