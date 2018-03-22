#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sched.h>
#include <pthread.h>
#include <sys/timerfd.h>
#include <time.h>
#include <stdint.h>
#include <wiringPi.h>
#include "ece4220lab3.h"

#define SCHED_POLICY SCHED_FIFO
#define SCHED_PRIORITY 55

#define BTN1 27		//BTN1 LED

int main( void )
{

	wiringPiSetup();
	pinMode(BTN1, INPUT);
	pullUpDnControl( BTN1, PUD_DOWN );

	int sys;
	sys = system( "mkfifo /tmp/N_pipe2" );
	
	//Declarations
	int pipe_N_pipe2;

	//Check if pipe opened!
	if( ( pipe_N_pipe2 = open( "/tmp/N_pipe2", O_WRONLY ) ) < 0 )
	{
		puts( "Pipe N_pipe2 failed to open!" );
		exit(-1);
	}

	puts( "Pipe created!" );

    int timer_fd = timerfd_create(CLOCK_REALTIME, 0);	// returns a file descriptor
	struct itimerspec itval;	// structure to hold period and starting time
	struct sched_param param;
	
	param.sched_priority = SCHED_PRIORITY;
	sched_setscheduler( 0, SCHED_POLICY, &param );

    	itval.it_interval.tv_sec = 0;
    	itval.it_interval.tv_nsec = 75000000;

    	itval.it_value.tv_sec = 0;
	itval.it_value.tv_nsec = 10000;

	timerfd_settime(timer_fd, 0, &itval, NULL);
	uint64_t num_periods = 0;
	
	struct timespec spec;
	
	//puts(" Waiting for button press! \n");

	while(1)
	{

		if( check_button() == 1 )
		{
			clock_gettime( CLOCK_REALTIME, &spec );
			unsigned long ns = (unsigned long) (spec.tv_sec * 1000000000 ) + (spec.tv_nsec);
			write( pipe_N_pipe2, &ns, sizeof(ns) );
			clear_button();
			puts( "Button pressed" );
		}
		
		read(timer_fd, &num_periods, sizeof(num_periods));

		//puts( "Button loop" );
	
	}

}

