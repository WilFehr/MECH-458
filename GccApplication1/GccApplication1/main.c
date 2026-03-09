/*#########################
 # MILESTONE: 1
 # PROGRAM: 1
 # PROJECT: Lab1Demo
 # GROUP: 5
 # LAB SECTION: B02
 # NAME1: Asher Barnsdale
 # NAME2: Wil Fehr
 # DESC: 
 # DATA
 # REVISED:
 
 # Created: 2026-01-21
 //limited library ask before using
 //compiler
 */ 

//includes
#include <avr/io.h>
#include <stdlib.h>
#include <util/delay_basic.h>


void delaynus(int n) // delay microsecond
{
	int k;
	for(k=0;k<n;k++)
	_delay_loop_1(1);
}
void delaynms(int n) // delay millisecond
{
	int k;
	for(k=0;k<n;k++)
	delaynus(1000);
}

/*--------------MAIN LOOP-----------------*/
int main(int argc, char* argv[])
{
	//DDRC is defined in iom2560, it appears to be a memory address
	//DDR data direction register
	DDRC = 0b11111111;//sets all pins on port l to output
	//Port C is also defined in iom2560.h, also is a memory address
	PORTC = 0xFF; //sets all pins as high
	
	while(1){
		PORTC = 0b11000000;//xx-- ----
		delaynms(250);
		PORTC = 0b11100000;//xxx- ----
		delaynms(250);
		PORTC = 0b11110000;//xxxx ----
		delaynms(250);
		PORTC = 0b01111000;//-xxx x---
		delaynms(250);
		PORTC = 0b00111100;//--xx xx--
		delaynms(250);
		PORTC = 0b00011110;//---x xxx-
		delaynms(250);
		PORTC = 0b00001111;//---- xxxx
		delaynms(250);
		PORTC = 0b00000111;//---- -xxx
		delaynms(250);
		
		PORTC = 0b00000011;//---- --xx
		delaynms(250);
		PORTC = 0b00000111;//---- -xxx
		delaynms(250);
		PORTC = 0b00001111;//---- xxxx
		delaynms(250);
		PORTC = 0b00011110;//---x xxx-
		delaynms(250);
		PORTC = 0b00111100;//--xx xx--
		delaynms(250);
		PORTC = 0b01111000;//-xxx x---
		delaynms(250);
		PORTC = 0b11110000;//xxx- ----
		delaynms(250);
		PORTC = 0b11100000;//xx-- ----
		delaynms(250);
		
	}
	
	
	
	return(0);
}

