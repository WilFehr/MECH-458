/*
 * Lab5WorkingCopy1.c
 *
 * Created: 2026-03-04 4:10:42 PM
 * Author : kazoo
 */ 


/*******************  START OF INCLUDES   *********************/
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include "lcd.h"
#include <stdint.h>

/*******************  END OF INCLUDES  ************************/



/*******************    START OF DEFINES     *****************/

/******************    END OF DEFINES     *******************/



/****************    START OF GLOBAL VARIABLES   ***************/
volatile uint16_t ADC_result;//16-bit result should be able to hold 10 bit value
volatile unsigned int ADC_result_flag;
volatile unsigned char change_dir;
volatile unsigned char direction = 0;
volatile unsigned char killed = 0;
volatile char STATE;
volatile int curposition = 0b00110000;

	//constants
const char steps_arr[4] = {0b00110000, 0b00000110, 0b00101000, 0b00000101};
/****************    END OF GLOBAL VARIABLES   *****************/




/***********    START OF FUNCTION DECLARATIONS   ***************/
void nTimer(int count);
void home_stepper();
void turnCW90();
void turnCCW90();
/***********    END OF FUCTION DECLARATIONS   ***************/


/**********************************OVERVIEW************************



                                                       
************************************************************************/

int main(void)
{
    CLKPR = 0x80;
    CLKPR = 0x01;		//  sets system clock to 8MHz
	
	/*
	pre-scale TCCR1 to 1/8th
	*/
	//TCCR1,3,4, and 5 are 16 bit 
	TCCR1B |= _BV(CS11); //bitwise or and assignment table 17-6

    STATE = 0;

    cli();		// Disables all interrupts

	/******* PORT USAGE********/
	//PORTA
	//STEPPER MOTOR
	DDRA = 0xFF;
	
	//PORTB
	//DCMOTOR
	//DDRB = 0b10001111;
	
	//PORTC
	//DISPLAY ON PORT C
	DDRC = 0xFF;
	
	//PORTD
	//set bits 3-0 as inputs
	//DDRD = 0xF0;
	
	//PORTE
	
	
	//PORTF
	//ADC IS ON PORT F
	
	//PORTL
	//bit 7 is Hall effect
	DDRL = 0b00000000;
	
	/***** END PORT USAGE *****/
	
	
	/****** *******/ 
	/****** ******/
	//Initialize LCD module
	InitLCD(LS_BLINK|LS_ULINE);
	LCDClear();
	
	
	home_stepper();
	LCDWriteInt(curposition, 3);
	nTimer(500);
	LCDClear();
	turnCW90();

	
	LCDWriteInt(curposition, 3);
	nTimer(500);
	LCDClear();
	turnCCW90();

	
	LCDWriteInt(curposition, 3);
	nTimer(500);
	LCDClear();
	turnCW90();
	
	
	LCDWriteInt(curposition, 3);
	nTimer(500);
	LCDClear();
	turnCW90();
	
	LCDWriteInt(curposition, 3);
    while (1) 
    {
    }
}

void nTimer(int count){
	//variables
	int i = 0;//loop counter
	
	/* 
	sets the Waveform Generator Mode in 
	tccr1(broken into a and b) to 0100(bits3,2,1,0)
	mode when a ocr and tcnt match tcnt resets
	wgm 010 is clear counter when comparison match (ocr = counter) and turn on
	compare match does trigger interrupt flag
	*/
	TCCR1B |= _BV(WGM12);//pg 128 table 16-8
	
	//output compare register
	//16 bit clock
	OCR1A = 0x03E8;//1000 clock cycles(clock is 1MHZ, so each cycle is 1us) - 1ms
	
	//initialize timer counter
	TCNT1 = 0x0000;
	
	
	
	
	while(i < count){
		/*
		TIFR
		Timer interrupt flag register
		holds all interrupt flags
		gets triggered when the counter gets to 1000(1ms)
		*/
		//checks if interrupt is there
		if((TIFR1 & 0x02) == 0x02){
			
			/*
			output compare flag triggers an interrupt in wgm 010
			
			
			*/
			TIFR1 |= _BV(OCF1A);//sets output compare flag to 1, which triggers a counter reset
			
			i++;//iterate counter
			
		}//stop checking 
	}//end while i < count
	
	
	return;
}//end nTimer

/*
void turnCW(int steps){
	for(int i =0; i < steps; i++){
		//increment current position
		curposition++;
		//test if valid
		if(curposition > 3 ){
			curposition = 0;
		}
		//output A to current step
		PORTA = steps_arr[curposition];
		//delay 20ms
		nTimer(20);
		
	}//end for
	
	return;
}//end cw
*/


//int steps_to_stop = 

void home_stepper(){
	while((PINL & 0b10000000) == 0b10000000 ){//Sensor_ex != 1;
		//increment current position
		curposition++;
		//test if valid
		if(curposition > 3 ){
			curposition = 0;
		}
		//output A to current step
		PORTA = steps_arr[curposition];
		//delay 20ms
		nTimer(20);
	}
}

//trap accel
void turnCW90(){
	for(int i =0; i < 50; i++){
		//increment current position
		curposition++;
		//test if valid
		if(curposition > 3 ){
			curposition = 0;
		}
		//output A to current step
		PORTA = steps_arr[curposition];
		
		int delay = ((i<10)?(20-(1+i)/2):10);
		delay = ((i>39)?(i-30):(delay));
		//delay 20ms-
		nTimer(delay);
		
	}//end for
	
	return;
}//end cw


void turnCCW90(){
	for(int i =0; i < 50; i++){
		//increment curposition(can use ternary and assignment) and
		curposition--;
		//test if valid
		if(curposition < 0 ){
			curposition = 3;
		}
		//output A to current step
		PORTA = steps_arr[curposition];
		//delay 20ms
		
		int delay = ((i<10)?(20-(1+i)/2):10);
		delay = ((i>39)?(i-30):(delay));
		//delay 20ms-
		nTimer(delay);
		
	}
	
	return;
}