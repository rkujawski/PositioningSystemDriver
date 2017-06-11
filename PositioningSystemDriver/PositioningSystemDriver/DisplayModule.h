/*
 * DisplayModule.h
 *
 * Created: 01.06.2017 20:58:52
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
 * |PC7 -						  | PC2 -				| PD0 - RS232 RxD0				|
 * |PC6 -						  | PC1 -				| PD1 - RS232 TxD0				|
 * |PC5 -						  | PC0 -				| PB5 -	SPI [MOSI]				|
 * |PC4 -						  | PD7 -				| PB7 - SPI [CLK]				|
 * |PC3 -						  |	PB3 - BUZZER		| PB4 - SPI [CS]				|
 * |______________________________|_____________________|_______________________________|	
 *
 *  RS232	- Main logic connection
 *  SPI		- 3x Max7219CNG 7-segment display driver (serialized x3)
 *  
 *	DisplayModule library.
 *  
 * RTFK @2017
 */ 

 #ifndef DISPPLAYMODULE_H
 #define DISPLAYMODULE_H


 //------MAX7219 REGISTERS---------------------------------
 #define MAX7219_NOOP		0x00
 #define MAX7219_DIG0		0x01
 #define MAX7219_DIG1		0x02
 #define MAX7219_DIG2		0x03
 #define MAX7219_DIG3		0x04
 #define MAX7219_DIG4		0x05
 #define MAX7219_DIG5		0x06
 #define MAX7219_DIG6		0x07
 #define MAX7219_DIG7		0x08
 #define MAX7219_DECODE		0x09
 #define MAX7219_INTENSITY	0x0A
 #define MAX7219_SCANLIMIT  0x0B
 #define MAX7219_SHUTDOWN	0x0C
 #define MAX7219_TEST		0x0F
 //-----MAX7219 SETTINGS----------------------------------
 #define SHUTDOWN					0x00
 #define NORMAL_OP					0x01
 #define TEST_ON					0x01
 #define TEST_OFF					0x00
 #define NO_DECODE_7_0				0x00
 #define NO_DECODE_0_DECODE_7_1		0x01
 #define NO_DECODE_7_4_DECODE_3_0	0x0F
 #define DECODE_7_0					0xFF
 #define DIGIT_TEST_TIME			100
 /*-----MAX7219 OTHER VALUES------------------------------
 #intensity register 0x00 - 0x0F
	0x00 is 1/32 intensity, 0x0F is 31/32 intensity
 #scanlimit register 0x00 - 0x07
	0x00 displays 0 digit only, 0x07 displays all digits

	 [0] - 0  - 0x00
	 .
	 .
	 [9] - 9  - 0x09
	 [-] - 10 - 0x0A
	 [E] - 11 - 0x0B
	 [H] - 12 - 0x0C
	 [L] - 13 - 0x0D
	 [P] - 14 - 0x0E
	 [ ] - 15 - 0x0F


 ---------------------------------------------------------*/
 #define SS_ON	PORTB &= ~(1 << PB4);
 #define SS_OFF PORTB |= (1 << PB4);

 void init_SPI(void);
 void send_SPI(uint8_t data);
 void set_Display_X(uint8_t reg, uint8_t data);
 void set_Display_Y(uint8_t reg, uint8_t data);
 void set_Display_Z(uint8_t reg, uint8_t data);
 void init_Display_X(void);
 void init_Display_Y(void);
 void init_Display_Z(void);
 void display_Number(uint8_t axis, uint32_t value);

 void init_SPI(void)
{
	DDRB |= (1 << PB4) | (1 << PB5)	| (1 << PB7);
	SPCR |= (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0); // DORD = 0 | CPOL = 0 | CPHA = 0 
	/* 
	   DDRB - PORT B I/O REGISTER
	   > MOSI, SCK, CS as outputs
	   SPCR - SPI Control REGISTER
	   > SPE		- SPI ENABLE,				1 - ENABLE, 0 - DISABLE
	   > MSTR		- MASTER/SLAVE SELECT,		1 - MASTER, 0 - SLAVE 
	   > SPR1, SPR0 - SCK(CLK) SPEED SELECT,		fOSC / 4, 16, 64, 128
	   > DORD		- DATA VECTOR ORDER 	    1 - LSB, 0* - MSB
	   > CPOL		- SCK(CLK) POLARIZATION		1 - IDLE HIGH, 0 - IDLE LOW
	   > CPHA		- SCK(CLK) PHASE			1 - FALLING, 0 - RISING
    */
}

void send_SPI(uint8_t data)
{
	SPDR =  data;
	while(!(SPSR & (1<<SPIF) ));
}

void set_Display_X(uint8_t reg, uint8_t data)
{
	SS_ON						// CS/SS low
		send_SPI(MAX7219_NOOP);  
		send_SPI(MAX7219_NOOP); // ignore 3rd chip
		send_SPI(MAX7219_NOOP);
		send_SPI(MAX7219_NOOP); // ignore 2nd chip
	send_SPI(reg);
	send_SPI(data);
	SS_OFF					    // CS/SS high
}

void set_Display_Y(uint8_t reg, uint8_t data)
{
	SS_ON
		send_SPI(MAX7219_NOOP);
		send_SPI(MAX7219_NOOP); // ignore 3rd chip
	send_SPI(reg);
	send_SPI(data);
		send_SPI(MAX7219_NOOP);
		send_SPI(MAX7219_NOOP); // ignore 1rd chip
	SS_OFF
}

void set_Display_Z(uint8_t reg, uint8_t data)
{
	SS_ON
	send_SPI(reg);
	send_SPI(data);
	send_SPI(MAX7219_NOOP);
	send_SPI(MAX7219_NOOP); // ignore 2nd chip
	send_SPI(MAX7219_NOOP);
	send_SPI(MAX7219_NOOP); // ignore 1st chip
	SS_OFF
}

void init_Display_X(void)
{
	set_Display_X(MAX7219_DECODE, DECODE_7_0);
	set_Display_X(MAX7219_SCANLIMIT, 5);
	set_Display_X(MAX7219_INTENSITY, 0x0F);
	set_Display_X(MAX7219_SHUTDOWN, NORMAL_OP);
	
	for(int i = 1; i < 7; i++)
	{
		set_Display_X(i, 8);
		_delay_ms(DIGIT_TEST_TIME);
		set_Display_X(i, 0x0F);
	}
}

void init_Display_Y(void)
{
	set_Display_Y(MAX7219_DECODE, DECODE_7_0);
	set_Display_Y(MAX7219_SCANLIMIT, 5);
	set_Display_Y(MAX7219_INTENSITY, 0x0F);
	set_Display_Y(MAX7219_SHUTDOWN, NORMAL_OP);
	
	for(int i = 1; i < 7; i++)
	{
		set_Display_Y(i, 8);
		_delay_ms(DIGIT_TEST_TIME);
		set_Display_Y(i, 0x0F);
	}
}

void init_Display_Z(void)
{
	set_Display_Z(MAX7219_DECODE, DECODE_7_0);
	set_Display_Z(MAX7219_SCANLIMIT, 5);
	set_Display_Z(MAX7219_INTENSITY, 0x0F);
	set_Display_Z(MAX7219_SHUTDOWN, NORMAL_OP);
	
	for(int i = 1; i < 7; i++)
	{
		set_Display_Z(i, 8);
		_delay_ms(DIGIT_TEST_TIME);
		set_Display_Z(i, 0x0F);
	}
}

void display_Number(uint8_t axis, uint32_t value)
{
    uint8_t flag_Zero = 1;

	uint32_t tab[6];
	tab[0] = value / 100000;
	value -= tab[0]*100000;
	tab[1] = value / 10000;
	value -= tab[1]*10000;
	tab[2] = value / 1000;
	value -= tab[2]*1000;
	tab[3] = value / 100;
	value -= tab[3]*100;
	tab[4] = value / 10;
	value -= tab[4]*10;
	tab[5] = value;

	tab[2] += 128;
	for(int i = 1; i < 7; i++)
	{	
		if(tab[i-1] != 0) flag_Zero = 0;
		if(tab[i-1] == 0 && flag_Zero == 1) tab[i-1] = 0x0F;
		switch(axis)
		{
			case(0):
				set_Display_X(i, tab[i-1]);
				break;
			case(1):
				set_Display_Y(i, tab[i-1]);
				break;
			case(2):
				set_Display_Z(i, tab[i-1]);
				break;
		}

	}

}



#endif