/*
* ----------------------------------------------------------------------------
* “THE COFFEEWARE LICENSE” (Revision 1):
* <ihsan@kehribar.me> wrote this file. As long as you retain this notice you
* can do whatever you want with this stuff. If we meet some day, and you think
* this stuff is worth it, you can buy me a coffee in return.
* -----------------------------------------------------------------------------
* Please define your platform spesific functions in this file ...
* -----------------------------------------------------------------------------
*/

#include <avr/io.h>

#define set_bit(reg,bit) reg |= (1<<bit)
#define clr_bit(reg,bit) reg &= ~(1<<bit)
#define check_bit(reg,bit) (reg&(1<<bit))

/* ------------------------------------------------------------------------- */
void nrf24_setupPins()
{
	set_bit(DDRD,3); // CE output
	set_bit(DDRB,1); // CSN output
	set_bit(DDRD,4); // SCK output
	set_bit(DDRD,6); // MOSI output
	clr_bit(DDRD,5); // MISO input
}
/* ------------------------------------------------------------------------- */
void nrf24_ce_digitalWrite(uint8_t state)
{
	if(state)
	{
		set_bit(PORTD,3);
	}
	else
	{
		clr_bit(PORTD,3);
	}
}
/* ------------------------------------------------------------------------- */
void nrf24_csn_digitalWrite(uint8_t state)
{
	if(state)
	{
		set_bit(PORTB,1);
	}
	else
	{
		clr_bit(PORTB,1);
	}
}
/* ------------------------------------------------------------------------- */
void nrf24_sck_digitalWrite(uint8_t state)
{
	if(state)
	{
		set_bit(PORTD,4);
	}
	else
	{
		clr_bit(PORTD,4);
	}
}
/* ------------------------------------------------------------------------- */
void nrf24_mosi_digitalWrite(uint8_t state)
{
	if(state)
	{
		set_bit(PORTD,6);
	}
	else
	{
		clr_bit(PORTD,6);
	}
}
/* ------------------------------------------------------------------------- */
uint8_t nrf24_miso_digitalRead()
{
	return check_bit(PIND,5);
}
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- 
void nrf24_setupPins()
{
	set_bit(DDRA,3); // CE output
	set_bit(DDRB,1); // CSN output
	set_bit(DDRA,4); // SCK output
	set_bit(DDRA,6); // MOSI output
	clr_bit(DDRA,5); // MISO input
}
/* ------------------------------------------------------------------------- *
void nrf24_ce_digitalWrite(uint8_t state)
{
	if(state)
	{
		set_bit(PORTA,3);
	}
	else
	{
		clr_bit(PORTA,3);
	}
}
/* ------------------------------------------------------------------------- *
void nrf24_csn_digitalWrite(uint8_t state)
{
	if(state)
	{
		set_bit(PORTB,1);
	}
	else
	{
		clr_bit(PORTB,1);
	}
}
/* ------------------------------------------------------------------------- *
void nrf24_sck_digitalWrite(uint8_t state)
{
	if(state)
	{
		set_bit(PORTA,4);
	}
	else
	{
		clr_bit(PORTA,4);
	}
}
/* ------------------------------------------------------------------------- *
void nrf24_mosi_digitalWrite(uint8_t state)
{
	if(state)
	{
		set_bit(PORTA,6);
	}
	else
	{
		clr_bit(PORTA,6);
	}
}
/* ------------------------------------------------------------------------- *
uint8_t nrf24_miso_digitalRead()
{
	return check_bit(PINA,5);
}
/* ------------------------------------------------------------------------- */
