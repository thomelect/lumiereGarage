/**
@file		main.c
@brief		code qui permet d'utiliser un module relais et un interrupteur de porte de type reed switch pour allumer automatiquement la lumière du garage l'orsque la porte s'ouvre.
lors de la fermeture de celle-ci, un compte à rebour d'une durée de 2min s'enclanche à la suite duquel le relai redevient inactif.
Tant et aussi longtemps que la porte est ouverte le relai reste actif lui aussi.
@author		Thomas Desrosiers
@version	1.0
@chip		Atmega32U4
@device		ArduinoMicro
@date		2020/2/16

@mainpage	microRelay
@author		Thomas Desrosiers
@section	MainSection1 Description
code qui permet d'utiliser un module relais et un interrupteur de porte de type reed switch pour allumer automatiquement la lumière du garage l'orsque quelqu'un ouvre la porte. lors de la fermeture de celle-ci, un compte à rebpur d'une durée de 2min s'enclanche à la suite duquel la lumière s'éteindra. tant et aussi longtemps que la porte est ouverte la lumière le reste elle aussi.
*/
#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define RELAY_INIT()		DDRB |= (1<<1) //initialise PB1 comme étant une sortie.
#define RELAY_ON()			PORTB |= (1<<1) //active le relai.
#define RELAY_OFF()			PORTB &= ~(1<<1) //désactive le relai.
#define SWITCH_DOOR_INIT()	PORTB |= (1<<3) //active la pullup pour l'interrupteur.
#define SWITCH_DOOR()		(PINB & (1<<3))
#define LED_INIT()			DDRC |= (1<<7) //initialise PC7 comme étant une sortie.

const uint8_t PLUS_MOINS = 4; //variable qui indique par bons de combien l'intensitée de la DEL augmente
uint8_t porteToggle = 0; //variable qui indique si la porte à été ouverte avant d'avoir été fermée.
volatile uint16_t toggleCnt = 0; //variable permettant au relai de rester actif sur une periode de temps x après la fermeture de la porte.
volatile uint8_t toggleFlag = 0; //varible qui vaut 1 lorsque le délai est atteint.
volatile uint8_t toggleCntMin = 0; //nombre de minutes
volatile uint16_t ledCnt = 0; //variable permettant d'avoir un délai entre chaque changement d'intensité de la DEL.
volatile uint8_t ledFlag = 0; //varible qui vaut 1 lorsque le délai est atteint.

/**
*@brief  Fonction d'initialisation des différents I/O et fonctions.
*/
void miscInit(void);

/**
*@brief  Fonction qui initialise le mode veille.
*/
void sleepModeON(void); //prototype de fonction.

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
			if (!SWITCH_DOOR() && (OCR4A < 200)) //Lorsque la porte est ouverte et que la DEL n'est pas à son intensité maximale (200) son intensité augmente.
			{
				OCR4A += PLUS_MOINS; //Augmente l'intensité de la DEL.
			}
			if (SWITCH_DOOR() && (OCR4A > 0)) //Lorsque la porte est fermée et que la DEL n'est pas à son intensité minimale (0) son intensité diminue.
			{
				OCR4A -= PLUS_MOINS; //Diminue l'intensité de la DEL.
			}
		}
		if (!SWITCH_DOOR()) //Si la porte est ouverte,
		{
			RELAY_ON(); //Le relai est activé.
			porteToggle = 1; //Permet de savoir si la porte à déjà été ouverte depuis le démarage afin de ne pas tomber inutilement en mode veille.
			toggleCnt = 0; //Remet le compteur à 0.
			toggleCntMin = 0; //Remet le compteur des minutes à 0 chaques fois que la porte est ouverte.
		}
		
		if (SWITCH_DOOR()) //Si la porte est fermée,
		{
			if (toggleFlag) //Si le délai est écoulé,
			{
				toggleFlag = 0;
				RELAY_OFF(); //Le relai n'est pas activé.
				if (porteToggle) //Si la porte à déjà été ouverte,
				{
					porteToggle = 0; //État de la porte == fermée
					sleepModeON(); //Mode veille.
				}
			}
		}
	}
}

/**
*@brief Interruption qui génère les intervales de temps pour fade le DEL et compter le délai avant que la relai ne soit plus actif.
*/
ISR(TIMER0_COMPA_vect)
{
	toggleCnt++;
	if (toggleCnt == 15000) //15'000 = 1min 62.5ns * 256 * 250 * 15'000 = 60s.
	{
		toggleCnt = 0; //compteur est remis à zéro à chaques minutes.
		toggleCntMin++;
		if (toggleCntMin == 15) //délai est réglé À 15min.
		{
			toggleCntMin = 0; //compteur remis à zéro à chaque 5minutes.
			toggleFlag = 1;
		}
	}
	ledCnt++;
	if (ledCnt == 5) //chaques 20ms la DEL augmente ou diminue d'intensité en faisant des bons de 4 pour un maximum de 200 (((0.020 * 200) / 4) = 1sec).
	{
		ledCnt = 0;
		ledFlag = 1;
	}
}

void miscInit(void)
{
	//Initialisation des E/S
	LED_INIT();
	RELAY_INIT();
	SWITCH_DOOR_INIT();
	
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
	OCR0A = 249;
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
	OCR4C = 200;
	OCR4A = 0;
}

void sleepModeON(void)
{
	//SMCR: – – – – SM2 SM1 SM0 SE
	//PRR0: PRTWI – PRTIM0 – PRTIM1 PRSPI – PRADC
	//PRR1: PRUSB – – PRTIM4 PRTIM3 – – PRUSART1
	SMCR |= (1<<SM1) | (1<<SE); //SM1 (Sleep Mode #1) fait référence au mode Power-down. Le mode Power-Down est choisi et le bit Sleep Enable est mis à 1.
	PRR0 |= (1<<PRTWI)/* | (1<<PRTIM0)*/ | (1<<PRTIM1) | (1<<PRSPI) | (1<<PRADC); //TWI, Timer/Counter0, Timer/Counter1, SPI and ADC sont désactivés pour réduire la consomation pendant la veille.
	PRR1 |= (1<<PRUSB) | (1<<PRTIM4) | (1<<PRTIM3) | (1<<PRUSART1); //USB clock, Timer/Counter4, Timer/Counter3 and USART1 sont désactivés pour réduire la consomation pendant la veille.
	PCICR |= 1; //Active l'interruption externe de type "Pin Change Interrupt". Tout changement sur une des broches PCINT0 à PCINT7 provoquera une interruption.
	PCMSK0 |= (1<<PCINT3); //Active l'interruption PCINT0 si PCINT3 change d'état.
	sei(); //sei doit être présent afin de pouvoir se sortir du monde veille en utilisant une interruption.
	sleep_cpu(); //Passage en mode veille.
	SMCR &= ~(1<<SE); //Au réveil le bit SE doit être remis à 0.
}