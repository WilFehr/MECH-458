#include<avr/io.h>

int debug(char intput);//addition: this says intput not input, error here

int main(int argc, char *argv[]){
	char readInput;
	
	DDRA = 0x00;	//Set all of Port A to input bits
	DDRC = 0xFF;	//Set all of Port C to output bits for red LEDs

	/* Poll for Port A input and adjust LEDs on PORTC */
	/* Make sure all inputs have a value ... ie. not floating!!!*/
	while( 1 ){
		readInput = PINA & 0b00000111;	//NOTE: we read off the register PINA NOT PORTA
		debug(readInput);
	}
	return 0;

}



int debug(char input){
	switch (input){
		case (0x01):
		PORTC = 0b00000001;
		break;
		case (0x02):
		PORTC = 0b00000010;
		break;
		case (0x04):
		PORTC = 0b00000100;
		break;
		case (0x08):
		PORTC = 0b00001000;
		break;
		default:
		PORTC = 0b00000000;
		break;
		}/*switch*/
		return(input);
		}/* debug */