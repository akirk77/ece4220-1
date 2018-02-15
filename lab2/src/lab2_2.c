#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/timerfd.h>
#include <time.h>
#include <pthread.h>
#include <string.h>

#define RT_PRIORITY 51  // kernel is priority 50
#define BUFF_SIZE 100

typedef struct
{
	int thr_id;
	int per_sec;
	int per_nsec;
	int start_sec;
	int start_nsec;
	char filename[20];
} read_file;

typedef struct
{
	int thr_id;
	int per_sec;
	int per_nsec;
	int start_sec;
	int start_nsec;
	char lines[20][BUFF_SIZE];
} add_to_array;

char buffer[BUFF_SIZE];

void read_to_buffer( void * ptr )
{

	read_file * info = (read_file*) ptr;

	// --- Configure Timer ---
    int timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);	// returns a file descriptor

	struct itimerspec itval;	// structure to hold period and starting time
	struct sched_param param;

	param.sched_priority = RT_PRIORITY;
	sched_setscheduler(0, SCHED_FIFO, &param);
	
    itval.it_interval.tv_sec = info->per_sec;
    itval.it_interval.tv_nsec = info->per_nsec;

    itval.it_value.tv_sec = info->start_sec;
    itval.it_value.tv_nsec = info->start_nsec;


    //Open File
	FILE * file;
	if( (file = fopen( info->filename, "r" ) ) == NULL )
	{
		puts( "\nThe file could not be open. Check your spelling!");
		exit(1);
	}

	
	timerfd_settime(timer_fd, 0, &itval, NULL);


	uint64_t num_periods = 0;

	//Wait one period
	read(timer_fd, &num_periods, sizeof(num_periods));

	while( !feof(file) )
	{
		fgets( buffer, BUFF_SIZE, file );
		read(timer_fd, &num_periods, sizeof(num_periods));
	}

	fclose( file );

    if( num_periods > 1 ) {
        puts("MISSED WINDOW");
        exit(1);
    }

}


void buffer_to_array( void * ptr )
{
	add_to_array * info = (add_to_array*) ptr;

	// --- Configure Timer ---
    int timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);	// returns a file descriptor

	struct itimerspec itval;	// structure to hold period and starting time
	struct sched_param param;

	param.sched_priority = RT_PRIORITY;
	sched_setscheduler(0, SCHED_FIFO, &param);

    itval.it_interval.tv_sec = info->per_sec;
    itval.it_interval.tv_nsec = info->per_nsec;

    itval.it_value.tv_sec = info->start_sec;
    itval.it_value.tv_nsec = info->start_nsec;

	timerfd_settime(timer_fd, 0, &itval, NULL);

		
	uint64_t num_periods = 0;

	//Wait one period
	read(timer_fd, &num_periods, sizeof(num_periods));

	int i = 0;
	while( i < 20 )
	{
		strcpy( info->lines[i], buffer );
		read(timer_fd, &num_periods, sizeof(num_periods));
		++i;
	}

    if( num_periods > 1 ) {
        puts("MISSED WINDOW");
        exit(1);
    }
}
    

int main( void )
{

	puts( "Starting!");

	read_file first_data = { 1, 0, 4000000, 0, 1000000, "first.txt" };
	pthread_t first;
	pthread_create( &first, NULL, (void *) read_to_buffer, (void *) &first_data );

	read_file second_data = { 2, 0, 4000000, 0, 3000000, "second.txt" };
	pthread_t second;
	pthread_create( &second, NULL, (void *) read_to_buffer, (void *) &second_data );

	add_to_array third_data = { 3, 0, 2000000, 0, 2000000 };
	pthread_t third;
	pthread_create( &third, NULL, (void *) buffer_to_array, (void *) &third_data );

	pthread_join( first, NULL );
	pthread_join( second, NULL );
	pthread_join( third, NULL );

	puts( "\nEverything: ");
	int i = 0;
	while( i < 20 )
	{
		printf( "%s", third_data.lines[i] );
		++i;
	}

	puts( "\n\nEnding!" );

	return 0;

}
