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
#include "LinkedQueue.h"
#include <stdint.h>

/*******************  END OF INCLUDES  ************************/



/*******************    START OF DEFINES     *****************/
//stepper states
#define BLACK 0
#define STEEL 1
#define WHITE 2
#define ALUM 3

//state machine states
#define polling 0
#define reflective 1
#define drop_result 2
#define pause 3
#define ramp_down 4

/******************    END OF DEFINES     *******************/



/****************    START OF GLOBAL VARIABLES   ***************/
volatile uint16_t ADC_result;//16-bit result should be able to hold 10 bit value
volatile unsigned int ADC_result_flag = 0;
volatile unsigned char change_dir;
volatile unsigned char paused_check = 1;
volatile unsigned char direction = 0;
volatile unsigned char killed = 0;
volatile char STATE;
volatile int curposition = 0b00110000;
volatile uint16_t curmaterialmin;
volatile int reflect_count = 0;
volatile unsigned int ramped_down = 0;
volatile uint8_t timer_running = 0;


	//constants
const char steps_arr[4] = {
	0b00110110,  // A+, B+
	0b00101110,  // A-, B+
	0b00101101,  // A-, B-
	0b00110101   // A+, B-
};
	
	//non-volatile
int delay = 10;//ms delay
char curMode;
link* headptr;
link* tailptr;
link* newlink;
link *rtnlink;		
element eTest;
char sorted[5] = {0, 0, 0, 0, 0};//Black, Steel, White, Alum, between sensor
/****************    END OF GLOBAL VARIABLES   *****************/




/***********    START OF FUNCTION DECLARATIONS   ***************/
void nTimer(int count);
void home_stepper();//home in on black
void turnCW90();//turn stepper
void turnCCW90();//turn stepper
void turnCW180();//turn stepper
void turnCCW180();//turn stepper
/***********    END OF FUCTION DECLARATIONS   ***************/


/**********************************OVERVIEW************************



                                                       
************************************************************************/

int main(void)
{

/************* CLOCK SET UP ***************/
    CLKPR = 0x80;
    CLKPR = 0x01;		//  sets system clock to 8MHz
	
	
	//pre-scale TCCR1 to 1/8th
	//TCCR1,3,4, and 5 are 16 bit 
	TCCR1B |= _BV(CS11); //bitwise or and assignment table 17-6
/****** END CLOCK SET UP ***********/


    cli();		// Disables all interrupts


/***************** PORT USAGE*****************/
	//PORTA
	//STEPPER MOTOR
	DDRA = 0xFF;
	
	//PORTB
	//DCMOTOR
	DDRB = 0b10001111;
	
	//PORTC
	//DISPLAY ON PORT C
	DDRC = 0xFF;
	
	//PORTD
	//set bits 3-0 as inputs
	DDRD = 0xF0;
	
	//PORTE
	
	
	//PORTF
	//ADC IS ON PORT F
	
	//PORTL
	//bit 7 is Hall effect
	DDRL = 0b00000000;	
/****************** END PORT USAGE ************/
	
	
/****** LCD INIT *******/ 
	//Initialize LCD module
	InitLCD(LS_BLINK|LS_ULINE);
	LCDClear();
/****** END LCD INIT ******/


/************ INTERRUPT CONFIG ************/
	// config the external interrupt ======================================
	//table15-1
	EIMSK |= (_BV(INT0)); // enable INT0
	EICRA |= _BV(ISC01); // rising edge interrupt
	EICRA |= _BV(ISC00); // rising edge interrupt
	
	EIMSK |= (_BV(INT1)); // enable INT1
	EICRA |= _BV(ISC11); // falling edge interrupt
	EICRA &= ~_BV(ISC10); // falling edge interrupt
	
	EIMSK |= (_BV(INT2)); // enable INT2
	EICRA |= _BV(ISC21); // falling edge interrupt
	EICRA &= ~_BV(ISC20); // falling edge interrupt

	EIMSK |= (_BV(INT3)); // enable INT3
	EICRA |= _BV(ISC31); // rising edge interrupt
	EICRA |= _BV(ISC30); // rising edge interrupt
/************ END INTERRUPT CONFIG ************/
	
/********** ADC CONFIG ***************/
	// by default, the ADC input (analog input is set to be ADC0 / PORTF0
	ADCSRA |= _BV(ADEN);		// enable ADC
	ADCSRA |= _BV(ADIE);		// enable interrupt of ADC

	//ADLAR- analog digital Left Adjust register
	//get rid of _BV(ADLAR)
	//REFS0- reference selection(turn on capacitor at AVCC
	ADMUX |= _BV(REFS0); // Read Technical Manual & Complete Comment

/********** END ADC CONFIG ***************/
	
	sei();//global interrupt enable
	
 /****************           PWM INIT             **************************/
	 //set up port B for fast PWM, fast PWM WGM0,WGM1 all high
	 //com0a1, is clear OC0A on compare match, (table 16-2)
	 TCCR0A |= 0b10000011;//sets both WGM01 and WGM00 to one also set COM0A1 to one,
 
	 //Timer counter 2
	 TCCR0B |= 0b00000011; //divide clock by 8(still too big) makes it 3.9kHz, 64 0b00000011
 
	 //output compare register
	 //8 bit count this is duty cycle
	 OCR0A = 0x60;//96(37.5%?)
 /****************          END PWM INIT             **************************/
	
	home_stepper();
	setup(&headptr, &tailptr);
	curMode = 0;//to black
	
	
	PORTB = 0b00001101;
	
	STATE = 0;

	goto POLLING_STAGE;
	
	//polling state
	POLLING_STAGE:
	switch(STATE){
		case(polling):
		goto POLLING_STAGE;
		break;
		
		case(pause):
		goto PAUSE_STAGE;
		break;
		
		case(reflective):
		goto REFLECTIVE_STAGE;
		break;
		
		case(drop_result):
		goto DROP_STAGE;
		break;
		
		case(ramp_down):
		goto END_STAGE;
		
		default:
		goto POLLING_STAGE;
	}//end of switch polling
	
	PAUSE_STAGE:
		PORTB = 0b00001111;//break to vcc
		LCDClear();
		LCDWriteString("PAUSED");
		
		//the paused prints
		
		LCDWriteStringXY(0, 0, "PAUSED");
		
		LCDWriteStringXY(7, 0, "B:");
		LCDWriteIntXY(9, 0, sorted[BLACK], 2);
		
		LCDWriteStringXY(12, 0, "S:");
		LCDWriteIntXY(14, 0, sorted[STEEL], 2);

		LCDWriteStringXY(7, 1, "W:");
		LCDWriteIntXY(9, 1, sorted[WHITE], 2);
		
		LCDWriteStringXY(12, 1, "A:");
		LCDWriteIntXY(14, 1, sorted[ALUM], 2);
				
		LCDWriteStringXY(0, 1, "BB:");
		LCDWriteIntXY(3, 1, sorted[4], 2);
		
		
		if(paused_check == 0x01){//toggle state
			PORTB = 0b00001101;
			LCDClear();
			
		}
	STATE = polling;
	goto POLLING_STAGE;
	
    REFLECTIVE_STAGE:
		
		curmaterialmin = 1024;
		reflect_count = 0;
		
		while((PIND & 0x01) == 0x01 ){//OR pin is active
			if(ADC_result_flag == 0){
				ADCSRA |= _BV(ADSC);//analog digital start one conversion
			}else{
				if(ADC_result < curmaterialmin){
					curmaterialmin = ADC_result;
				}
				ADC_result_flag = 0;
				reflect_count++;
			}//end of ADC result check
		
			
		}//end of while
		
		
		
		//while((PIND & 0x01) == 0x01);
		//nTimer(25);
		
		if(reflect_count > 0){

			//add material to queue
			//(before adding check with display)
			
			char material_type = 0;
		
			if(curmaterialmin > 930){//black
				material_type = BLACK;
			}else if(curmaterialmin > 700){//white
				material_type = WHITE;
			}else if(curmaterialmin > 300){//steel
				material_type = STEEL;
			}else{//alum
				material_type = ALUM;
			}
			
			
			
			//create link
			//idk how this work so this is a guess
			initLink(&newlink);
			newlink->e.itemCode = material_type;
			enqueue(&headptr, &tailptr, &newlink);
			sorted[4] += 1;//add 1 to the between sensors
			
			//LCDWriteStringXY(0, 1, "Val Count:");
			//LCDWriteIntXY(11, 1, reflect_count, 5);
			//nTimer(4000);
		}
		STATE = 0;
		goto POLLING_STAGE;
		
		
	DROP_STAGE:
		STATE = 0;
		
		//brake to vcc
		PORTB |= 0b00001111;
		nTimer(25);
		
		char material_type = 0;
		
		//read material from queue
		dequeue(&headptr, &tailptr, &rtnlink); // remove the item at the head of the list
		material_type = rtnlink->e.itemCode;
		free(rtnlink);
		
		
		if( material_type == BLACK){
			//LCDWriteStringXY(10, 0, "BLACK");
			sorted[4] -= 1;
			sorted[BLACK] += 1;
		}else if( material_type == WHITE ){
			//LCDWriteStringXY(10, 0, "WHITE");
			sorted[4] -= 1;
			sorted[WHITE] += 1;
		}else if( material_type == STEEL ){
			//LCDWriteStringXY(10, 0, "STEEL");
			sorted[4] -= 1;
			sorted[STEEL] += 1;
		}else if( material_type == ALUM ){
			//LCDWriteStringXY(10, 0, "ALUM");
			sorted[4] -= 1;
			sorted[ALUM] += 1;
		}
		
		//move stepper to section
		if(material_type == curMode){
			curMode = material_type;
		}else if(material_type == ((curMode + 1)%4) ){
			turnCCW90();
			curMode = material_type;
		}else if(material_type == ((curMode + 2)%4)){
			turnCCW180();
			curMode = material_type;
		}else if(material_type == ((curMode + 3)%4)){
			turnCW90();
			curMode = material_type;
		}
		
		//start belt
		PORTB = 0b00001101;
	
		if((ramped_down == 1) && (sorted[4] == 0)){
			goto END_STAGE;
		}
	
		//LCDClear();
		//LCDWriteString("DROP");
		//nTimer(3000);
		goto POLLING_STAGE;
		
	END_STAGE:
		
		//give time to drop last piece
		nTimer(500);
		
		PORTB |= 0b00001111;
		LCDClear();
		
		LCDWriteStringXY(0, 0, "Ramped");
		LCDWriteStringXY(0, 1, "Down:)");
		
		LCDWriteStringXY(7, 0, "B:");
		LCDWriteIntXY(9, 0, sorted[BLACK], 2);
		
		LCDWriteStringXY(12, 0, "S:");
		LCDWriteIntXY(14, 0, sorted[STEEL], 2);

		LCDWriteStringXY(7, 1, "W:");
		LCDWriteIntXY(9, 1, sorted[WHITE], 2);
		
		LCDWriteStringXY(12, 1, "A:");
		LCDWriteIntXY(14, 1, sorted[ALUM], 2);
		
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


void home_stepper(){
	for(int i = 0; i < 25; i++){
		curposition++;
		//test if valid
		if(curposition > 3 ){
			curposition = 0;
		}
		//output A to current step
		PORTA = steps_arr[curposition];
		//delay 20ms
		nTimer(15);
	}
	
	while((PINL & 0b10000000) == 0b10000000 ){//Sensor_HE != 1;
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
	delay = 20;
	for(int i =0; i < 50; i++){
		//increment current position
		curposition++;
		//test if valid
		if(curposition > 3 ){
			curposition = 0;
		}
		//output A to current step
		PORTA = steps_arr[curposition];
		
		delay = ((i<10)?(20-(1+i)/2):10);
		delay = ((i>39)?(i-30):(delay));
		//delay 20ms-
		nTimer(delay);
		
	}//end for
	
	return;
}//end cw

//trap accel
void turnCCW90(){
	delay = 20;
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
		
		delay = ((i<10)?(20-(1+i)/2):10);
		delay = ((i>39)?(i-30):(delay));
		//delay 20ms-
		nTimer(delay);
		
	}
	
	return;
}

//trap accel
void turnCCW180(){
	delay = 20;
	for(int i =0; i < 100; i++){
		//increment curposition(can use ternary and assignment) and
		curposition--;
		//test if valid
		if(curposition < 0 ){
			curposition = 3;
		}
		//output A to current step
		PORTA = steps_arr[curposition];
		//delay 20ms
		
		delay = ((i<10)?(20-(1+i)/2):10);
		delay = ((i>89)?(i-80):(delay));
		//delay 20ms-
		nTimer(delay);
		
	}
	
	return;
}

//trap accel
void turnCW180(){
	delay = 20;
	for(int i =0; i < 100; i++){
		//increment current position
		curposition++;
		//test if valid
		if(curposition > 3 ){
			curposition = 0;
		}
		//output A to current step
		PORTA = steps_arr[curposition];
		
		delay = ((i<10)?(20-(1+i)/2):10);
		delay = ((i>89)?(i-80):(delay));
		//delay 20ms-
		nTimer(delay);
		
	}//end for
	
	return;
}//end cw

/*

isr

pause/resume


ramp down
use external timer


*/
ISR(ADC_vect){
	ADC_result = ADCL;
	ADC_result += (ADCH << 8);//adjust to 10bit mode
	ADC_result_flag = 1;
}

//int0 OR opposite reflect
ISR(INT0_vect){
	
	if((PIND & 0x01) == 0x01){
		STATE = reflective;
	}
}

//int1 EX end of belt sensor
ISR(INT1_vect){
	if((PIND & 0x02) != 0x02){
		STATE = drop_result;
	}
}

//int2 ramp down button
ISR(INT2_vect){//on int2 falling edge(active low)
	
	if (timer_running) return;

	ramped_down = 0;
	timer_running = 1;

	// Stop Timer3 while configuring
	TCCR3B = 0;

	// Clear control registers
	TCCR3A = 0;
	TCCR3B = 0;

	// Reset counter
	TCNT3 = 0;

	// Set compare value for longest delay
	OCR3A = 5535;

	// Clear pending compare flag
	TIFR3 |= (1 << OCF3A);

	// Enable compare match A interrupt
	TIMSK3 |= (1 << OCIE3A);

	// CTC mode: WGM32 = 1
	TCCR3B |= (1 << WGM32);

	// Start timer with prescaler 1024
	TCCR3B |= (1 << CS32) | (1 << CS30);
	
}

//timer 3 ISR (for ramp down timer)
ISR(TIMER3_COMPA_vect)
{
	TCCR3B = 0;
	timer_running = 0;
	ramped_down = 1;
}

//int3 pause button
ISR(INT3_vect){//on rising edge(active high)
	if((PIND & 0x08) == 0x08){
		//debounce
		nTimer(25);
		
		//flip bit 0 of the paused check
		paused_check ^= 0x01;
		STATE = pause;
		
		//debounce
		while((PIND & 0x08) == 0x08);//loop while PD3 is high
		nTimer(25);
	}
	
}

ISR(BADISR_vect)
{
	LCDClear();
	LCDWriteString("ISR BAD");
	nTimer(3000);
}
