/**
@file		main.c
@brief		Code qui permet d'utiliser un module relais, un interrupteur de porte de type reed switch ainsi qu'un détecteur de mouvements pour allumer automatiquement la lumière du garage lorsque quelqu'un ouvre la porte ou qu'un mouvement est détecté.
			Lors de la fermeture de celle-ci et en l'absence de mouvement, un compte à rebour d'une durée de 10min s'enclanche à la suite duquel la lumière s'éteindra.
			Tant et aussi longtemps que la porte est ouverte ou qu'un mouvement est détecté, la lumière le reste elle aussi.
@author		Thomas Desrosiers
@version	2.0
@chip		ATmega32U4
@device		Arduino Micro
@date		2020/2/16

@mainpage	lumiereGarage
@author		Thomas Desrosiers
@section	MainSection1 Description
			Code qui permet d'utiliser un module relais, un interrupteur de porte de type reed switch ainsi qu'un détecteur de mouvements pour allumer automatiquement la lumière du garage lorsque quelqu'un ouvre la porte ou qu'un mouvement est détecté.
			Lors de la fermeture de celle-ci et en l'absence de mouvement, un compte à rebour d'une durée de 10min s'enclanche à la suite duquel la lumière s'éteindra.
			Tant et aussi longtemps que la porte est ouverte ou qu'un mouvement est détecté, la lumière le reste elle aussi.
*/

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>

#define RELAY_INIT()			(DDRB |= (1<<1)) //initialise PB1 comme étant une sortie.
#define RELAY_SET(a)			(PORTB = (PORTB & ~(1<<1)) | ((a && 1) << 1)) //État du relais.
#define CAPTEUR_MOVE()			(PINB & (1<<0))
#define CAPTEUR_PORTE_INIT()	(PORTB |= (1<<3)) //active la pullup pour l'interrupteur.
#define CAPTEUR_PORTE()			(PINB & (1<<3))
#define DEL_INIT()				(DDRC |= (1<<7)) //initialise PC7 comme étant une sortie.
#define _TIMER_SEC_CYCLE_CNT	15000 //15'000 * 4ms = 60sec.
#define _TIMER_MIN_CYCLE_CNT	10 //Nombre 10 minutes comptées en interruption.

const uint8_t PLUS_MOINS = 4; //Variable qui indique par bons de combien l'intensitée de la DEL augmente

volatile uint16_t toggleCntSec = 0; //Variable permettant au relai de rester actif sur une periode de temps x après la fermeture de la porte.
volatile uint8_t toggleFlag = 0; //Varible qui vaut 1 lorsque le délai est atteint.
volatile uint8_t toggleCntMin = 0; //Nombre de minutes
volatile uint16_t ledCnt = 0; //Variable permettant d'avoir un délai entre chaque changement d'intensité de la DEL.
volatile uint8_t ledFlag = 0; //Varible qui vaut 1 lorsque le délai est atteint.

/**
*@brief  Fonction d'initialisation des différents I/O et fonctions.
*/
void miscInit(void);

/**
*@brief  Fonction d'initialisation du timer #0.
*/
void timer0Init(void);

/**
*@brief  Fonction d'initialisation du timer #4.
*/
void timer4Init(void);

int main(void)
{
	miscInit();
	while (1)
	{
		if (ledFlag) //Si le flag est vrai...
		{
			ledFlag = 0;
			if ((!CAPTEUR_PORTE() || CAPTEUR_MOVE()) && (OCR4A < 200)) //Si la porte est ouverte ou qu'un mouvement est détecté et que la DEL n'est pas à son intensité maximale (200)...
			{
				OCR4A += PLUS_MOINS; //Augmente l'intensité de la DEL.
				if (OCR4A >= 200)
				{
					OCR4A = 200;
				}
			}
			if ((CAPTEUR_PORTE() && !CAPTEUR_MOVE()) && (OCR4A > 0)) //Si la porte est fermée et qu'aucun mouvement n'est détecté et que la DEL n'est pas à son intensité minimale (0)...
			{
				OCR4A -= PLUS_MOINS; //Diminue l'intensité de la DEL.
				if (OCR4A <= 0)
				{
					OCR4A = 0;
				}
			}
		}
		if ((!CAPTEUR_PORTE() || CAPTEUR_MOVE())) //Si la porte est ouverte ou qu'un mouvement est détecté...
		{
			RELAY_SET(1); //Le relai est activé.
			toggleCntSec = 0; //Remet le compteur à 0.
			toggleCntMin = 0; //Remet le compteur des minutes à 0 chaques fois que la porte est ouverte.
		}
		
		if ((CAPTEUR_PORTE() && !CAPTEUR_MOVE())) //Si la porte est fermée et qu'aucun mouvement n'est détecté...
		{
			if (toggleFlag) //Si le délai est écoulé...
			{
				toggleFlag = 0;
				RELAY_SET(0); //Le relai n'est pas activé.
			}
		}
	}
}

/**
*@brief Interruption qui génère les intervales de temps pour fade le DEL et compter le délai avant que la relai ne soit plus actif.
*/
ISR(TIMER0_COMPA_vect)
{
	toggleCntSec++;
	if (toggleCntSec >= _TIMER_SEC_CYCLE_CNT) //15'000 = 1min 62.5ns * 256 * 250 * 15'000 = 60s.
	{
		toggleCntSec -= _TIMER_SEC_CYCLE_CNT; //Compteur est remis à zéro à chaques minutes.
		toggleCntMin++;
		if (toggleCntMin >= _TIMER_MIN_CYCLE_CNT) //Délai est réglé À 10min.
		{
			toggleCntMin -= _TIMER_MIN_CYCLE_CNT; //Compteur remis à zéro à chaque 5minutes.
			toggleFlag = 1;
		}
	}
	
	ledCnt++;
	if (ledCnt == 5) //Chaques 20ms la DEL augmente ou diminue d'intensité en faisant des bons de 4 pour un maximum de 200 (((0.020 * 200) / 4) = 1sec).
	{
		ledCnt = 0;
		ledFlag = 1;
	}
}

void miscInit(void)
{
	//Initialisation des E/S
	DEL_INIT();
	RELAY_INIT();
	CAPTEUR_PORTE_INIT();
	
	//Initialisation des Timers.
	timer0Init();
	timer4Init();
}

void timer0Init(void)
{
	//TCCR0A : COM0A1 COM0A0 COM0B1 COM0B0 – – WGM01 WGM00
	//TCCR0B : FOC0A FOC0B – – WGM02 CS02 CS01 CS00
	//TIMSK0 : – – – – – OCIE0B OCIE0A TOIE0
	
	TCCR0A |= (1<<WGM01);
	TCCR0B |= (1<<CS02);
	TIMSK0 |= (1<<OCIE0A);
	OCR0A = 250-1;
	sei();
}

void timer4Init(void)
{
	//TCCR4A: COM4A1 COM4A0 COM4B1 COM4B0 FOC4A FOC4B PWM4A PWM4B
	//TCCR4B: PWM4X PSR4 DTPS41 DTPS40 CS43 CS42 CS41 CS40
	//TCCR4C: COM4A1S COM4A0S COM4B1S COMAB0S COM4D1 COM4D0 FOC4D PWM4D
	//TCCR4D: FPIE4 FPEN4 FPNC4 FPES4 FPAC4 FPF4 WGM41 WGM40
	//TCCR4E: TLOCK4 ENHC4 OC4OE5 OC4OE4 OC4OE3 OC4OE2 OC4OE1 OC4OE0
	
	TCCR4A |= (1<<COM4A1) | (1<<PWM4A);
	TCCR4B |= (1<<CS40) | (1<<CS43);
	OCR4C = 200-1;
	OCR4A = 0;
}