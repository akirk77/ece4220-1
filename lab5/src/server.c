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

#define MAX_MSG 40

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
	char name[12];
	strcpy( name, argv[2] );


	int sock;
	int length;
	int n;
	socklen_t fromlen;
	struct sockaddr_in server;
	struct sockaddr_in addr;
	char buffer[MAX_MSG];
	
	
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

	printf( "My Local IP is: %s\n\n", inet_ntoa( ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr ) );

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
		printf( "Command recieved from client: %s", buffer );
		
		if( !strcmp( buffer, "WHOIS\n" ) )
		{
			puts( "Doing a WHOIS" );
		}
		else if( !strcmp( buffer, "VOTE\n" ) )
		{
			puts( "WOW! It's time to elect another leader of the free world!" );
		}
	

		//printf( "Command recieved from client: %s", buffer );

		//Do comparisons here to decide what operation will be executed when each command is received.	
	
	}

	return 0;

}
