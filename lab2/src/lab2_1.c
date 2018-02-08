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

typedef struct
{
	int search;
	int data_matrix[20][100];
	int count[255];
} datas;

void search_one( void * ptr )
{

	datas * dat = (datas*) ptr;

	printf( "\n\n>> Searching for %d with one thread.\n", (*dat).search );

	//Kill thread!
	pthread_exit( 0 );

}


void search_rows( void * ptr )
{

	//datas * dat = (datas*) ptr;


}

void search_cols( void * ptr )
{

	//datas * dat = (datas*) ptr;

}

void search_elems( void * ptr )
{

	//datas * dat = (datas*) ptr;

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
	int rows = 20;
	int cols = 100;

	puts( "Read rows");
	fflush( stdout );
	fscanf( file, " %d", &rows );
	puts( "Read cols");
	fscanf( file, " %d", &cols );


	/*puts( "Loop and load data!");
	fflush( stdout );
	int i,j;
	for( i = 0; i < rows; i++ )
	{
		printf( "\n%d: ", i );
		for( j = 0; j < cols; j++ )
		{
			printf( "%d ", j );
			fscanf( file, " %d", &theData.data_matrix[i][j] );
		}
	}*/

	//Close the file
	fclose( file );

	//TESTING: Data read correctly
	/*printf( "\n" );
	for( i = 0; i < rows; i++ )
	{
		for( j = 0; j < cols; j++ )
		{
			printf( "%d ", theData.data_matrix[i][j] );
		}
		printf( "\n" );
	}*/

	//START SEARCH WITH ONE THREAD
	pthread_t threadOne;
	pthread_create( &threadOne, NULL, (void *) &search_one, (void *) &theData);
	pthread_join( threadOne, NULL );

	//END SEARCH WITH ONE THREAD


	//START SEARCH WITH THREADS PER ROWS
	pthread_t threadRows;
	pthread_create( &threadRows, NULL, (void *) &search_rows, (void *) &theData);
	pthread_join( threadRows, NULL );

	//END SEARCH WITH THREADS PER ROWS


	//START SEARCH WITH THREADS PER COLUMNS
	pthread_t threadCols;
	pthread_create( &threadCols, NULL, (void *) &search_cols, (void *) &theData);
	pthread_join( threadCols, NULL );

	//END SEARCH WITH THREADS PER COLUMNS


	//START SEARCH WITH THREADS PER ELEMENT
	pthread_t threadElems;
	pthread_create( &threadElems, NULL, (void *) &search_elems, (void *) &theData);
	pthread_join( threadElems, NULL );

	//END SEARCH WITH THREADS PER ELEMENT


	return 0;
}
