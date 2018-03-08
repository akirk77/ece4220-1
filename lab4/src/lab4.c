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
#include "ece4220lab3.h"

typedef struct
{
	int location;
	unsigned int time;
} gps_data;


void event_thread( void * ptr )
{
	gps_data * buffer = ( gps_data * ) ptr;

	while(1)
	{
		//Sleep 250 ms
		usleep( 250000 );
		//double milli = ( (double) buffer->time) / 1000000;

		//Try to read pipe and output
		printf( "The THINGY is:\n> %d at\n %u (ms)\n", buffer->location, buffer->time );

	}
}

void printing_thread( void * ptr )
{

}


int main(void)
{
	//Declarations
	int pipe_N_pipe1;
	gps_data buffer;
	pthread_t pthread0, pthread1;

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

		clock_gettime( CLOCK_REALTIME, &endTime );
		diff = ( 1000000000 * (endTime.tv_sec - startTime.tv_sec) ) + endTime.tv_nsec - startTime.tv_nsec;

		buffer.time = diff;

	}

	return EXIT_SUCCESS;

}
