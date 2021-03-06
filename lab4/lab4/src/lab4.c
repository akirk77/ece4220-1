/*
 ============================================================================
 Name        : lab4.c
 Author      : Joel Abshier
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sched.h>
#include <pthread.h>
#include <time.h>
#include "ece4220lab3.h"


void child_thread( void * ptr );
void event_thread( void * ptr );
void printing_thread( void * ptr );


typedef struct
{
	int location;
	unsigned long time;
	int ppipe;
} gps_data;

typedef struct
{
  int beforeLoc;
  unsigned long beforeTime;
  int afterLoc;
  unsigned long afterTime;
} gps_comp;

typedef struct
{
	double location;
	unsigned long time;
} print_data;

void event_thread( void * ptr )
{
	gps_data * buffer = ( gps_data * ) ptr;

	//Declarations
	int pipe_N_pipe2;

	//Check if pipe opened!
	if( ( pipe_N_pipe2 = open( "/tmp/N_pipe2", O_RDONLY ) ) < 0 )
	{
		puts( "Pipe N_pipe2 failed to open!" );
	}

	unsigned int ms;
	pthread_t my_child, pthread1;
  
  	int simple_pipe[2];
  	if( pipe(simple_pipe) < 0 )
  	{
    		puts( "Pipe Creation Error");
  	}
	
	buffer->ppipe = simple_pipe[1];
 	//printf( "Pipe[0] = %d; Pipe[1] = %d;", simple_pipe[0], simple_pipe[1] ); 
	pthread_create( &pthread1, NULL, (void *) printing_thread, (void *) &simple_pipe[0] );


	while(1)
	{
		puts( "waiting for press...");
		read( pipe_N_pipe2, &ms, sizeof(ms) );

		//I'm doing things!!!
		//puts( "Press recieved from Process 2!\n");

		//Update Buffer with time
		//puts( "Updating Buffer Time" );
		//buffer->time = ms;

		//Create child thread to wait for data change
		//puts( "Creating new Child" );
		pthread_create( &my_child, NULL, (void *) child_thread, (void *) buffer );

	}


}

void child_thread( void * ptr )
{
	//puts( "A child Thread Created" );

	//Follow Buffer Data
	gps_data * buffer = (gps_data *) ptr;

	//Get current time
	struct timespec spec;
	clock_gettime( CLOCK_REALTIME, &spec );
	unsigned long currentTime = ( spec.tv_sec * 1000000000 ) + ( spec.tv_nsec );

	gps_comp newData;
	newData.beforeLoc = buffer->location;
	newData.beforeTime = buffer->time;

	while( newData.beforeLoc == buffer->location )
	{
		usleep(20);
	}

	newData.afterTime = buffer->time;
	newData.afterLoc = buffer->location;

	//printf( "\n\nTEST: %lu,  %lu", newData.beforeTime, newData.afterTime );

	//puts( "The data changed and I can now interpolate!\n");
  
  	//Setup vars
 	double x0 = (double) currentTime;
  	double x1 = (double) newData.beforeTime;
  	double x2 = (double) newData.afterTime;
  	double y1 = (double) newData.beforeLoc;
  	double y2 = (double) newData.afterLoc;
  
  	//Interpolation
	printf( "\n\nInterpolate\n"
		"> x0 = %lf\n"
		"> x1 = %lf\n"
		"> x2 = %lf\n"
		"> y1 = %lf\n"
		"> y2 = %lf\n",
		x0, x1, x2, y1, y2 );
	//double interpolated = (double) ( y1 + ( ( x0 - x1 ) / ( x2 - x1 ) * ( y2 - y1 ) ) );  
          double interpolated = (double) ( y1 + ( ( x0 - x1 ) * ( y2 - y1 ) / ( x2 - x1 ) ) );
	printf( "Interpolated Value: %lf\n\n", interpolated );	

  	//Write the data to the simple printing pipe
  	print_data toPrint;
  	toPrint.location = interpolated;
	toPrint.time = x0;  

	//printf( "Child: Pipe[1]: %d", buffer->ppipe );

 	write( buffer->ppipe, &toPrint, sizeof( gps_data ) );

	//printf( "Location is %d at %u\n", (int)interpolated, x0 );

	pthread_exit(0);

}

void printing_thread( void * ptr )
{

	int * laPipeP = (int*) ptr;
	int simple_pipe = *laPipeP;
	print_data printData;

	//printf( "Print: pipe[0]: %d", simple_pipe ); 
 
	while(1)
	{
		//puts( "Printing thread waiting...\n" );
		read( simple_pipe, &printData, sizeof( gps_data ) );   
		printf( "Location is %lf at %u\n", printData.location, printData.time );
  	}

}


int main(void)
{
	//Declarations
	int pipe_N_pipe1;
	gps_data buffer;
	pthread_t pthread0;

	//Check if pipe opened!
	if( ( pipe_N_pipe1 = open( "/tmp/N_pipe1", O_RDONLY ) ) < 0 )
	{
		puts( "Pipe N_pipe1 failed to open!" );
	}

	//Open Threads
	pthread_create( &pthread0, NULL, (void *) event_thread, (void *) &buffer );

	//Forever!!!
	while(1)
	{

		//Sleep 250 ms
		usleep( 250000 );
		//puts( "Test!");

		//Timer structs
		struct timespec startTime, endTime;
		unsigned int diff;

		//START SEARCH WITH ONE THREAD
		clock_gettime( CLOCK_MONOTONIC, &startTime );

		//Try to read pipe and output
		int thingy = 0;
		if( read( pipe_N_pipe1, &thingy, sizeof( thingy ) ) )
		{
			buffer.location = thingy;
		}

		//clock_gettime( CLOCK_REALTIME, &endTime );
		//diff = ( 1000000000 * (endTime.tv_sec - startTime.tv_sec) ) + endTime.tv_nsec - startTime.tv_nsec;

		struct timespec spec;
		clock_gettime( CLOCK_REALTIME, &spec );
		buffer.time = (unsigned long) ( spec.tv_sec * 1000000000 ) + spec.tv_nsec ;

	}

	return EXIT_SUCCESS;

}
