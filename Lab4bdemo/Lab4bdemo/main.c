/*
 * Lab4bdemo.c
 *
 * Created: 2026-02-17 2:53:28 PM
 * Author : kazoo
 */ 

#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include "lcd.h"

// define the global variables that can be used in every function ==========
volatile unsigned char ADC_result;
volatile unsigned int ADC_result_flag;
volatile unsigned char change_dir;
volatile unsigned char killed = 0;

void nTimer(int count);

void main()
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
	
	
	//configure ports
	DDRL = 0xff;
	DDRB = 0xff;
	DDRC = 0xff;
	//port F has the ADC
	
	/*
	TCCR- timer/counter control register
	_BV should set the bit to 1 
	and |= should turn that bit in the register to 1
	CS- Clock Select 
	select clock 1, bit 1 to 1, 010 (bits 2,1,0)
	010- /8 table 17-6
	*/
	TCCR1B |= _BV(CS11); //bitwise or and assignment table 17-6
	
/****************            INTERRUPT CONFIG                      *****************/
	cli(); // disable all of the interrupt ==========================
	
	
	// config the external interrupt ======================================
	//table15-1
	EIMSK |= (_BV(INT2)); // enable INT2
	EICRA |= _BV(ISC21); // falling edge interrupt
	EICRA &= ~_BV(ISC20); // falling edge interrupt

	EIMSK |= (_BV(INT3)); // enable INT3
	EICRA |= _BV(ISC31); // rising edge interrupt
	EICRA |= _BV(ISC30); // rising edge interrupt
			// config ADC =========================================================
	
		// by default, the ADC input (analog input is set to be ADC0 / PORTF0
	ADCSRA |= _BV(ADEN);		// enable ADC
	ADCSRA |= _BV(ADIE);		// enable interrupt of ADC
	
	//ADLAR- analog digital Left Adjust register
	//REFS0- reference selection(turn on capacitor at AVCC 
	ADMUX |= _BV(ADLAR) | _BV(REFS0); // Read Technical Manual & Complete Comment
	
	// set the PORTC as output to display the ADC result ==================
	
	
		//Initialize LCD module
		InitLCD(LS_BLINK|LS_ULINE);

		//Clear the screen
		LCDClear();

		//Simple string printing
		LCDWriteString("Congrats ");

	
	// sets the Global Enable for all interrupts ==========================
	sei();
	
	// initialize the ADC, start one conversion at the beginning ==========
	ADCSRA |= _BV(ADSC);//analog digital start one conversion
	
/***********                  END INTERRUPT CONFIG                      *********/
	
	
/****************           PWM INIT             **************************/
	//set up port B for fast PWM, fast pwm WGM0,WGM1 all high
	//com0a1, is clear OC0A on compare match, (table 16-2)
	TCCR0A |= 0b10000011;//sets both WGM01 and WGM00 to one also set COM0A1 to one,
	
	
	//timer mask interrupt
	//only enables bit 1(value 2) to be read OCIE1A(interrupt enable)
	//TIMSK0 = TIMSK0 | 0b00000010;
	
	//Timer counter 2
	TCCR0B |= 0b00000011; //divide clock by 8(still too big) makes it 3.9kHz, 64 0b00000011
	
	//output compare register
	//8 bit count this is duty cycle
	OCR0A = 0x80;//128
/****************          END PWM INIT             **************************/	
	PORTB = 0b00001101;
	
	while (1)
	{	
		/*if(killed){
			PORTB |= 0b00001111;
			while(1);
		}*/
		
		
		
		
		if (ADC_result_flag)
		{
			
			//PORTC = ADC_result;
			
			OCR0A = ADC_result;
			
			ADC_result_flag = 0x00;
			
			//PORTL = 0x08;
			//nTimer(100);
			ADCSRA |= _BV(ADSC);//analog digital start one conversion
			
		}
		
	}
} // end main





// the interrupt will be trigured if the ADC is done ========================
ISR(ADC_vect)
{
	ADC_result = ADCH;
	ADC_result_flag = 1;
	
	//PORTL = 0x01;
	//nTimer(100);
}

//int2 kill switch
ISR(INT2_vect){//on int2 falling edge(active low)
	//write 
	//disable interrupts
	//cli();
	//PORTL = 0x05;
	//brake to vcc
	PORTB |= 0b00001111;
	//nTimer(25);
	while(1);
	//killed = 1;
	
	
	
	
}

//int3 change direction
ISR(INT3_vect){//on rising edge(active high)
	if((PIND & 0x08) == 0x08){
		//debounce
		nTimer(25);
	
	
		//PORTL = 0x03;
		//nTimer(100);
		////store inverse of current direction
		change_dir = PORTB ^ 0b00001010;
		////turn on in a and in b(break to Vcc)
		PORTB |= 0b00001111;
		nTimer(5);
		PORTB = change_dir;
	
		//debounce
		while((PIND & 0x08) == 0x08);//loop while PD3 is high
		nTimer(25);
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
	
	
	//timer mask interrupt
	//only enables bit 1(value 2) to be read OCIE1A(interrupt enable) 
	//TIMSK1 = TIMSK1 | 0b00000010;
	
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