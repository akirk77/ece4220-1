/*
 ============================================================================
 Name        : Lab2w1.c
 Author      : Joel Abshier
 ============================================================================
 */

/*
 * 1. Load the data
 * 2. Get current time
 * 3. Create Threads
 * 4. Join Threads
 * 5. Get stop time
 * 6. Print Count and Ex Time
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

typedef struct
{
	int search;
	int data_matrix[20][100];
	int count[255];
	int rows;
	int cols;
} datas;

typedef struct
{
	int thread_id;
	datas * dat;
} thread;

void search_one( void * ptr )
{

	datas * dat = (datas*) ptr;

	printf( "\n\n>> Searching for %d with one thread.\n", (*dat).search );

	int i, j;
	int search = (*dat).search;
	int rows = (*dat).rows;
	int cols = (*dat).cols;
	for( i = 0; i < rows; i++ )
	{
		for( j = 0; j < cols; j++ )
		{
			if( (*dat).data_matrix[i][j] == search )
			{
				++(*dat).count[0];
			}
		}
	}

	printf( "\n\n>> Found %d instances of %d.\n", (*dat).count[0], search );

	//Kill thread!
	pthread_exit( 0 );

}


void search_rows( void * ptr )
{

	thread * thread_info = (thread * ) ptr;
	datas * dat = thread_info->dat;
	fflush( stdout );
	int id = thread_info->thread_id;

	int j;
	int search = (*dat).search;
	int cols = (*dat).cols;

	for( j = 0; j < cols; j++ )
	{
		if( (*dat).data_matrix[id][j] == search )
		{
			++(*dat).count[id];
		}
	}

	//Kill thread!
	pthread_exit( 0 );


}

void search_cols( void * ptr )
{

	thread * thread_info = (thread * ) ptr;
	datas * dat = thread_info->dat;
	int id = thread_info->thread_id;

	int j;
	int search = (*dat).search;
	int rows = (*dat).rows;

	for( j = 0; j < rows; j++ )
	{
		if( (*dat).data_matrix[j][id] == search )
		{
			++(*dat).count[id];
		}
	}

	//Kill thread!
	pthread_exit( 0 );

}

void search_elems( void * ptr )
{

	thread * thread_info = (thread * ) ptr;
	datas * dat = thread_info->dat;
	int id = thread_info->thread_id;

	int search = (*dat).search;
	int col = id / (*dat).rows;
	int row = id % (*dat).rows;

	//printf( "\nR: %d; C: %d; V: %d\n", row, col, (*dat).data_matrix[row][col] );

	if( (*dat).data_matrix[row][col] == search )
	{
		++(*dat).count[id];
	}

	//Kill thread!
	pthread_exit( 0 );

}



int main(int argc, char ** argv )
{
	if( argc != 3 )
	{
		puts( "Invalid command lind arguments: Use form \"./Run (filename) (int) \"" );
		return 1;
	}

	//Copy file name from command line argument
	char filename[100];
	strcpy( filename, argv[1] );
	printf( "\nThe file is: \"%s\"", filename );

	//Get search number from command line argument
	int searchnum = atoi( argv[2] );
	printf( "\nThe search term is: \"%d\"", searchnum );

	//Create data structure
	datas * decData = malloc( sizeof( datas ) );
	datas theData = *decData;
	theData.search = searchnum;


	//Open File for reading
	FILE * file;
	if( (file = fopen( filename, "r" ) ) == NULL )
	{
		puts( "\nThe file could not be open. Check your spelling!");

		return 0;
	}

	puts( "\nFile opened" );

	puts( "Reading rows and columns ");
	int rows, cols;

	fflush( stdout );
	fscanf( file, " %d", &rows );
	fscanf( file, " %d", &cols );

	theData.rows = rows;
	theData.cols = cols;


	puts( "Loop and load data!");
	int i,j;
	for( i = 0; i < rows; i++ )
	{
		//printf( "\n%2d: ", i );
		for( j = 0; j < cols; j++ )
		{
			fscanf( file, " %d", &theData.data_matrix[i][j] );
			//printf( "%5d", theData.data_matrix[i][j] );
		}
	}

	//Close the file
	fclose( file );

	//TESTING: Data read correctly
	/*printf( "\n" );
	for( i = 0; i < rows; i++ )
	{
		for( j = 0; j < cols; j++ )
		{
			printf( "%5d", theData.data_matrix[i][j] );
		}
		printf( "\n" );
	}*/

	//Timer structs
	struct timespec startTime, endTime;
	unsigned int diff;

	//START SEARCH WITH ONE THREAD
	clock_gettime( CLOCK_MONOTONIC, &startTime );

	pthread_t threadOne;
	pthread_create( &threadOne, NULL, (void *) &search_one, (void *) &theData);
	pthread_join( threadOne, NULL );

	clock_gettime( CLOCK_REALTIME, &endTime );
	diff = 1000000000 * (endTime.tv_sec - startTime.tv_sec) + endTime.tv_nsec - startTime.tv_nsec;
	printf( "\nOne Thread search finished in %u ns", diff );

	//END SEARCH WITH ONE THREAD

	//Reset count
	int w = 0;
	for( w = 0; w < 225; w++ ) { theData.count[w] = 0; }

	//START SEARCH WITH THREADS PER ROWS
	pthread_t threadRows[rows];
	thread thread_data[rows];

	clock_gettime( CLOCK_MONOTONIC, &startTime );

	int k;
	for( k = 0; k < rows; k++ )
	{
		//printf( "\n\nCreating Thread: %d", k );
		thread_data[k].thread_id = k;
		thread_data[k].dat = &theData;
		pthread_create( &threadRows[k], NULL, (void *) &search_rows, (void *) &thread_data[k] );
	}
	for( k = 0; k < rows; k++ )
	{
		pthread_join( threadRows[k], NULL );
	}

	clock_gettime( CLOCK_REALTIME, &endTime );
	diff = 1000000000 * (endTime.tv_sec - startTime.tv_sec) + endTime.tv_nsec - startTime.tv_nsec;
	printf( "\nThreads per rows search finished in %u ns", diff );


	int rowCount = 0;
	for( w = 0; w < 225; w++ ) { rowCount += theData.count[w]; }
	printf( "\n\n>> Found %d instances of %d in row search.\n", rowCount, theData.search );

	//END SEARCH WITH THREADS PER ROWS

	//Reset count
	for( w = 0; w < 225; w++ ) { theData.count[w] = 0; }


	//START SEARCH WITH THREADS PER COLUMNS
	pthread_t threadCols[cols];
	thread thread_datac[cols];

	clock_gettime( CLOCK_MONOTONIC, &startTime );
	for( k = 0; k < cols; k++ )
	{
		//printf( "\n\nCreating Thread: %d", k );
		thread_datac[k].thread_id = k;
		thread_datac[k].dat = &theData;
		pthread_create( &threadCols[k], NULL, (void *) &search_cols, (void *) &thread_datac[k] );
	}
	for( k = 0; k < cols; k++ )
	{
		pthread_join( threadCols[k], NULL );
	}

	clock_gettime( CLOCK_REALTIME, &endTime );
	diff = 1000000000 * (endTime.tv_sec - startTime.tv_sec) + endTime.tv_nsec - startTime.tv_nsec;
	printf( "\nThreads per column search finished in %u ns", diff );


	int colCount = 0;
	for( w = 0; w < 225; w++ ) { colCount += theData.count[w]; }
	printf( "\n\n>> Found %d instances of %d in column search.\n", colCount, theData.search );

	//END SEARCH WITH THREADS PER COLUMNS


	//Reset count
	for( w = 0; w < 225; w++ ) { theData.count[w] = 0; }


	//START SEARCH WITH THREADS PER ELEMENT
	int elems = cols * rows;
	pthread_t threadElems[elems];
	thread thread_datae[elems];

	clock_gettime( CLOCK_MONOTONIC, &startTime );
	for( k = 0; k < elems; k++ )
	{
		//printf( "\nCreating Thread: %d", k );
		thread_datae[k].thread_id = k;
		thread_datae[k].dat = &theData;
		pthread_create( &threadElems[k], NULL, (void *) &search_elems, (void *) &thread_datae[k] );
	}
	for( k = 0; k < elems; k++ )
	{
		pthread_join( threadElems[k], NULL );
	}

	clock_gettime( CLOCK_REALTIME, &endTime );
	diff = 1000000000 * (endTime.tv_sec - startTime.tv_sec) + endTime.tv_nsec - startTime.tv_nsec;
	printf( "\nThreads per element search finished in %u ns", diff );

	int elemCount = 0;
	for( w = 0; w < 225; w++ ) { elemCount += theData.count[w]; }
	printf( "\n\n>> Found %d instances of %d in element search.\n", elemCount, theData.search );

	//END SEARCH WITH THREADS PER ELEMENT


	return 0;
}
