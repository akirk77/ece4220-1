/*
 ============================================================================
 Name        : ECE 4220: Lab 1
 Author      : Joel Abshier
 Version     : 1.0.0
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h>


#define LED1 8		//RED LED
#define LED4 21		//BLUE LED

#define BTN1 27
#define BTN2 0
#define BTN3 1
#define BTN4 24
#define BTN5 28

#define SPKR 22

int spkr();
int leds();
void changeLights();

void changeLights()
{

	//This variable x will be ever increasing, starting from 0
	static int x = 0;

	//If x is an even number, turn LED1 on and LED 4 off. Otherwise turn LED1 off and LED4 on
	if( x%2 == 1 )
	{
		digitalWrite( LED1, HIGH );
		digitalWrite( LED4, LOW );
	}
	else
	{
		digitalWrite( LED1, LOW );
		digitalWrite( LED4, HIGH );
	}

	//increase x
	x++;

	return;
}

int leds()
{

	//Welcome
	puts("Begin LED Example");

	//Setup the wiringPi library
	wiringPiSetup();

	//Set up the LEDs for writing
	pinMode(LED1, OUTPUT);
	pinMode(LED4, OUTPUT);

	while(1)
	{
		//Wait one second
		usleep( 100000 );

		//Call changeLights function
		changeLights();
	}

	return 0;

}

int spkr()
{

	//Welcome
	puts("Begin Speaker Example");

	//Setup the wiringPi library
	wiringPiSetup();

	//Setup the buttons and speakers for reading or writing
	pinMode( BTN1, INPUT );
	pinMode( BTN2, INPUT );
	pinMode( BTN3, INPUT );
	pinMode( BTN4, INPUT );
	pinMode( BTN5, INPUT );
	pinMode( SPKR, OUTPUT );

	//Collect a user input between 1 and 5
	int input = 0;
	while( (input < 1) || (input > 5) )
	{
		puts( "Input which button you will be using (1-5): " );
		scanf( "%d", &input );
	}

	//Depending on which button was chosen, set the btn int to the value of the constant
	int btn;
	switch( input )
	{
		case 1:
			btn = BTN1;
			break;
		case 2:
			btn = BTN2;
			break;
		case 3:
			btn = BTN3;
			break;
		case 4:
			btn = BTN4;
			break;
		case 5:
			btn = BTN5;
			break;
	}

	//Set button control to down
	pullUpDnControl( btn, PUD_DOWN );

	printf( "Waitng for BTN%d to be pressed!", btn );

	//Flush so the previous scanf doesn't break anything
	fflush( stdout );
	fflush( stdin );

	//Wait for the button to be pressed
	int readVal = 0;
	while( (readVal = digitalRead(btn)) == 0 )
	{
		//Nothing
	}

	puts( "\nButton Pressed!" );

	//Once the button has been pressed, play the sound
	int play = 0;
	while( 1 )
	{
		//For each iteration through the loop, either turn the speaker on or off and then change the value of play so that the opposite will occure
		if( play == 0)
		{
			digitalWrite( SPKR, HIGH );
			play = 1;
		}
		else
		{
			digitalWrite( SPKR, LOW );
			play = 0;
		}

		//Wait. High numbers produce lower frequency sounds and lower values make it higher
		usleep( 750 );
	}

}

int main( void )
{
	//Collect a user input as 1 or 2 in order to select which part of the lab to run
	int input = 0;
	while( (input < 1) || (input > 2) )
	{
		//Nice words
		puts( "Choose a program:\n  1. LED Flash\n  2. Speaker Sound\n" );
		//Read value into input
		scanf( "%d", &input );
	}

	//Call a different example, depending on which option was chosen
	switch( input )
	{
		case 1:
			//If they chose 1, run the LED Flashing Example
			leds();
			break;
		case 2:
			//If they chose 2, run the speaking whining example
			spkr();
			break;
	}

	return 0;
}

