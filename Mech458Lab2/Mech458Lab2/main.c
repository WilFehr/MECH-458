/*
 * Mech458Lab2.c
 *
 * Created: 2026-01-28 12:32:40 PM
 * Author : Wil Fehr
 
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

void nTimer(int count);//delays by count ms

int main(int argc, char* argv[])
{
	/*
	CLKPR- CLocK Pre-scale Register
	can lower system clock frequency 
	not exact, pg 47 on 
	*/
	
	//this halves the clock, makes it 8Mhz
	//must be first lines in all code
	CLKPR = 0x80;//clock change enable bit
	CLKPR = 0x01;// divide by 2, table 10-15
	
	/*
	TCCR- timer/counter control register
	_BV should set the bit to 1 
	and |= should turn that bit in the register to 1
	CS- Clock Select 
	select clock 1, bit 1 to 1, 010 (bits 2,1,0)
	010- /8 table 17-6
	*/
	TCCR1B |= _BV(CS11); //bitwise or and assignment table 17-6
	
	//data direction register C
	DDRC = 0xFF;
	PORTC = 0b00000011;
	
    /* Replace with your application code */
    while (1) 
    {
		
		for(int i = 0; i < 6; i++){
			nTimer(250);
			PORTC = PORTC << 1;
		}
		
		for(int i = 0; i < 6; i++){
			nTimer(250);
			PORTC = PORTC >> 1;
		}
		
		/*
		PORTL = 0b01000000;//set bit
		nTimer(1000);
		
		PORTL = 0b00010000;//turn on
		nTimer(1000);*/
			
    }//end of while
}//end of main

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
	
	
	//timer mask interrupt
	//only enables bit 1(value 2) to be read OCIE1A(interrupt enable) 
	TIMSK1 = TIMSK1 | 0b00000010;
	
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