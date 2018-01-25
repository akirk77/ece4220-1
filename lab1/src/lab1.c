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


void changeLights()
{
	static int x = 0;

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

	x++;

	return;
}

int leds()
{

	puts("Begin LED Example");

	wiringPiSetup();

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

	puts("Begin Speaker Example");

	wiringPiSetup();

	pinMode( BTN1, INPUT );
	pinMode( BTN2, INPUT );
	pinMode( BTN3, INPUT );
	pinMode( BTN4, INPUT );
	pinMode( BTN5, INPUT );

	pinMode( SPKR, OUTPUT );

	int input = 0;

	while( (input < 1) || (input > 5) )
	{
		puts( "Input which button you will be using (1-5): " );
		scanf( "%d", &input );
	}

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

	pullUpDnControl( btn, PUD_DOWN );

	printf( "Waitng for BTN%d to be pressed!", btn );
	fflush( stdout );
	fflush( stdin );

	int readVal = 0;
	while( (readVal = digitalRead(btn)) == 0 )
	{
		//Nothing
	}

	puts( "\nButton Pressed!" );

	int play = 0;
	while( 1 )
	{
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

		usleep( 750 );
	}

}

int main( void )
{
	int input = 0;

	while( (input < 1) || (input > 2) )
	{
		puts( "Choose a program:\n  1. LED Flash\n  2. Speaker Sound\n" );
		scanf( "%d", &input );
	}

	switch( input )
	{
		case 1:
			leds();
			break;
		case 2:
			spkr();
			break;
	}

	return 0;
}

