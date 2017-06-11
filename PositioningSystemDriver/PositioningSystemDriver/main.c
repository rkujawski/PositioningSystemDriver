/*
 * PositioningSystemDriver.c
 *
 * Created: 30.05.2017 21:13:52
 * Author : Robert Kujawski
 *
 * Pin configuration ATMEGA 644
 *  ____________________________________________________________________________________
 * |PD2 - Encoder 1. interruption | PD4 - End-switch X	| PA0 - Encoder 1. signal A		|
 * |PD3 - Encoder 2. interruption | PD5 - End-switch Y	| PA1 - Encoder 1. signal B		|
 * |PB2 - Encoder 3. interruption | PD6 - End-switch Z	| PA2 - Encoder 2. signal A		|
 * |	   						  |						| PA3 - Encoder 2. signal B		|
 * |							  |						| PA4 - Encoder 3. signal A		|
 * |							  |						| PA5 - Encoder 3. signal B		|
 * |______________________________|_____________________|_______________________________|
 * |PC7 - REFERENCE switch		  | PC2 - Z0. button	| PD0 - RS232 RxD0				|
 * |PC6 - RELATIVE button		  | PC1 - RED LED		| PD1 - RS232 TxD0				|
 * |PC5 - ABSOLUTE button		  | PC0 - GREEN LED		| PB5 -	SPI [MOSI]				|
 * |PC4 - X0. button			  | PD7 - N.C.			| PB7 - SPI [CLK]				|
 * |PC3 - Y0. button			  |	PB3 - BUZZER		| PB4 - SPI [CS]				|
 * |______________________________|_____________________|_______________________________|	
 *
 *  RS232	- Main logic connection
 *  SPI		- 3x Max7219CNG 7-segment display driver (serialized x3)
 * 
 * 
 *                           .-----------------TTTT_-----______
 *                        /''''''''''(______O] ----------____  \______/]_	
 *     __...---'"""\_ --''   Q                               ___________@    ASM>>		 ASM>>			ASM>>		ASM>>>
 * |'''                   ._   _______________=---------"""""""	
 * |                ..--''|   l L |_l   |
 * |          ..--''      .  /-___j '   '
 * |    ..--''           /  ,       '   '
 * |--''                /           `    \
 *                      L__'         \    -
 *                                    -    '-.
 *                                     '.    /
 *                                       '-./
 * AK74 Code Development Techniques
 * RTFK @2017
 */ 
#define  F_CPU 20000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "DisplayModule.h"

//***************************************
//*     ASM PORT OPRATIONS MACROS       *
//***************************************
	#define SBI(port,bit) 	asm("sbi %0, %1" : : "I" (_SFR_IO_ADDR(port)), "I" (bit))
	#define CBI(port,bit) 	asm("cbi %0, %1" : : "I" (_SFR_IO_ADDR(port)), "I" (bit))

	#define Sp(wrd,bit) 	CBI(wrd,bit)	//set bit in port
	#define Rp(wrd,bit) 	SBI(wrd,bit)	// reset bit in port
	#define Tp(wrd,bit) 	wrd ^= (1<<bit) //toogle  bit in port

#define REF		PC7
#define INT_REF PCINT23
#define REL		PC6
#define INT_REL PCINT22
#define ABS		PC5
#define INT_ABS PCINT21
#define X_BTN	PC4
#define INT_X	PCINT20
#define Y_BTN	PC3
#define INT_Y	PCINT19
#define Z_BTN	PC2
#define INT_Z	PCINT18
#define GREEN	PC0
#define RED		PC1
#define axis_X 0
#define axis_Y 1
#define axis_Z 2

#define RELATIVE 1
#define ABSOLUTE 0

/* ABSOLUTE COORDINATES */
uint32_t ABS_X = 700200;
uint32_t ABS_Y = 500000;
uint32_t ABS_Z = 1200;
/* RELATIVE COORDINATES */
uint32_t ZERO_X = 0;
uint32_t ZERO_Y = 0;
uint32_t ZERO_Z = 0;
int32_t  REL_X = 0;
int32_t	 REL_Y = 0;
int32_t  REL_Z = 0;
/*--------------------- */
uint8_t POS_MODE = 0; // 0 - ABS, 1 - REL

void init_Interrupts(void);
void update_Display(uint8_t mode);
void update_Relative(void);

int main(void)
{

	
	DDRC |= (1 << GREEN) | (1 << RED) ;   //PC7-PC2 as inputs, PC1-PC0 as output
	PORTC = 0xFF;  //PC7-PC2 pull-up active, LEDs ON

	init_SPI();
	init_Interrupts();
	init_Display_X();
	init_Display_Y();
	init_Display_Z();

	
	sei();
	while(1)
	{
		
		update_Display(POS_MODE);
		_delay_ms(100);
	}

	
}
void init_Interrupts(void)
{
	PCMSK2 |= (1 << INT_REF) | (1 << INT_REL) | (1 << INT_ABS) | (1 << INT_X) | (1 << INT_Y) | (1 << INT_Z);
	PCICR  |= (1 << PCIE2);
	PCIFR  |= (1 << PCIE2);
}

ISR(PCINT2_vect)
{

		/* There was a fucking problem with that, because of fucking JTAG 4 fucking PINS are in domain Out Of Service! */
		if (!(PINC & (1 << ABS))) POS_MODE = 0;
		
		if (!(PINC & (1 << REL))) 
		{
			POS_MODE = 1;
			REL_X = ABS_X;
			REL_Y = ABS_Y;
			REL_Z = ABS_Z;
		}
	
		if (!(PINC & (1 << X_BTN))) 
		{
			ZERO_X = ABS_X;
			POS_MODE = 1;
		}

		if (!(PINC & (1 << Y_BTN))) 
		{
			ZERO_Y = ABS_Y;
			POS_MODE = 1;
		}

		if (!(PINC & (1 << Z_BTN))) 
		{ 
			ZERO_Z = ABS_Z;
			POS_MODE = 1;
		}
		_delay_ms(50);


}

void update_Display(uint8_t mode)
{
	switch(mode)
	{
		case 0:

		display_Number(axis_X, ABS_X);
		display_Number(axis_Y, ABS_Y);
		display_Number(axis_Z, ABS_Z);
		break;

		case 1:
		update_Relative();
		display_Number(axis_X, REL_X);
		display_Number(axis_Y, REL_Y);
		display_Number(axis_Z, REL_Z);
		break;
	}


}

void update_Relative(void)
{
	REL_X = ABS_X - ZERO_X;
	REL_Y = ABS_Y - ZERO_Y;
	REL_Z = ABS_Z - ZERO_Z;
}