/*
 ============================================================================
 Name        : lab3.c
 Author      : Joel Abshier
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>
#include <sched.h>
#include <pthread.h>
#include "ece4220lab3.h"


#define LED1 8		//RED LED
#define LED2 9		//YELLOW LED
#define LED3 7		//GREEN LED
#define LED4 21		//BLUE LED

#define BTN1 27		//BTN1 LED

void run_light_sequence( void * ptr )
{

	puts("Scheduling!");
	struct sched_param param;
	param.sched_priority = 51;
	sched_setscheduler( 0, SCHED_FIFO, &param );

	puts( "Turn LEDs off" );
	digitalWrite( LED1, LOW );
	digitalWrite( LED2, LOW );
	digitalWrite( LED3, LOW );
	digitalWrite( LED4, LOW );

	int * billy = (int*) ptr;

	puts("Starting while loop!");

	fflush(stdout);

	while(1)
	{
		//Green Light
		digitalWrite( LED3, HIGH );
		sleep(1);
		digitalWrite( LED3, LOW );

		//Yellow Light
		digitalWrite( LED2, HIGH );
		sleep(1);
		digitalWrite( LED2, LOW );

    //If the button has been pressed, open the crosswalk and wait, then turn off
		if( check_button() == 1 )
		{
			digitalWrite( LED1, HIGH );
			clear_button();

			sleep(1);
			digitalWrite( LED1, LOW );
		}

	}

}

int main(void)
{
	puts("Welcome to FROGGER!!!");

	//Setup the wiringPi library
	wiringPiSetup();

  //Setup Pin Modes
	puts( "Setting LED Pin Modes");
	pinMode(LED1, OUTPUT);
	pinMode(LED2, OUTPUT);
	pinMode(LED3, OUTPUT);
	pinMode(LED4, OUTPUT);

  //Setup input button (not needed with kernel module)
	pinMode(BTN1, INPUT);
	pullUpDnControl( BTN1, PUD_DOWN );

  //Setup thread and join it
	puts( "Setting up thread!" );
	int test;
	pthread_t runSequence;
	pthread_create( &runSequence, NULL, (void *) run_light_sequence, (void *) &test );
	pthread_join( runSequence, NULL );

	return EXIT_SUCCESS;
}
