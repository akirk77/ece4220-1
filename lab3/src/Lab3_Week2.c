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

#define SCHED_POLICY SCHED_RR

typedef struct
{
	int priority;
} priority_dat;

pthread_mutex_t lock;

//Thread for the crosswalk light
void red_light( void * ptr )
{

  //Cast the void pointer data that has the priority
	priority_dat * prior = (priority_dat*) ptr;

  //Set scheduling priority and scheme
	struct sched_param param;
	param.sched_priority = (*prior).priority;
	sched_setscheduler( 0, SCHED_POLICY, &param );

  //Infinite Loop
	while(1)
	{

    //If the crosswalk button has been pressed, run the sequence
		if( check_button() == 1 )
		{

      //Lock the semaphore
			pthread_mutex_lock( &lock );

      //Turn off all the other lights
			puts( "Red Light" );
			digitalWrite( LED1, LOW );
			digitalWrite( LED2, LOW );
			digitalWrite( LED3, LOW );
			digitalWrite( LED4, LOW );

      //Turn on Red Light
			digitalWrite( LED1, HIGH );

      //Reset the button pressed state to not
			clear_button();

      //Wait
			sleep( 1 );

      //Unlock the semaphore
			pthread_mutex_unlock( &lock );

      //Brief wait to ensure that the same thread does not starve others
			usleep( 1000 );

		}
	}

}
                
//Thread for the green light
void green_light( void * ptr )
{
                                            
  //Cast the void pointer data that has the priority
	priority_dat * prior = (priority_dat*) ptr;
                                                
  //Set scheduling priority and scheme
	struct sched_param param;
	param.sched_priority = (*prior).priority;
	sched_setscheduler( 0, SCHED_POLICY, &param );

	while(1)
	{                           
    //Lock the semaphore
		pthread_mutex_lock( &lock );
                               
    //Turn off all the other lights
		puts( "Green Light" );
		digitalWrite( LED1, LOW );
		digitalWrite( LED2, LOW );
		digitalWrite( LED3, LOW );
		digitalWrite( LED4, LOW );
                              
    //Turn on Green Light
		digitalWrite( LED3, HIGH );

    //Wait
		sleep( 1 );
                                         
    //Unlock the semaphore
		pthread_mutex_unlock( &lock );
                                     
    //Brief wait to ensure that the same thread does not starve others
		usleep( 1000 );
	}
}

void orange_light( void * ptr )
{

  //Cast the void pointer data that has the priority
	priority_dat * prior = (priority_dat*) ptr;
                                                
  //Set scheduling priority and scheme
	struct sched_param param;
	param.sched_priority = (*prior).priority;
	sched_setscheduler( 0, SCHED_POLICY, &param );

	while(1)
	{                   
    //Lock the semaphore
		pthread_mutex_lock( &lock );
                                          
    //Turn off all the other lights
		puts( "Orange Light" );
		digitalWrite( LED1, LOW );
		digitalWrite( LED2, LOW );
		digitalWrite( LED3, LOW );
		digitalWrite( LED4, LOW );
                                  
    //Turn on Yellow Light
		digitalWrite( LED2, HIGH );

    //Wait
		sleep( 1 );
                                         
    //Unlock the semaphore
		pthread_mutex_unlock( &lock );
                                     
    //Brief wait to ensure that the same thread does not starve others
		usleep( 1000 );
	}

}

int main( int argc, char **argv )
{

  //Check that command line parameters are correct
	if( argc != 4 )
	{
		puts( "Invalid startup parameters! \nUse: ./[NAME] [PPL] [PRIO_GREEN] [PRIO_YELLOW]");
		exit(0);
	}

  //Nice welcome message ;)
	puts("Welcome to FROGGER!!!");

	//Setup the wiringPi library
	wiringPiSetup();

  //Set the pin modes
	puts( "Setting LED Pin Modes");
	pinMode(LED1, OUTPUT);
	pinMode(LED2, OUTPUT);
	pinMode(LED3, OUTPUT);
	pinMode(LED4, OUTPUT);

	//Set priority variables
	priority_dat priority_1;
	priority_1.priority = atoi( argv[3] );
	priority_dat priority_2;
	priority_2.priority = atoi( argv[1] );
	priority_dat priority_3;
	priority_3.priority = atoi( argv[2] );

	//Setup threads
	pthread_t redLight, greenLight, orangeLight;

	//Start Threads
	pthread_create( &redLight, NULL, (void *) red_light, (void *) &priority_1 );
	pthread_create( &greenLight, NULL, (void *) green_light, (void *) &priority_2 );
	pthread_create( &orangeLight, NULL, (void *) orange_light, (void *) &priority_3 );

	//Join Threads
	pthread_join( redLight, NULL );
	pthread_join( greenLight, NULL );
	pthread_join( orangeLight, NULL );

	return 0;
}
