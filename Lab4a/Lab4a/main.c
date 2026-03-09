/*
 * Lab4a.c
 *
 * Created: 2026-02-11 2:20:58 PM
 * Author : 
 */ 

/* include libraries */
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>//comment out

void turnCW(int steps);
void turnCCW(int steps);
void stepper_init();
void nTimer(int count);

/*
   bit 5 | bit 4| bit 3| bit 2| bit 1| bit 0|
_____E0__|__L1__|__L2__|__E1__|__L3__|__L4__|
1-|  1   |   1  |  0   |  0   |  x   |  x   |
2-|  0   |   x  |  x   |  1   |  1   |  0   |
3-|  1   |   0  |  1   |  0   |  x   |  x   |
4-|  0   |   x  |  x   |  1   |  0   |  1   |

*/
//pad front with 00(may have to pad the end)
//assumed order 76543210
char steps_arr[4] = {0b00110000, 0b00000110, 0b00101000, 0b00000101};

/*defines*/
//stepper motor pins change as needed
#define en0 PA5
#define en1	PA2
#define L1 PA4
#define L2 PA3
#define L3 PA1
#define L4 PA0

/*global*/
//state
//current step
volatile int curposition = 0;



int main(void)
{
	//clock stuff comment out
	/*
	CLKPR- CLocK Pre-scale Register
	can lower system clock frequency 
	not exact, pg 47 on 
	*/
	
	//this halves the clock, makes it 8Mhz
	//must be first lines in all code
	CLKPR = 0x80;//clock change enable bit
	CLKPR = 0x01;// divide by 2, table 10-15(prescale)
	
	/*
	TCCR- timer/counter control register
	_BV should set the bit to 1 
	and |= should turn that bit in the register to 1
	CS- Clock Select 
	select clock 1, bit 1 to 1, 010 (TCCR bits 2,1,0)
	010- /8 table 17-6
	*/
	TCCR1B |= _BV(CS11); //bitwise or and assignment table 17-6
	
	
	//DATA DIRECTION REGISTER CONFIG
	DDRA = 0xFF;
	DDRB = 0xFF;//
	
	
	//set up port B for fast PWM, fast pwm WGM0,WGM1 all high
	//com0a1, is clear OC0A on compare match, (table 16-2)
	TCCR0A |= 0b10000011;//sets both WGM01 and WGM00 to one also set COM0A1 to one,
	
	
	//timer mask interrupt
	//only enables bit 1(value 2) to be read OCIE1A(interrupt enable)
	TIMSK0 = TIMSK0 | 0b00000010;
	
	//Timer counter 2
	TCCR0B |= 0b00000010; //divide clock by 8(still too big) makes it 3.9kHz, 64 0b00000011
	
	//output compare register
	//8 bit count this is duty cycle
	OCR0A = 0x80;//128
	
	
	
	
	
	
	
	turnCW(90);
	//3sec
	nTimer(3000);
	//17 step
	turnCW(17);
	//2 sec
	nTimer(2000);
	//33 step
	turnCW(33);
	//2 sec
	nTimer(2000);
	//100 steps
	turnCW(100);
	//then ccw in same ammount order
	//2 sec
	nTimer(2000);
	//17 step
	turnCCW(17);
	//2 sec
	nTimer(2000);
	//33 step
	turnCCW(33);
	//2 sec
	nTimer(2000);
	//100 steps
	turnCCW(100);
    /* Replace with your application code */
    while (1) 
    {
    }
}




void turnCW(int steps){
	for(int i =0; i < steps; i++){
		/*
		if(++curposition > 3){
			curpos
		}
		*/
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

void stepper_init(){
	turnCW(90);
	return;
}



void turnCCW(int steps){
		for(int i =0; i < steps; i++){
			//increment curposition(can use ternary and assignment) and
			curposition--;
			//test if valid
			if(curposition < 0 ){
				curposition = 3;
			}
			//output A to current step
			PORTA = steps_arr[curposition];
			//delay 20ms
			nTimer(20);
			
		}
		
		return;
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
