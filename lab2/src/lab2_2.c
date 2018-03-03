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

  //Make the thread realtime by prioritizing it above the kernel
	param.sched_priority = RT_PRIORITY;
	sched_setscheduler(0, SCHED_FIFO, &param);
	
  //Set parameters for period of clock
  itval.it_interval.tv_sec = info->per_sec;
  itval.it_interval.tv_nsec = info->per_nsec;
  itval.it_value.tv_sec = info->start_sec;
  itval.it_value.tv_nsec = info->start_nsec;


  //Open File
	FILE * file;
	if( (file = fopen( info->filename, "r" ) ) == NULL )
	{
		//If the file couldn't be opened, end the application
    puts( "\nThe file could not be open. Check your spelling!");
		exit(1);
	}

	//Start the timer
	timerfd_settime(timer_fd, 0, &itval, NULL);

  //For error checking
	uint64_t num_periods = 0;

	//Wait one period
	read(timer_fd, &num_periods, sizeof(num_periods));

  //Loop until the file ends
	while( !feof(file) )
	{
    //Read the line from the file into the buffer
		fgets( buffer, BUFF_SIZE, file );
    //Wait a period
		read(timer_fd, &num_periods, sizeof(num_periods));
	}

  //Close the file
	fclose( file );

  //If something went wrong with reading let's over react and close the program out!
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

  //Make it realtime (ahead of the kernel)
	param.sched_priority = RT_PRIORITY;
	sched_setscheduler(0, SCHED_FIFO, &param);

  //Set parameters for timing
  itval.it_interval.tv_sec = info->per_sec;
  itval.it_interval.tv_nsec = info->per_nsec;
  itval.it_value.tv_sec = info->start_sec;
  itval.it_value.tv_nsec = info->start_nsec;

  //Skip the first period
	timerfd_settime(timer_fd, 0, &itval, NULL);

	//Track number of periods for error checking later	
	uint64_t num_periods = 0;

	//Wait one period
	read(timer_fd, &num_periods, sizeof(num_periods));

  //Loop for maximum possible lines and read from the buffer!
	int i = 0;
	while( i < 20 )
	{
		//Copy the value in the buffer into the array
    strcpy( info->lines[i], buffer );
    
    //Wait period
		read(timer_fd, &num_periods, sizeof(num_periods));
		
    //Next                    
    ++i;
	}

  //If something went really wrong!
    if( num_periods > 1 ) {
        puts("MISSED WINDOW");
        exit(1);
    }
}
    

int main( void )
{

	//Get things going!
  puts( "Starting!");

  //Create the first thread's data and then create the thread with a pointer to that struct. This will read the first.txt file
	read_file first_data = { 1, 0, 4000000, 0, 1000000, "first.txt" };
	pthread_t first;
	pthread_create( &first, NULL, (void *) read_to_buffer, (void *) &first_data );
                                                                                                                              
  //Create the second thread's data and then create the thread with a pointer to that struct. This will read the second.txt file
	read_file second_data = { 2, 0, 4000000, 0, 3000000, "second.txt" };
	pthread_t second;
	pthread_create( &second, NULL, (void *) read_to_buffer, (void *) &second_data );

  //Create the third thread's data structure and then create the thread. This will copy the string from the buffer into the big 'ol array of strings
	add_to_array third_data = { 3, 0, 2000000, 0, 2000000 };
	pthread_t third;
	pthread_create( &third, NULL, (void *) buffer_to_array, (void *) &third_data );

  //Wait for every thread to finish!!
	pthread_join( first, NULL );
	pthread_join( second, NULL );
	pthread_join( third, NULL );

  //Print out every line in the third_data array
	puts( "\nEverything: ");
	int i = 0;
	while( i < 20 )
	{
		printf( "%s", third_data.lines[i] );
		++i;
	}

  //We all done
	puts( "\n\nEnding!" );

  //Now we done for realzies
	return 0;

}
