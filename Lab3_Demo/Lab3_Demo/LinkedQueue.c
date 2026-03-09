/* Solution Set for the LinkedQueue.c */
/* 	
	Course		: UVic Mechatronics 458
	Milestone	: 3
	Title		: Data structures for MCUs and the Linked Queue Library

	Name 1:					Student ID:
	Name 2:					Student ID:
	
	Description: You can change the following after you read it.  Lab3 Demo
	
	This main routine will only serve as a testing routine for now. At some point you can comment out
	The main routine, and can use the following library of functions in your other applications

	To do this...make sure both the .C file and the .H file are in the same directory as the .C file
	with the MAIN routine (this will make it more convenient)
*/

/* include libraries */
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>//comment out
#include "LinkedQueue.h" 	/* This is the attached header file, which cleans things up */
							/* Make sure you read it!!! */
/* global variables */
/* Avoid using these */

void nTimer(int count);

/* main routine 
   You need to add the mtimer function to this project.    */

int main(){	

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
	select clock 1, bit 1 to 1, 010 (bits 2,1,0)
	010- /8 table 17-6
	*/
	TCCR1B |= _BV(CS11); //bitwise or and assignment table 17-6
	
	//end of clock stuff

	link *head;			/* The ptr to the head of the queue */
	link *tail;			/* The ptr to the tail of the queue */
	link *newLink;		/* A ptr to a link aggregate data type (struct) */
	link *rtnLink;		/* same as the above */
	element eTest;		/* A variable to hold the aggregate data type known as element */

	DDRC = 0xFF; 		/* Used for debugging purposes only LEDs on PORTC */
	//DDRD = 0xFF;
	DDRA = 0x00;

	rtnLink = NULL;
	newLink = NULL;

	setup(&head, &tail);


	while (1){
		//set up
		setup(&head, &tail);
		
		//collect button presses
		for( int i = 0; i < 4; i++){
			while((PINA & 0x04) == 0x04){}//button not pressed, detect
			
			nTimer(25);
			
			//create new link
			initLink(&newLink);
			//load data onto link node
			newLink->e.itemCode = PINA & 0b00000011;
			//enqueue link
			enqueue(&head, &tail, &newLink);
			
			while( (PINA & 0x04) == 0){}//button pressed
			
			nTimer(25);
		}

		//dequeue
		dequeue(&head, &rtnLink);//moves head pointer forward
		//free
		free(rtnLink);

		
		for(int i = 0; i< 3; i++){
			//dequeue
			dequeue(&head, &rtnLink);//moves head pointer forward
			
			PORTC += (rtnLink->e.itemCode << (2*i));
			
			//free
			free(rtnLink);
			//delay
			nTimer(2000);
			
		}
		nTimer(5000);
		PORTC = 0x00;

	}


	/* 
		Many of the following lines will test to see if your algorithms will work. You do not necessarily
		need the MCU attached to the computer to test this, and can do most of the work using the 
		debugger in the AVR studio while observing the I/O View in the top right corner. Click the tab
		or PORTC to see how the output changes while 'stepping' through the routines.
	*/
	/* Initialize a new link here */
	initLink(&newLink);
	newLink->e.itemCode = 3;
	newLink->e.stage = 4;
	enqueue(&head, &tail, &newLink);
	PORTC = head->e.itemCode;	//  You need to insert mTimer in between each output in order to see the values.
	//nTimer(2000);
	
	PORTC = tail->e.stage;		//  Or the LEDs will just flash by very quickly.  You will need about 2 seconds delay.
	//nTimer(2000);
	
	initLink(&newLink);
	newLink->e.itemCode = 5;
	newLink->e.stage = 6;
	enqueue(&head, &tail, &newLink);
	PORTC = head->e.itemCode;
	//nTimer(2000);
	
	PORTC = tail->e.stage;
	//nTimer(2000);

	initLink(&newLink);
	newLink->e.itemCode = 7;
	newLink->e.stage = 8;
	enqueue(&head, &tail, &newLink);
	PORTC = head->e.itemCode;
	//nTimer(2000);
	
	PORTC = tail->e.stage;
	//nTimer(2000);
	
	PORTC = 0x00;
	//nTimer(2000);

	/* Tests to see if firstValue works */
	eTest = firstValue(&head);
	PORTC = eTest.itemCode;
	//nTimer(2000);
	
	PORTC = 0x00;
	//nTimer(2000);

	/* Tests if Size works */
	PORTC = size(&head, &tail);
	//nTimer(2000);

	/* Tests if dequeue works - Uncomment to use
	Also, insert mTimer between each output in order to see the values.   */
	
	//dequeue(&head, &rtnLink); /* remove the item at the head of the list */
	//PORTC = rtnLink->e.itemCode;
	//dequeue(&head, &rtnLink); /* remove the item at the head of the list */
	//PORTC = rtnLink->e.itemCode;
	//dequeue(&head, &rtnLink); /* remove the item at the head of the list */
	//PORTC = rtnLink->e.itemCode;
	
	/* Tests is empty */
	PORTC = isEmpty(&head);

	/* Tests to see if clearQueue works*/
	clearQueue(&head, &tail);
	PORTC = size(&head, &tail);

	PORTC = isEmpty(&head);


	return(0);
}/* main */

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


/**************************************************************************************/
/***************************** SUBROUTINES ********************************************/
/**************************************************************************************/







/**************************************************************************************
* DESC: initializes the linked queue to 'NULL' status
* INPUT: the head and tail pointers by reference
*/

void setup(link **h,link **t){
	*h = NULL;		/* Point the head to NOTHING (NULL) */
	*t = NULL;		/* Point the tail to NOTHING (NULL) */
	return;
}/*setup*/




/**************************************************************************************
* DESC: This initializes a link and returns the pointer to the new link or NULL if error 
* INPUT: the head and tail pointers by reference
*/
void initLink(link **newLink){
	//link *l;
	*newLink = malloc(sizeof(link));
	(*newLink)->next = NULL;
	return;
}/*initLink*/




/****************************************************************************************
*  DESC: Accepts as input a new link by reference, and assigns the head and tail		
*  of the queue accordingly				
*  INPUT: the head and tail pointers, and a pointer to the new link that was created 
*/
/* will put an item at the tail of the queue */
void enqueue(link **h, link **t, link **nL){

	if (*t != NULL){
		/* Not an empty queue */
		(*t)->next = *nL;
		*t = *nL; //(*t)->next;
	}/*if*/
	else{
		/* It's an empty Queue */
		//(*h)->next = *nL;
		//should be this
		*h = *nL;
		*t = *nL;
	}/* else */
	return;
}/*enqueue*/




/**************************************************************************************
* DESC : Removes the link from the head of the list and assigns it to deQueuedLink
* INPUT: The head and tail pointers, and a ptr 'deQueuedLink' 
* 		 which the removed link will be assigned to
*/
/* This will remove the link and element within the link from the head of the queue */
void dequeue(link **h, link **deQueuedLink){
	/* ENTER YOUR CODE HERE */
	*deQueuedLink = *h;	// Will set to NULL if Head points to NULL
	/* Ensure it is not an empty queue */
	if (*h != NULL){
		*h = (*h)->next;
	}/*if*/
	
	return;
}/*dequeue*/




/**************************************************************************************
* DESC: Peeks at the first element in the list
* INPUT: The head pointer
* RETURNS: The element contained within the queue
*/
/* This simply allows you to peek at the head element of the queue and returns a NULL pointer if empty */
element firstValue(link **h){
	return((*h)->e);
}/*firstValue*/





/**************************************************************************************
* DESC: deallocates (frees) all the memory consumed by the Queue
* INPUT: the pointers to the head and the tail
*/
/* This clears the queue */
void clearQueue(link **h, link **t){

	link *temp;

	while (*h != NULL){
		temp = *h;
		*h=(*h)->next;
		free(temp);
	}/*while*/
	
	/* Last but not least set the tail to NULL */
	*t = NULL;		

	return;
}/*clearQueue*/





/**************************************************************************************
* DESC: Checks to see whether the queue is empty or not
* INPUT: The head pointer
* RETURNS: 1:if the queue is empty, and 0:if the queue is NOT empty
*/
/* Check to see if the queue is empty */
char isEmpty(link **h){
	/* ENTER YOUR CODE HERE */
	return(*h == NULL);
}/*isEmpty*/





/**************************************************************************************
* DESC: Obtains the number of links in the queue
* INPUT: The head and tail pointer
* RETURNS: An integer with the number of links in the queue
*/
/* returns the size of the queue*/
int size(link **h, link **t){

	link 	*temp;			/* will store the link while traversing the queue */
	int 	numElements;

	numElements = 0;

	temp = *h;			/* point to the first item in the list */

	while(temp != NULL){
		numElements++;
		temp = temp->next;
	}/*while*/
	
	return(numElements);
}/*size*/

