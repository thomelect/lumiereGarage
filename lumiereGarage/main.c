/**
@file		main.c
@brief		code qui permet d'utiliser un module relais et un interrupteur de porte de type reed switch pour allumer automatiquement la lumi�re du garage l'orsque la porte s'ouvre.
lors de la fermeture de celle-ci, un compte � rebour d'une dur�e de 2min s'enclanche � la suite duquel le relai redevient inactif.
Tant et aussi longtemps que la porte est ouverte le relai reste actif lui aussi.
@author		Thomas Desrosiers
@version	1.0
@chip		Atmega32U4
@device		ArduinoMicro
@date		2020/2/16

@mainpage	microRelay
@author		Thomas Desrosiers
@section	MainSection1 Description
code qui permet d'utiliser un module relais et un interrupteur de porte de type reed switch pour allumer automatiquement la lumi�re du garage l'orsque quelqu'un ouvre la porte. lors de la fermeture de celle-ci, un compte � rebpur d'une dur�e de 2min s'enclanche � la suite duquel la lumi�re s'�teindra. tant et aussi longtemps que la porte est ouverte la lumi�re le reste elle aussi.
*/
#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

const uint8_t PLUS_MOINS = 4;//variable qui indique par bons de combien l'intensit�e de la del augmente
uint8_t porteToggle = 0;//variable qui indique si la porte � �t� ouverte avant d'avoir �t� ferm�e.

volatile uint16_t toggleCnt = 0;//variable permettant au relai de rester actif sur une periode de temps x apr�s la fermeture de la porte.
volatile uint8_t toggleFlag = 0;//varible qui vaut 1 lorsque le d�lai est atteint.
volatile uint8_t toggleCntMin = 0;//nombre de minutes
volatile uint16_t ledCnt = 0;//variable permettant d'avoir un d�lai entre chaque changement d'intensit� de la del.
volatile uint8_t ledFlag = 0;//varible qui vaut 1 lorsque le d�lai est atteint.

#define RELAY_INIT()	DDRB |= (1<<1)//initialise PB1 comme �tant une sortie.
#define RELAY_ON()		PORTB |= (1<<1)//active le relai.
#define RELAY_OFF()		PORTB &= ~(1<<1)//d�sactive le relai.

#define PULL_UP()		PORTB |= (1<<3)//active la pullup pour l'interrupteur.
#define SWITCH_MAGNET()	(PINB & (1<<3))

#define LED_INIT()		DDRC |= (1<<7)//initialise PC7 comme �tant une sortie.

/**
 *@brief  Fonction qui initialise le mode veille.
*/
void sleepModeON();//prototype de fonction.

/**
 *@brief Interruption qui g�n�re les intervales de temps pour fade le del et compter le d�lai avant que la relai ne soit plus actif.
*/
ISR(TIMER0_COMPA_vect)
{
	toggleCnt++;
	if (toggleCnt == 15000)//15'000 = 1min 62.5ns * 256 * 250 * 15'000 = 60s.
	{
		toggleCnt = 0;//compteur est remis � z�ro � chaques minutes.
		toggleCntMin++;
		if (toggleCntMin == 15)//d�lai est r�gl� � 15min.
		{
			toggleCntMin = 0;//compteur remis � z�ro � chaque 5minutes.
			toggleFlag = 1;
		}
	}
	ledCnt++;
	if (ledCnt == 5)//chaques 20ms la del augmente ou diminue d'intensit� en faisant des bons de 4 pour un maximum de 200 (((0.020 * 200) / 4) = 1sec).
	{
		ledCnt = 0;
		ledFlag = 1;
	}
}

int main(void)
{
	//macro
	LED_INIT();
	RELAY_INIT();
	PULL_UP();
	//timer 4 initialisation pour le pwm de la del.
	TCCR4A |= (1<<COM4A1) | (1<<PWM4A);
	TCCR4B |= (1<<CS40) | (1<<CS43);
	OCR4C = 200;
	OCR4A = 0;
	//timer 0 initialisation pour le d�lai du relai.
	TCCR0A |= (1<<WGM01);
	TCCR0B |= (1<<CS02);
	TIMSK0 |= (1<<OCIE0A);
	OCR0A = 249;
	sei();
	while (1)
	{
		if (ledFlag)//si le timer de del est = 1.
		{
			ledFlag = 0;
			if (!SWITCH_MAGNET() && (OCR4A < 200))//lorsque la porte est ouverte et que la del n'est pas � son intensit� maximale (200) son intensit� augmente.
			{
				OCR4A += PLUS_MOINS;//augmente l'intensit� de la del.
			}
			if (SWITCH_MAGNET() && (OCR4A > 0))//lorsque la porte est ferm�e et que la del n'est pas � son intensit� minimale (0) son intensit� diminue.
			{
				OCR4A -= PLUS_MOINS;//diminue l'intensit� de la del.
			}
		}
		if (!SWITCH_MAGNET())//si la porte est ouverte,
		{
			RELAY_ON();//le relai est activ�.
			porteToggle = 1;//permet de savoir si la porte � d�j� �t� ouverte depuis le d�marage afin de ne pas tomber inutilement en mode veille.
			toggleCnt = 0;//remet le compteur � 0.
			toggleCntMin = 0;//remet le compteur des minutes � 0 chaques fois que la porte est ouverte.
		}
		
		if (SWITCH_MAGNET())//si la porte est ferm�e,
		{
			if (toggleFlag)//si le d�lai est �coul�,
			{
				toggleFlag = 0;
				RELAY_OFF();//le relai n'est pas activ�.
				if (porteToggle)//si la porte � d�j� �t� ouverte,
				{
					porteToggle = 0;//l'�tat de la porte est ferm�e
					sleepModeON();//tombe en mode de veille.
				}
			}
		}
	}
}

/**
 *@brief  Fonction qui initialise le mode veille.
*/
void sleepModeON()
{
	SMCR |= (1<<SM1) | (1<<SE);//SM1 (Sleep Mode #1) fait r�f�rence au mode Power-down. Le mode Power-Down est choisi et le bit Sleep Enable est mis � 1.
	PCICR |= 1;//active l'interruption PCINT0.
	PCMSK0 |= (1<<PCINT3);//active l'interruption PCINT0 si PCINT3 change d'�tat.
	PRR0 |= (1<<PRTWI)/* | (1<<PRTIM0)*/ | (1<<PRTIM1) | (1<<PRSPI) | (1<<PRADC);//TWI, Timer/Counter0, Timer/Counter1, SPI and ADC sont d�sactiv�s pour r�duire la consomation pendant la veille.
	PRR1 |= (1<<PRUSB) | (1<<PRTIM4) | (1<<PRTIM3) | (1<<PRUSART1);//USB clock, Timer/Counter4, Timer/Counter3 and USART1 sont d�sactiv�s pour r�duire la consomation pendant la veille.
	sei();//sei doit �tre pr�sent affin de pouvoir se sortir du monde veille en utilisant une interruption.
	sleep_cpu();//passage en mode veille.
	SMCR &= ~(1<<SE);//au r�veil le bit SE doit �tre remis � 0.
}