/*************************
 * DOCUMENTATION FICHIER *
 *************************/
/**
 * @file	   main.c
 * @author	   Thomas DESROSIERS
 * @brief      Contrôle d'une lumière de garage à l'aide d'un module 
 *             relais avec délai d'extinction.
 * 
 * @details    Ce fichier contient les définitions des fonctions permettant
 *             la gestion d'une lumière de garage à l'aide d'un module relais.
 *             La lumière s'active en réponse à la détection d'un mouvement
 *             par un capteur et reste allumée pendant un certain délai
 *             après que la porte du garage ait été fermée.
 *             Il inclut également les fonctions de détection d'état 
 *             du capteur de mouvement
 *             et du capteur de porte pour activer ou désactiver la lumière
 *             en fonction de ces événements.
 * @version	   3.0
 * @date 	   2023/08/20
 * 
 * @copyright  Copyright (c) 2023 Thomas DESROSIERS
 * 
 */

/************************
 * DOCUMENTATION PROJET *
 ************************/
/**
 * @mainpage   	main.c
 * @section		MainSection1 Informations du projet
 * @subsection 	SubSection1 Description du projet
 * 			   	Contrôle d'une lumière de garage à l'aide d'un module 
 *              relais avec délai d'extinction.
 * 
 * @line       	-----------------------------------------------
 * 
 * @subsection  SubSection2 Auteur
 * 			   	Thomas DESROSIERS
 * @subsection 	SubSection3 Version
 * 			   	3.0
 * @subsection 	SubSection4 Date
 * 			   	2023/08/20
 * @subsection 	SubSection5 Documentation supplémentaire
 *             	Pour plus de détails sur l'implémentation de chaque fonction,\n
 *             	veuillez vous référer à la documentation dans le code source.\n
 *             	Vous y trouverez des informations sur les paramètres d'entrée,\n
 *             	les valeurs de retour et les cas d'utilisation.
 * 
 */

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>


/***********
 * DEFINES *
 ***********/

/****************************************************
 * DÉFINITIONS RELATIVES AU TIMER ET AUX COMPTEURS. *
 ****************************************************/

#define _TIMER_SEC	            0   // Temps en secondes (valeur définie à 0).
#define _TIMER_MIN 	            15  // Temps en minutes (15 minutes).
#define _TIMER_CYCLE_TO_SEC_CNT 250 // Nombre de cycles représentant 1 seconde.
#define _TIMER_SEC_TO_MIN_CNT 	60  // Nombre de secondes représentant 1 minute.

/**************************************************
 * DÉFINITIONS RELATIVES AUX I/O ET AUX CAPTEURS. *
 **************************************************/

// Macros pour l'initialisation et le contrôle des relais.
#define RELAY_INIT_IO()	(DDRB |= (1 << 1))
#define RELAY_SET(a) 	(PORTB = (PORTB & ~(1 << 1)) | ((a && 1) << 1))

// Macros pour l'initialisation et la lecture du capteur de mouvement.
// Retourne 1 si un mouvement est détecté, sinon 0.
#define CAPTEUR_MOUVEMENT_GET() 	(PINB & (1 << 0))
#define CAPTEUR_MOUVEMENT_INIT()	(PORTB |= (0x01))

// Macros pour l'initialisation et la lecture du capteur de porte.
// Retourne 1 si la porte est fermée, sinon 0.
#define CAPTEUR_PORTE_GET() 	(PINB & (1 << 6))
#define CAPTEUR_PORTE_INIT()	(PORTB |= (1 << 6))

#define DEL_INIT_IO() 			(DDRC |= (1 << 7))

/***********************************
 * DÉFINITIONS RELATIVES À LA DEL. *
 ***********************************/

// Intervalle entre chaque ajustement d'intensité de la DEL (5 * 4ms = 20ms).
#define DEL_FADE_INTERVAL 		5   
#define DEL_INCREMENT_INTENSITE	4
#define DEL_MAX_INTENSITE 		200
#define DEL_MIN_INTENSITE 		0


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

// Compteur de cycles pour 1 seconde (utilisé pour mesurer le temps).
volatile uint8_t compteur_cycliques_1s = 0;
// Compteur de secondes (utilisé pour mesurer le temps et gérer les délais).
volatile uint8_t compteur_secondes = 0;
// Compteur de minutes (utilisé pour mesurer le temps et gérer les délais).
volatile uint8_t compteur_minutes = 0;
// Indicateur de temps actif du relais (devient 1 lorsque le délai est atteint).
volatile uint8_t flag_temps_actif_relais = 0;
// Compteur pour le délai entre chaque ajustement d'intensité de la DEL.
volatile uint8_t compteur_delai_intensite_del = 0;
// Indicateur pour indiquer que le délai d'ajustement d'intensité de la DEL est atteint.
volatile uint8_t flag_delai_intensite_del_atteint = 0; // Varible qui vaut 1 lorsque le délai est atteint.

// État précédent du relais (initialisé à 1 par défaut).
uint8_t etat_precedent_relai = 1;


/***************************
 * PROTOTYPES DE FONCTIONS *
 ***************************/

/**
 * @brief  Initialise divers éléments du système.
 * 
 */
void init_divers(void);

/**
 * @brief  Initialise les composants matériels.
 * 
 */
void init_hardware(void);

/**
 * @brief 				  Éffectue un mécanisme de débouncing pour le signal donné.
 * 
 * @param etat 			  L'état actuel du signal.
 * @param etat_precedent  Pointeur vers l'adresse de l'état précédent du signal.
 * @return				  L'état débouncé du signal.
 */
uint8_t debounce(uint8_t etat, uint8_t *etat_precedent);

/**
 * @brief                 Ajuste l'intensité de la DEL en fonction de
 *                        la détection de mouvements et de la position de la porte.
 * 
 * @param etat_mouvement  Valeur lue par le capteur de mouvement.
 * @param etat_porte      Valeur lue par le capteur position de la porte.
 */
void ajuster_intensite_del(t_etat_mouvement etat_mouvement,
						   t_etat_porte etat_porte);

/**
 * @brief                 Gère l'état du relais en fonction de la détection de
 *                        mouvements et de la position de la porte.
 * 
 * @param etat_mouvement  Valeur lue par le capteur de mouvement.
 * @param etat_porte      Valeur lue par le capteur de position de la porte.
 */
void gestion_etat_relais(t_etat_mouvement etat_mouvement,
						 t_etat_porte etat_porte);

/**
 * @brief   Vérifie et renvoie l'état du mouvement.
 * 
 * @return  Valeur de t_etat_mouvement selon la 
 *          valeur lue par le capteur de mouvement.
 */
t_etat_mouvement verifier_etat_mouvement(void);

/**
 * @brief   Vérifie et renvoie l'état de la porte.
 * 
 * @return  Valeur de t_etat_porte selon la 
 *          valeur lue par le capteur de position de la porte.
 */
t_etat_porte verifier_etat_porte(void);

/**
 *@brief  Initialisation du timer #0.
 */
void init_timer0(void);

/**
 *@brief  Initialisation du timer #4.
 */
void init_timer4(void);

/**
 * @brief         Contraint une valeur donnée entre
 *                une valeur minimale et maximale.
 * 
 * @param valeur  La valeur à contraindre.
 * @param min     La valeur minimale.
 * @param max     La valeur maximale.
 * @return        La valeur contrainte.
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

        // Si le délai est atteint...
		if (flag_delai_intensite_del_atteint)
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
 *@brief  Interruption qui génère les intervales de temps pour
          fade le DEL et compter le délai avant que la relai ne soit plus actif.
 */
ISR(TIMER0_COMPA_vect)
{

	compteur_cycliques_1s++;

	if (compteur_secondes >= _TIMER_SEC)
	{

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

			//  Compteur remis à zéro à chaque minute.
			compteur_secondes -= _TIMER_SEC_TO_MIN_CNT;

			// Incrément du compteur de minutes.
			compteur_minutes++;
		}
	}

	compteur_delai_intensite_del++;

    // Chaques DEL_FADE_INTERVAL (20ms) la DEL augmente ou diminue d'intensité.
	if (compteur_delai_intensite_del >= DEL_FADE_INTERVAL)
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

        // Remet le compteur des minutes à 0 chaques fois que la porte est ouverte.
		RELAY_SET(1);
		compteur_secondes = 0;
		compteur_minutes = 0;
	}

    // Sinon, si la porte est fermée et qu'aucun mouvement n'est détecté...
	else if (etat_mouvement == MOUVEMENT_NON_DETECTE &&
			 etat_porte == PORTE_FERMEE)
	{

        // Si le délai est écoulé...
		if (flag_temps_actif_relais)
		{

			RELAY_SET(0); // Le relai est pas désactivé.
			flag_temps_actif_relais = 0;
		}
	}
}

t_etat_mouvement verifier_etat_mouvement(void)
{

	return CAPTEUR_MOUVEMENT_GET();
}

t_etat_porte verifier_etat_porte(void)
{

    // Appel de la fonction debounce afin d'éviter les erreurs de lecture.
	return debounce(CAPTEUR_PORTE_GET(), &etat_precedent_relai);
}

void init_timer0(void)
{

	// TCCR0A : COM0A1 COM0A0 COM0B1 COM0B0 – – WGM01 WGM00
	// TCCR0B : FOC0A FOC0B – – WGM02 CS02 CS01 CS00
	// TIMSK0 : – – – – – OCIE0B OCIE0A TOIE0
	TCCR0A |= (1 << WGM01);
	TCCR0B |= (1 << CS02);
	TIMSK0 |= (1 << OCIE0A);
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

    // Si la valeur est inférieur au seuil minimum...
	if (valeur <= min)
	{

        // Valeur retournée limitée au seuil minimum.
		resultat = min;
	}

    // Sinon, si la valeur est suppérieure au seuil maximum...
	else if (valeur >= max)
	{

        // Valeur retournée limitée au seuil maximum.
		resultat = max;
	}

    // Sinon (la valeur est valide)...
	else
	{

		resultat = valeur;
	}

	return resultat;
}

uint8_t debounce(uint8_t etat, uint8_t *etat_precedent)
{

	uint8_t valeur_debounce = 0;

    // Si la valeur actuellement lue est la même que celle précédemment lue...
	if (etat && (*etat_precedent))
	{
        
        // La valeur retourné par la fonction sera 1.
        valeur_debounce = 1;
    }

    // etat_precedent prend la valeur de l'état actuel.
	*etat_precedent = etat;

	return valeur_debounce;
}