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


/***********
 * DEFINES *
 ***********/

#define _TIMER_MIN 	15
#define _TIMER_SEC	0

// Initialise PB1 comme étant une sortie.
#define RELAY_INIT_IO()	(DDRB |= (1 << 1))
#define RELAY_SET(a) 	(PORTB = (PORTB & ~(1 << 1)) | ((a && 1) << 1))

// Retourne 1 si un mouvement est détecté, sinon 0.
#define CAPTEUR_MOUVEMENT_GET() 	(PINB & (1 << 0))
#define CAPTEUR_MOUVEMENT_INIT()	(PORTB |= (0x01))

// Retourne 1 si la porte est fermée, sinon 0.
#define CAPTEUR_PORTE_GET() 	(PINB & (1 << 6))
#define CAPTEUR_PORTE_INIT()	(PORTB |= (1 << 6))

#define DEL_INIT_IO() 			(DDRC |= (1 << 7))
#define DEL_FADE_INTERVAL 		5 // 5 * 4ms = 20ms.
#define DEL_INCREMENT_INTENSITE	4 // Variable qui indique par bons de combien l'intensitée de la DEL augmente
#define DEL_MAX_INTENSITE 		200
#define DEL_MIN_INTENSITE 		0

// Définition de nombre de cycle d'interruption qui représente 1sec.
// 15'000 * 4ms = 60sec.
#define _TIMER_CYCLE_TO_SEC_CNT 250

// Nombre de minutes comptées en interruption.
#define _TIMER_SEC_TO_MIN_CNT 	60


/********************
 *      ENUMS       *
 * STRUCTS & UNIONS *
 ********************/

typedef enum
{

	MOUVEMENT_NON_DETECTE,
	MOUVEMENT_DETECTE

} t_etat_mouvement;

typedef enum
{

	PORTE_OUVERTE,
	PORTE_FERMEE

} t_etat_porte;


/*************
 * VARIABLES *
 *************/

volatile uint8_t compteur_cycliques_1s = 0;
volatile uint16_t compteur_secondes = 0;	  // Variable permettant au relai de rester actif sur une periode de temps x après la fermeture de la porte.
volatile uint8_t compteur_minutes = 0;		  // Nombre de minutes
volatile uint8_t flag_temps_actif_relais = 0; // Varible qui vaut 1 lorsque le délai est atteint.

volatile uint8_t compteur_delai_intensite_del = 0;	   // Variable permettant d'avoir un délai entre chaque changement d'intensité de la DEL.
volatile uint8_t flag_delai_intensite_del_atteint = 0; // Varible qui vaut 1 lorsque le délai est atteint.

uint8_t etat_precedent_relai = 1;


/***************************
 * PROTOTYPES DE FONCTIONS *
 ***************************/

/**
 * @brief 
 * 
 */
void init_divers(void);

/**
 * @brief 
 * 
 */
void init_hardware(void);

/**
 * @brief 
 * 
 * @param etat 
 * @param etat_precedent 
 * @return uint8_t 
 */
uint8_t debounce(uint8_t etat, uint8_t *etat_precedent);

/**
 * @brief 
 * 
 * @param etat_mouvement 
 * @param etat_porte 
 */
void ajuster_intensite_del(t_etat_mouvement etat_mouvement,
						   t_etat_porte etat_porte);

/**
 * @brief 
 * 
 * @param etat_mouvement 
 * @param etat_porte 
 */
void gestion_etat_relais(t_etat_mouvement etat_mouvement,
						 t_etat_porte etat_porte);

/**
 * @brief 
 * 
 * @return t_etat_mouvement 
 */
t_etat_mouvement verifier_etat_mouvement(void);

/**
 * @brief 
 * 
 * @return t_etat_porte 
 */
t_etat_porte verifier_etat_porte(void);

/**
 *@brief  Fonction d'initialisation du timer #0.
 */
void init_timer0(void);

/**
 *@brief  Fonction d'initialisation du timer #4.
 */
void init_timer4(void);

/**
 * @brief 
 * 
 * @param valeur 
 * @param min 
 * @param max 
 * @return uint8_t 
 */
uint8_t contraindre_valeur(uint8_t valeur, uint8_t min, uint8_t max);


/********
 * MAIN *
 ********/

int main(void)
{

	init_divers();

	while (1)
	{

		t_etat_mouvement etat_mouvement = verifier_etat_mouvement();
		t_etat_porte etat_porte = verifier_etat_porte();

		if (flag_delai_intensite_del_atteint) // Si le flag est vrai...
		{

			flag_delai_intensite_del_atteint = 0;
			ajuster_intensite_del(etat_mouvement, etat_porte);
		}

		gestion_etat_relais(etat_mouvement, etat_porte);
	}
}


/*****************
 * INTERRUPTIONS *
 *****************/

/**
 *@brief Interruption qui génère les intervales de temps pour fade le DEL et compter le délai avant que la relai ne soit plus actif.
 */
ISR(TIMER0_COMPA_vect)
{

	compteur_cycliques_1s++;
	// compteur_secondes_temps_actif_relais++;

	if (compteur_secondes >= _TIMER_SEC)
	{
		// flag_temps_actif_relais = 1;
		//  Si le nombre de minutes voulu est lui aussi atteint...
		if (compteur_minutes >= _TIMER_MIN)
		{

			// Remise à 0 des minutes et des secondes.
			compteur_secondes -= _TIMER_SEC;
			compteur_minutes -= _TIMER_SEC_TO_MIN_CNT;

			flag_temps_actif_relais = 1;
		}
	}

	// Si 250 cycles (1 seconde) s'est écoulé...
	if (compteur_cycliques_1s >= _TIMER_CYCLE_TO_SEC_CNT)
	{
		// Compteur est remis à zéro à chaques seconde.
		compteur_cycliques_1s -= _TIMER_CYCLE_TO_SEC_CNT;

		// Incrément du compteur de secondes.
		compteur_secondes++;

		// Si 60 secondes (1 minute) s'est écoulé...
		if (compteur_secondes >= _TIMER_SEC_TO_MIN_CNT)
		{
			// flag_temps_actif_relais = 1;
			//  Compteur remis à zéro à chaque minute.
			compteur_secondes -= _TIMER_SEC_TO_MIN_CNT;

			// Incrément du compteur de minutes.
			compteur_minutes++;
		}

		// Si le nombre de secondes voulu est atteint...
	}

	compteur_delai_intensite_del++;
	if (compteur_delai_intensite_del >= DEL_FADE_INTERVAL) // Chaques 20ms la DEL augmente ou diminue d'intensité en faisant des bons de 4 pour un maximum de 200 (((0.020 * 200) / 4) = 1sec).
	{

		compteur_delai_intensite_del -= DEL_FADE_INTERVAL;
		flag_delai_intensite_del_atteint = 1;
	}
}


/****************************
 * DÉFINITIONS DE FONCTIONS *
 ****************************/

void init_divers(void)
{

	// Initialisation des E/S.
	init_hardware();

	// Initialisation des Timers.
	init_timer0();
	init_timer4();
}

void init_hardware(void)
{

	DEL_INIT_IO();
	RELAY_INIT_IO();
	CAPTEUR_PORTE_INIT();
}

void ajuster_intensite_del(t_etat_mouvement etat_mouvement,
						   t_etat_porte etat_porte)
{

	// Si la porte est ouverte ou qu'un mouvement est détecté (etat)
	// et que l'intensité de la DEL n'est pas encore au maximum...
	if ((etat_mouvement == MOUVEMENT_DETECTE ||
		 etat_porte == PORTE_OUVERTE) &&
		OCR4A < DEL_MAX_INTENSITE)
	{

		// Si l'intensité de la DEL est suppérieure au maximum,
		// elle est ramenée à DEL_MAX_INTENSITE.
		OCR4A = contraindre_valeur((OCR4A + DEL_INCREMENT_INTENSITE),
								   DEL_MIN_INTENSITE,
								   DEL_MAX_INTENSITE);
	}

	// Sinon, si la porte est fermée et qu'aucun mouvement n'est détecté (etat)
	// et que l'intensité de la DEL n'est pas encore au minimum...
	else if ((etat_mouvement == MOUVEMENT_NON_DETECTE ||
			  etat_porte == PORTE_FERMEE) &&
			 OCR4A > DEL_MIN_INTENSITE)
	{

		// Si l'intensité de la DEL est inférieure au minimum,
		// elle est ramenée à DEL_MIN_INTENSITE.
		OCR4A = contraindre_valeur((OCR4A - DEL_INCREMENT_INTENSITE),
								   DEL_MIN_INTENSITE,
								   DEL_MAX_INTENSITE);
	}
}

void gestion_etat_relais(t_etat_mouvement etat_mouvement,
						 t_etat_porte etat_porte)
{

	// Si la porte est ouverte ou qu'un mouvement est détecté...
	if (etat_mouvement == MOUVEMENT_DETECTE ||
		etat_porte == PORTE_OUVERTE)
	{

		RELAY_SET(1);		   // Le relai est activé.
		compteur_secondes = 0; // Remet le compteur à 0.
		compteur_minutes = 0;  // Remet le compteur des minutes à 0 chaques fois que la porte est ouverte.
	}

	else if (etat_mouvement == MOUVEMENT_NON_DETECTE &&
			 etat_porte == PORTE_FERMEE) // Si la porte est fermée et qu'aucun mouvement n'est détecté...
	{

		if (flag_temps_actif_relais) // Si le délai est écoulé...
		{

			flag_temps_actif_relais = 0;
			RELAY_SET(0); // Le relai n'est pas activé.
		}
	}
}

int verifier_activation_lumiere(void)
{

	// Retourne 1 si la porte est ouverte ou si un mouvement est détecté.
	return (!debounce(CAPTEUR_PORTE_GET(), &etat_precedent_relai) || CAPTEUR_MOUVEMENT_GET());
}

t_etat_mouvement verifier_etat_mouvement(void)
{

	return CAPTEUR_MOUVEMENT_GET();
}

t_etat_porte verifier_etat_porte(void)
{

	return debounce(CAPTEUR_PORTE_GET(), &etat_precedent_relai);
}

void init_timer0(void)
{

	// TCCR0A : COM0A1 COM0A0 COM0B1 COM0B0 – – WGM01 WGM00
	// TCCR0B : FOC0A FOC0B – – WGM02 CS02 CS01 CS00
	// TIMSK0 : – – – – – OCIE0B OCIE0A TOIE0
	TCCR0A |= (1 << WGM01);	 // CTC
	TCCR0B |= (1 << CS02);	 // Prescaler /256
	TIMSK0 |= (1 << OCIE0A); //
	OCR0A = 250 - 1;
	sei();
}

void init_timer4(void)
{

	// TCCR4A: COM4A1 COM4A0 COM4B1 COM4B0 FOC4A FOC4B PWM4A PWM4B
	// TCCR4B: PWM4X PSR4 DTPS41 DTPS40 CS43 CS42 CS41 CS40
	// TCCR4C: COM4A1S COM4A0S COM4B1S COMAB0S COM4D1 COM4D0 FOC4D PWM4D
	// TCCR4D: FPIE4 FPEN4 FPNC4 FPES4 FPAC4 FPF4 WGM41 WGM40
	// TCCR4E: TLOCK4 ENHC4 OC4OE5 OC4OE4 OC4OE3 OC4OE2 OC4OE1 OC4OE0
	TCCR4A |= (1 << COM4A1) | (1 << PWM4A);
	TCCR4B |= (1 << CS40) | (1 << CS43);
	OCR4C = 200 - 1;
	OCR4A = 0;
}

uint8_t contraindre_valeur(uint8_t valeur, uint8_t min, uint8_t max)
{

	uint8_t resultat = 0;

	if (valeur <= min)
	{

		resultat = min;
	}
	else if (valeur >= max)
	{

		resultat = max;
	}
	else
	{

		resultat = valeur;
	}

	return resultat;
}

uint8_t debounce(uint8_t etat, uint8_t *etat_precedent)
{

	uint8_t valeur_debounce = 0;

	if (etat && etat)
		valeur_debounce = 1;
	*etat_precedent = etat;

	return valeur_debounce;
}