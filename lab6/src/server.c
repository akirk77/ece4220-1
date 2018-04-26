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
#include <fcntl.h>
#include <pthread.h>
#include <sched.h>

#define MAX_MSG 40

#define CHAR_DEV "/dev/Lab6"

int sock;
struct sockaddr_in server;
struct sockaddr_in addr;
char name[12];
char * myIP;
socklen_t fromlen;

void wait4btn( void * ptr );

void error( const char *msg )
{
	perror( msg );
	exit( 0 );
}

int main( int argc, char * argv[] )
{

	if( argc != 3 )
	{
		puts( "Use as follows: /server [PORT#] [NAME]" );
		return 1;
	}
	
	int port = atoi( argv[1] );
	//char name[12];
	strcpy( name, argv[2] );


	//int sock;
	int length;
	int n;
	//socklen_t fromlen;
	//struct sockaddr_in server;
	//struct sockaddr_in addr;
//	struct sockaddr_in rec;
	char buffer[MAX_MSG];
	int I_THE_MASTER = 0;
	int cuteNumber;	
	
	sock = socket( AF_INET, SOCK_DGRAM, 0 );
	if( sock < 0 ) {
		error( "Opening socket" );
	}

	length = sizeof( server );
	bzero( &server, length );
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( port );

	if( bind(   sock, (struct sockaddr *) &server, length ) < 0 ) {
		error( "binding" );
	}

	fromlen= sizeof( struct sockaddr_in );

	struct ifreq ifr;
	ifr.ifr_addr.sa_family = AF_INET;
	snprintf( ifr.ifr_name, IFNAMSIZ, "wlan0" );
	ioctl( sock, SIOCGIFADDR, &ifr );	

	myIP = malloc( sizeof(char) * 16 );
	myIP = inet_ntoa( ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr );
	printf( "My Local IP is: %s\n\n", inet_ntoa( ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr ) );

	//Setup broadcasting
	int broadcast = 1;
	addr.sin_addr.s_addr = inet_addr( "128.206.19.255" );
	addr.sin_port = htons( port );
	if( setsockopt( sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast) ) < 0 )
        {
        	printf( "Broadcast failure" );
	        exit( -1 );
        }

	//Create thread to listen for button input
	pthread_t btnThread;
	pthread_create( &btnThread, NULL, (void *) wait4btn, (void *) &sock );

	while( 1 )
	{	

		bzero( buffer, MAX_MSG );
			

		n = recvfrom( sock, buffer, MAX_MSG, 0, (struct sockaddr *) &addr, &fromlen );
		if( n < 0 ) {
			error( "Receive from" );
		}	

		int action = 0;
		/*
		Compare the command with different options and set the action to a value, if it has one. otherwise action will remain 0, which will cause no action to happen.
		*/
		printf( "\nReceived :: %s\n", buffer );
	
		fflush( stdout );	
		if( !strcmp( buffer, "WHOIS\n" ) )
		{
			puts( "Doing WHOIS" );
			if( I_THE_MASTER == 1 )
			{
				puts("I the master" );
					
				char mastery[MAX_MSG];
				sprintf( mastery, "%s on board %s is master!", name, myIP );
				
				//addr.sin_addr.s_addr = inet_addr( "128.206.19.255" );
				sendto( sock, mastery, MAX_MSG, 0, (const struct sockaddr *)&addr, fromlen );
			}
		}
		else if( !strcmp( buffer, "VOTE\n" ) )
		{
			puts( "WOW! It's time to elect another leader of the free world!" );

		        cuteNumber = rand() % 10;
                        char voteReturn[18];
                        sprintf( voteReturn, "# %s %d", myIP, cuteNumber);

			addr.sin_addr.s_addr = inet_addr( "128.206.19.255" );
                        n = sendto( sock, voteReturn, MAX_MSG, 0, (const struct sockaddr *)&addr, fromlen );
			if( n < 0 )
			{
				error( "Sending Vote" );
			}

//	                recvfrom( sock, buffer, MAX_MSG, 0, (struct sockaddr *) &addr, &fromlen );
//			printf( "\nReceived :: %s", buffer );


			I_THE_MASTER = 1;
		}
		else if( buffer[0] == '#' )
		{
	
			unsigned int cNum;
			unsigned int cIp;
      
			unsigned int myLastAddr;
      			sscanf( myIP, "%*u.%*u.%*u.%u %u", &myLastAddr );
			
			sscanf( buffer, "# %*u.%*u.%*u.%u %u",  &cIp, &cNum );
			printf( "\nVote recieved with :: Ip: %u | Num : %u | myIPL : %u | myNUM : %d\n", cIp, cNum, myLastAddr, cuteNumber );
      
		      if( cNum < cuteNumber )
		      {
		        I_THE_MASTER = 1;
		      }
		      else if( cNum == cuteNumber ) 
		      {
		        if( cIp > myLastAddr ) {
		          I_THE_MASTER = 0;
		        } else {                  
		          I_THE_MASTER = 1;
		        }
		      }
		      else
		      {
		        I_THE_MASTER = 0;
		      }

		}
		else if( buffer[0] == '@' )
		{
		
			int cdev_id, dummy;
			static char lastChar;
			
			if( lastChar != buffer[1] )
			{
		
				if( ( cdev_id = open( CHAR_DEV, O_WRONLY ) ) == -1 )
				{
					printf( "Cannot open device %s\n", CHAR_DEV );
				}
			
				if( buffer[1] != '\0' )
				{
					dummy = write( cdev_id, buffer, sizeof( buffer ) );
				}

				close( cdev_id );

				if( I_THE_MASTER )
				{
					//Forward the note to all the other boards!
	                	         addr.sin_addr.s_addr = inet_addr( "128.206.19.255" );
        	                	n = sendto( sock, buffer, MAX_MSG, 0, (const struct sockaddr *) & addr, fromlen );
		                        if( n < 0 )
        	        	        {
        		                        error( "Broadcasting other note" );
	                	        }
	
				}	
	
			}

			lastChar = buffer[1];

		}

		//printf( "Command recieved from client: %s", buffer );

		//Do comparisons here to decide what operation will be executed when each command is received.	
	
	}

	return 0;

}

void wait4btn( void * ptr )
{

	//puts( "Button Press Thread Started" );

	//Open the character device
	int cdev_id, dummy;
	char buffer[MAX_MSG];

	if((cdev_id = open( CHAR_DEV, O_RDONLY )) == -1 )
	{
		printf( "Cannot open device %d\n", CHAR_DEV );
	}

	while(1)
	{
		//puts( "Started btnWait loop" );

		dummy = read( cdev_id, buffer, sizeof(buffer) );
		if( dummy != sizeof(buffer) )
		{
			printf( "Read Failed" );
			break;
		}
		else if( buffer[0] != '\0' )
		{	
			printf( "\nRecieved Button pressed: %s", buffer );
			buffer[0] = '\0';

			printf( "\nSending Command: @%c to %s", buffer[1], myIP );
	
                        char send2Self[MAX_MSG];
                        sprintf( send2Self, "@%c", buffer[1] );

                        addr.sin_addr.s_addr = inet_addr( myIP );
                        sendto( sock, send2Self, MAX_MSG, 0, (const struct sockaddr *)&addr, fromlen );

		}
	
		usleep( 200 );	

	}	

}
