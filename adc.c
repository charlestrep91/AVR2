///////////////////////////////////////////////////////////////////////////
/*
	adc.c
 	ELE542 - ÉTÉ2015
 	Jonathan Lapointe LAPJ05108303
 	Charles Trépanier TREC07029107

	Contains the ISR and the functions used to process motor speed values 
	read from the MCU's ADC.
*/
///////////////////////////////////////////////////////////////////////////

#include "adc.h"
#include "moteur.h"
#include "hardware.h"
#include <avr/interrupt.h>


#define ADC_MOTEUR_GAUCHE 	  0x00
#define ADC_MOTEUR_DROIT  	  0x01
#define ADC_MOTEUR_DROIT_MSK  0x08
#define ADC_MOTEUR_GAUCHE_MSK 0x04
#define ADC_MUX_SETTTING  	(0<<REFS1)|(0<<REFS0)|(0<<ADLAR)
#define ADC_NEG_VALUE 	  	1		
#define ADC_POS_VALUE     	0 
#define ADC_NB_SAMPLE_MAX 	32
#define ADC_CAL_BIT   		0x10
#define ADC_SET_CAL  		PORTA|= ADC_CAL_BIT
#define ADC_CLEAR_CAL 		PORTA&=~ADC_CAL_BIT

volatile U8  adcMuxState	 =0;
volatile S32 adcMoteurGAvg	 =0;
volatile S32 adcMoteurDAvg	 =0;
volatile S32 adcNbSamplesG	 =0;
volatile S32 adcNbSamplesD   =0;
volatile U16 Mcounter		 =0;
volatile S16 adcValue		 =0;
volatile U8  adcUpdateSetting=0;
volatile U8  adcMotMskVarD=0;
volatile U8  adcMotMskVarG=0;


//Valeurs de calibration 
 S32 adcCalibMinValGP=0;
 S32 adcCalibMaxValGP=1024;
 S32 adcCalibMinValGM=0;
 S32 adcCalibMaxValGM=1024;
 S32 adcCalibMinValDP=0;
 S32 adcCalibMaxValDP=1024;
 S32 adcCalibMinValDM=0;
 S32 adcCalibMaxValDM=1024;
 

//@fn utilisé localement 
void adcReadCalibValue(S32 *adcValD,S32 *adcValG);

/*
	@fn adcInit(void)
	@des init du peripherique adc
*/
void adcInit(void)
{
    
	ADMUX=ADC_MOTEUR_DROIT;	
 	ADCSRA=(1<<ADEN )
		  	|(0<<ADSC )	
		 	|(1<<ADATE)	
		  	|(0<<ADIF )
		  	|(1<<ADIE )
		  	|(1<<ADPS2)
		  	|(1<<ADPS1)
		  	|(1<<ADPS0);	

	//init letat de la variable de lecture
   	adcMuxState=ADC_MOTEUR_DROIT;	
	SFIOR=0x0F&SFIOR;
}

/*
	@fn void adcStartConversion(void)
	@des demarre les lectures
*/
void adcStartConversion(void)
{
   ADCSRA |= 1<<ADSC;
}


/*
	@fn ISR(ADC_vect)
	@des routine d'interruption
*/
ISR(ADC_vect,ISR_NAKED)
{
	OS_INT_ENTER();
	ADMUX = (ADMUX & 0x01) ? (0x00) : (0x01);
	if(ADMUX&0x01)
	{	
			//assignation de la valeur de mux pour la prochaine conversion
		adcNbSamplesD++;
			//addition ou soustraction de la valeur lue selon le bit de signe
		adcMoteurDAvg+=	((PINA&adcMotMskVarD)==0)? ((S32)(ADC&0x03ff)):(-(S32)(ADC&0x03ff));
	}
	else 
	{
		//assignation de la valeur de mux pour la prochaine conversion
		adcNbSamplesG++;
		//addition ou soustraction de la valeur lue selon le bit de signe
		adcMoteurGAvg+=	((PINA&adcMotMskVarG)==0)? ((S32)(ADC&0x03ff)):(-(S32)(ADC&0x03ff));
	}
	OS_INT_EXIT();
}

/*
	void adcCalculateAvg(float * moteurD, float * moteurG)
	@des calcul des valeurs moyennes 
	@arg pointeur sur valeur des vitesse moteurs 
*/	
void adcCalculateAvg(float * moteurD, float * moteurG)
{
	S32 nbSampleD;
	S32 nbSampleG;
	S32 MoteurDVal;
	S32 MoteurGVal;
	cli();
	nbSampleD=adcNbSamplesD;
	nbSampleG=adcNbSamplesG;
	MoteurDVal=adcMoteurDAvg;
	MoteurGVal=adcMoteurGAvg;		
	sei();

	*moteurD=(MoteurDVal<0)?((float)((MoteurDVal/nbSampleD)+adcCalibMinValDM)/adcCalibMaxValDM):
		((float)((MoteurDVal/nbSampleD)-adcCalibMinValDP)/adcCalibMaxValDP);
	*moteurG=(MoteurGVal<0)?((float)((MoteurGVal/nbSampleG)+adcCalibMinValGM)/adcCalibMaxValGM):
		((float)((MoteurGVal/nbSampleG)-adcCalibMinValGP)/adcCalibMaxValGP);
	adcMoteurDAvg=0;
	adcMoteurGAvg=0;
	adcNbSamplesG=0;
	adcNbSamplesD=0;
	
	
}

/*
	void adcReadCalibValue(S32 *adcValD,S32 *adcValG)
	@des calcul des moyennes en type S32 pour la calibration
*/
void adcReadCalibValue(S32 *adcValD,S32 *adcValG)
{
	S32 nbSampleD;
	S32 nbSampleG;
	S32 MoteurDVal;
	S32 MoteurGVal;
	cli();
	nbSampleD=adcNbSamplesD;
	nbSampleG=adcNbSamplesG;
	MoteurDVal=adcMoteurDAvg;
	MoteurGVal=adcMoteurGAvg;		
	sei();
	*adcValD=MoteurDVal/nbSampleD;
	*adcValG=MoteurGVal/nbSampleG;
	adcMoteurDAvg=0;
	adcMoteurGAvg=0;
	adcNbSamplesG=0;
	adcNbSamplesD=0;
	
}

/*
	void adcCalibSeq(void)
	@des sequence de calibration
*/	
void adcCalibSeq(void)
{
	tREG08 portDreg;
	//VMAX calib+ lecture
	TIMSK=0;
	adcMotMskVarD=0;
	adcMotMskVarG=0;
	ADC_SET_CAL;
	portDreg.bit.b4=0;
	portDreg.bit.b5=0;
	portDreg.bit.b7=0;
	portDreg.bit.b3=0;
	portDreg.bit.b6=1;
	portDreg.bit.b2=1;
	PORTD=portDreg.byte;
	while(adcNbSamplesG<32);	
	OCR1A=0;
  	OCR1B=0;
	adcReadCalibValue(&adcCalibMaxValDP,&adcCalibMaxValGP);
	while(adcNbSamplesG<60);
	adcReadCalibValue(&adcCalibMaxValDP,&adcCalibMaxValGP);
	//VMIN calib+ lecture
	ADC_CLEAR_CAL;
	portDreg.bit.b4=0;
	portDreg.bit.b5=0;
	portDreg.bit.b7=0;
	portDreg.bit.b3=0;
	portDreg.bit.b6=1;
	portDreg.bit.b2=1;
	PORTD=portDreg.byte;
	OCR1A=0;
  	OCR1B=0;
	while(adcNbSamplesG<32);	
	adcReadCalibValue(&adcCalibMinValDP,&adcCalibMinValGP);
	while(adcNbSamplesG<60);
	adcReadCalibValue(&adcCalibMinValDP,&adcCalibMinValGP);

	//VMAX calib- lecture
	ADC_SET_CAL;
	portDreg.bit.b4=0;
	portDreg.bit.b5=0;
	portDreg.bit.b7=1;
	portDreg.bit.b3=1;
	portDreg.bit.b6=0;
	portDreg.bit.b2=0;
	PORTD=portDreg.byte;
	while(adcNbSamplesG<32);	
	OCR1A=0;
  	OCR1B=0;
	adcReadCalibValue(&adcCalibMaxValDM,&adcCalibMaxValGM);
	while(adcNbSamplesG<60);
	adcReadCalibValue(&adcCalibMaxValDM,&adcCalibMaxValGM);

	//VMIN calib+ lecture
	ADC_CLEAR_CAL;
	portDreg.bit.b4=0;
	portDreg.bit.b5=0;
	portDreg.bit.b7=1;
	portDreg.bit.b3=1;
	portDreg.bit.b6=0;
	portDreg.bit.b2=0;
	PORTD=portDreg.byte;
	OCR1A=0;
  	OCR1B=0;
	while(adcNbSamplesG<32);	
	adcReadCalibValue(&adcCalibMinValDM,&adcCalibMinValGM);
	while(adcNbSamplesG<60);
	adcReadCalibValue(&adcCalibMinValDM,&adcCalibMinValGM);

	TIMSK=(1<<TOIE1);
	ADC_CLEAR_CAL;
	adcMotMskVarD=ADC_MOTEUR_DROIT_MSK;
	adcMotMskVarG=ADC_MOTEUR_GAUCHE_MSK;
		
	adcCalibMaxValDP=(S32)(adcCalibMaxValDP-adcCalibMinValDP);
	adcCalibMaxValGP=(S32)(adcCalibMaxValGP-adcCalibMinValGP);
 	adcCalibMaxValDM=(S32)(adcCalibMaxValDM-adcCalibMinValDM);
	adcCalibMaxValGM=(S32)(adcCalibMaxValGM-adcCalibMinValGM);
	

}

