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
 
 # Created: 2026-01-20 4:21:12 PM
 //limited library ask before using
 //compiler
 */ 

//includes
#include <avr/io.h>
#include <stdlib.h>


/*--------------MAIN LOOP-----------------*/
int main(int argc, char* argv[])
{
	//DDRL is defined in iom2560, it appears to be a memory address
	//DDR data direction register
	DDRL = 0b11111111;//sets all pins on port L to output
	//Port L is also defined in iom2560.h, also is a memory address
	PORTL = 0b10100000; //sets pin0 and pin2 as high
	
	return(0);
}

