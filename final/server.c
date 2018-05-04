#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#define MAX_MSG 100

typedef struct command_
{
	char ip[40];
	char desc[40];
	double voltage;
	double timestamp;
	struct command_ * next;
} Command, *pCommand;


void error( const char *msg )
{
	perror( msg );
	exit( 0 );
}

void writeList( int, Command * head );
void freeList( Command * head );
void printList( Command * head );
void sendCommand( void * ptr );

void sortedInsert( Command**, Command* );
void insertionSort( Command ** );

int sock;
struct sockaddr_in server, anybody;
socklen_t fromlen;
char buffer[MAX_MSG];

int main( int argc, char * argv[] )
{

	if( argc != 2 )
	{
		puts( "Use as follows: /server [PORT#]" );
		return 1;
	}
	
	int port = atoi( argv[1] );
	char name[12];
	//strcpy( name, argv[2] );


	//int sock;
	int length;
	int n;
	//socklen_t fromlen;
	//struct sockaddr_in server;
	//struct sockaddr_in anybody;
	//struct sockaddr_in rec;
	//char buffer[MAX_MSG];
	int I_THE_MASTER = 0;
	int cuteNumber;	

	struct hostent *hp;
	
	sock = socket( AF_INET, SOCK_DGRAM, 0 );
	if( sock < 0 ) {
		error( "Opening socket" );
	}

	int boolval = 1;
	if( setsockopt( sock, SOL_SOCKET, SO_BROADCAST, &boolval, sizeof(boolval) ) < 0 )
	{
		puts( "error setting scok et options" );
		exit(-1);
	}

	anybody.sin_family = AF_INET;
	anybody.sin_port = htons( port );
	anybody.sin_addr.s_addr = inet_addr( "128.206.19.255" );

	fromlen= sizeof( struct sockaddr_in );



	strcpy( buffer, "Setting things up..." );
	
	n = sendto( sock, buffer, sizeof(buffer), 0, (struct sockaddr *) &anybody, fromlen );
	if( n < 0 )
	{
		puts( "\nMESSGE NTO SENT" );
	}

	puts( "\n WHILE IS STARTING" );


	//Start PThread for Sending
	pthread_t commandThread;
	pthread_create( &commandThread, NULL, (void *) sendCommand, (void*) NULL  );

	while( 1 )
	{	

		bzero( buffer, MAX_MSG );
			

		n = recvfrom( sock, buffer, MAX_MSG, 0, (struct sockaddr *) &server, &fromlen );
		if( n < 0 ) {
			error( "Receive from" );
		}	

		int action = 0;
		/*
		Compare the command with different options and set the action to a value, if it has one. otherwise action will remain 0, which will cause no action to happen.
		*/
		printf( "\nReceived :: %s\n", buffer );
	
		fflush( stdout );	
		
		if( buffer[0] == '#' )
		{
			}
		else if( buffer[0] == '$' )
		{
			puts( "Final Project" );
			
			double voltage;
			double ts;
			char command[40];
			char ip[40];
		
			sscanf( buffer, "$ %s %s %lf %lf", ip, command, &voltage, &ts );

			printf( "\nCommand: %s\nVoltage: %lf\nTimestamp: %lf\n", command, voltage, ts );

			Command * head = NULL;
			Command * prevCmd = NULL;
			Command * cmd;
	
			int records = 0;
			FILE * fp;
			fp = fopen( "events.txt", "r" );			
	
			if( fp != NULL )
			{			
				fscanf( fp, "%d\n", &records );
				printf( "\nThere are %d records!", records );
				
				//Command * head = NULL;
				//Command * prevCmd = NULL;
				//Command * cmd;
				char end = 0;

				int loop = records;
				while( loop > 0 )
				{
					Command * cmd = malloc( sizeof( Command ) );
					cmd->next = NULL;
					if( loop == records ) { head = cmd; }

					end = fscanf( fp, "$ %s %s %lf %lf\n", &cmd->ip, &cmd->desc, &cmd->voltage, &cmd->timestamp );

					printf( "\nRead values: %.20s, %.10s, %.6lf, %.6u", cmd->ip, cmd->desc, cmd->voltage, cmd->timestamp );
					
					if( prevCmd != NULL )
					{
						//printf( "\nPointed %p to %p", prevCmd, cmd );
						prevCmd->next = cmd;
					}
					prevCmd = cmd;

					--loop;
				}

			}
			else
			{
				//If No events.txt file exists, create it and put INIT command in there.	
			}
	
			fclose( fp );

			puts( "\nAdding new value to list" );

			//Add Element sent by server to list
			records++;
                        Command * newCmd = malloc( sizeof(Command) );
                        newCmd->next = NULL;
                        //cmd->desc = command;
			strcpy( newCmd->ip, ip );
			strcpy( newCmd->desc, command );
			newCmd->voltage = voltage;
			newCmd->timestamp = ts;

			prevCmd->next = newCmd;

			puts("\n\nTHESE VALUES EXIST: " );

			printList( head );                       
			sortList( &head );
			writeList( records, head );
			freeList( head );

		}

		//printf( "Command recieved from client: %s", buffer );

		//Do comparisons here to decide what operation will be executed when each command is received.	
	
	}

	return 0;

}

void insertionSort( Command ** head_ref )
{
    Command * sorted = NULL;
    Command *current = *head_ref;
    
    while (current != NULL)
    {
        Command * next = current->next;
        sortedInsert(&sorted, current);
        current = next;
    }
 
    *head_ref = sorted;
}
 
void sortedInsert( Command** head_ref, Command* new_node)
{
    Command * current;
    if (*head_ref == NULL || (*head_ref)->data >= new_node->data)
    {
        new_node->next = *head_ref;
        *head_ref = new_node;
    }
    else
    {
        current = * head_ref;
        while (current->next!=NULL &&
               current->next->data < new_node->data)
        {
            current = current->next;
        }
        new_node->next = current->next;
        current->next = new_node;
    }
}

void sendCommand( void * ptr )
{
	int n;

	while( 1 )
	{
	
		char * inBuffer[MAX_MSG];
		puts( "Input your message: " );
		scanf( "%s", inBuffer );
    
    n = sendto( sock, buffer, sizeof(buffer), 0, (struct sockaddr *) &anybody, fromlen );
    

	}	

	return;
}

void printList( Command * head )
{

	if( head == NULL )
	{
		return;
	}
	
	printList( head->next );
	usleep( 50000 );
	
	puts( head->desc );
}

void freeList( Command * head )
{

        if( head == NULL )
        {
                return;
        }

        freeList( head->next );
        usleep( 50000 );

	printf( "\nFreeing: %s", head->desc );
	free( head );

}

void writeList( int records, Command * head )
{

	puts( "Writing list back to file" );
                                         
  FILE * fp;
  if( access( "events.txt", F_OK ) != -1 ) 
  {    
    fp = fopen( "events.txt", "w" );
  }
  else
  {
    fp = fopen( "events.txt", "w" );
  }
	Command * cmd = head;
	if( fp != NULL )
	{
		fprintf( fp, "%d\n", records );
		while( cmd != NULL )
		{
			fprintf( fp, "$ %s %s %lf %lf\n", cmd->ip, cmd->desc, cmd->voltage, cmd->timestamp );
			cmd = cmd->next;		
		}
	}
	fclose( fp );        
	
}
