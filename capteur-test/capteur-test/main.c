/*
 * capteur-test.c
 *
 * Created: 2021-06-07 16:57:52
 * Author : thomas
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

#define DEL_CAPTEUR_INIT()	(DDRC |= (1<<7))
#define DEL_CAPTEUR_SET(a)	(PORTC = (PORTC & ~(1<<7)) | ((a && 1) << 7)) //Définition de l'état de la broche de sortie en fonction de la valeur reçue (0 = bas | 1 = haut).
#define CAPTEUR_INIT()		(PORTB |= (1<<0))
#define CAPTEUR()			(PINB & (1<<0))

//Prototypes des fonctions locales
/**
*@brief  Fonction qui regroupe l'initialisation des différents I/O et des librairies.
*/
void miscInit(void);

int main(void)
{
    miscInit();
    while (1) 
    {
		DEL_CAPTEUR_SET(CAPTEUR()); //La LED allume ou s'éteint en fonction d'une detection ou non d'un mouvement.
    }
}

void miscInit(void)
{
	DEL_CAPTEUR_INIT();
	//CAPTEUR_INIT();
}