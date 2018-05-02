#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timerfd.h>
#include <time.h>
#include <pthread.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>

#define MSG_SIZE 100

#define SPI_CHANNEL	      0	// 0 or 1
#define SPI_SPEED 	2000000	// Max speed is 3.6 MHz when VDD = 5 V
#define ADC_CHANNEL       2	// Between 0 and 3
struct timeval tv;

uint16_t get_ADC(int channel);

#define BUTTON1 27
#define BUTTON2 0

void* button1(void* ptr);
void* button2(void*);
double get_adc_value(void);
void send_to_socket(char IP[], char event[], double time, double voltage);
//void error(const char *argv[]);

int port = 0;

int sock, length, n;
int boolval = 1;			// for a socket option
socklen_t fromlen;
struct sockaddr_in server;
struct sockaddr_in addr;
char buffer[MSG_SIZE];



int main(int argc, char* argv[]){
	
	   if (argc < 2)
	   {
	      fprintf(stderr, "ERROR, no port provided\n");
	      exit(0);
	   }
	   port = atoi(argv[1]);
	   printf("%d\n", port);
	   

	
	//start getting ip address
	int fd;																			
	struct ifreq ifr;
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	char IP[13];

	/* I want to get an IPv4 IP address */
	ifr.ifr_addr.sa_family = AF_INET;

	/* I want IP address attached to "wlan0" */
	strncpy(ifr.ifr_name, "wlan0", IFNAMSIZ-1);

	ioctl(fd, SIOCGIFADDR, &ifr);

	close(fd);

	/* display result */
	strcpy(IP, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
	printf("My IP address is %s\n", IP);	
	//stop getting ip address

	
	//wiring pi setup
	wiringPiSetup();
	
	pinMode(BUTTON1, INPUT);
	pinMode(BUTTON2, INPUT);
	
	pullUpDnControl(BUTTON1, PUD_DOWN);
	pullUpDnControl(BUTTON2, PUD_DOWN);
	
	
	//threads to check for buttons
	pthread_t button1_thread;
	pthread_t button2_thread;
	//pthread_t wait_thread;
	
	pthread_create(&button1_thread, NULL, button1, (void*)&IP);
	pthread_create(&button2_thread, NULL, button2, NULL);
	//pthread_create(&wait_thread, NULL, wait, NULL);
	
	pthread_join(button1_thread, NULL);
	pthread_join(button2_thread, NULL);
	//pthread_join(wait_thread, NULL); 
	

	
	//create socket
		   /*int sock, length, n;
		   int boolval = 1;			// for a socket option
		   socklen_t fromlen;
		   struct sockaddr_in server;
		   struct sockaddr_in addr;
		   char buffer[MSG_SIZE];	// to store received messages or messages to be sent.
		   */


		   
		   sock = socket(AF_INET, SOCK_DGRAM, 0); // Creates socket. Connectionless.
		   if (sock < 0)
			   printf("did not open socket\n");

		   length = sizeof(server);			// length of structure
		   bzero(&server,length);			// sets all values to zero. memset() could be used
		   server.sin_family = AF_INET;		// symbol constant for Internet domain
		   server.sin_addr.s_addr = INADDR_ANY;		// IP address of the machine on which
		   server.sin_port = htons(port);	// the server is running
		   
		   

		   //server.sin_port = htons(port);	// port number

		   // binds the socket to the address of the host and the port number
		   if (bind(sock, (struct sockaddr *)&server, length) < 0)
		       printf("binding error\n");

		   // change socket permissions to allow broadcast
		   if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &boolval, sizeof(boolval)) < 0)
		   	{
		   		printf("error setting socket options\n");
		   		exit;
		   	}

		   fromlen = sizeof(struct sockaddr_in);	// size of structure

		
		   
		   
			// bzero: to "clean up" the buffer. The messages aren't always the same length...
			bzero(buffer,MSG_SIZE);		// sets all values to zero. memset() could be used	
			
		       n = recvfrom(sock, buffer, MSG_SIZE, 0, (struct sockaddr *)&addr, &fromlen);
		       if (n < 0)
		    	   error("recvfrom");

			   printf("Received a datagram: %s\n", buffer);
			
			
			
return 0;
}

void* button1(void* Darth_Vader){
	int thing1 = 0;
	double timestamp = 0;
	char event_type[5] = "BTN1";
	double ADC = 0;
	
	char* IP = (char*)Darth_Vader;
	
	
	while(1){
		while(thing1 == 0){
			thing1 = digitalRead(BUTTON1);
		}
		if(thing1 == 1){
			
			//send to socket
			
			//get time of day
			gettimeofday(&tv, NULL);
			timestamp = (double)(tv.tv_sec + 0.000001 * tv.tv_usec);
			printf("%lf\n", timestamp);
			
			//event type
			//event_type = "BTN1";
			printf("%s\n", event_type);
			
			//IP addresss
			//char IP[13] inherited from main
			printf("%s\n", IP);
			
			//get adc value
			ADC = get_adc_value();
			printf("%lf\n", ADC);
			
			//send to socket
			send_to_socket(IP, event_type, timestamp, ADC);
			
			
			thing1 = 0;
		}
		pullUpDnControl(BUTTON1, PUD_DOWN);		
		usleep(200000);
	}
	pthread_exit(0);
}

void* button2(void* Yoda){
	int thing2 = 0;
	
	while(1){
		while(thing2 == 0){
			thing2 = digitalRead(BUTTON2);
		}
		if(thing2 == 1){
			printf("button 2 was pushed\n");
			thing2 = 0;
		}
		pullUpDnControl(BUTTON2, PUD_DOWN);		
		usleep(200000);
	}

	pthread_exit(0);
}

double get_adc_value(void){
	
    uint16_t ADCvalue;
	
	// Configure the SPI
	if(wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED) < 0) {
		printf("wiringPiSPISetup failed\n");
		return -1 ;
	}

	{
		double ADCvalue = get_ADC(ADC_CHANNEL);
		ADCvalue = (double)ADCvalue / (double)1023;
		ADCvalue = (double)ADCvalue * (double)3.3;
		//printf("ADC Value: %lf\n", ADCvalue);
		fflush(stdout);
		usleep(10000);
		return ADCvalue;
	}
  //return (double)ADCvalue; 
}
uint16_t get_ADC(int ADC_chan)
{
	uint8_t spiData[3];
	spiData[0] = 0b00000001; // Contains the Start Bit
	spiData[1] = 0b10000000 | (ADC_chan << 4);	// Mode and Channel: M0XX0000
												// M = 1 ==> single ended
									// XX: channel selection: 00, 01, 10 or 11
	spiData[2] = 0;	// "Don't care", this value doesn't matter.
	
	// The next function performs a simultaneous write/read transaction over the selected
	// SPI bus. Data that was in the spiData buffer is overwritten by data returned from
	// the SPI bus.
	wiringPiSPIDataRW(SPI_CHANNEL, spiData, 3);
	
	// spiData[1] and spiData[2] now have the result (2 bits and 8 bits, respectively)
	
	return ((spiData[1] << 8) | spiData[2]);
}

void send_to_socket(char IP[], char event[], double time, double voltage){
	
	char string_to_send[MSG_SIZE];
	
	
	sprintf(string_to_send, "$ %s %s %lf %lf", IP, event, time, voltage);
	printf("%s\n", string_to_send);
	

	
/*	   int sock, length, n;
	   int boolval = 1;			// for a socket option
	   socklen_t fromlen;
	   struct sockaddr_in server;
	   struct sockaddr_in addr;
	   char buffer[MSG_SIZE];	// to store received messages or messages to be sent.
	   


	   
	   sock = socket(AF_INET, SOCK_DGRAM, 0); // Creates socket. Connectionless.
	   if (sock < 0)
		   printf("did not open socket\n");

	   length = sizeof(server);			// length of structure
	   bzero(&server,length);			// sets all values to zero. memset() could be used
	   server.sin_family = AF_INET;		// symbol constant for Internet domain
	   server.sin_addr.s_addr = INADDR_ANY;		// IP address of the machine on which
	   server.sin_port = htons(port);	// the server is running
	   

	   server.sin_port = htons(port);	// port number

	   // binds the socket to the address of the host and the port number
	   if (bind(sock, (struct sockaddr *)&server, length) < 0)
	       printf("binding error\n");

	   // change socket permissions to allow broadcast
	   if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &boolval, sizeof(boolval)) < 0)
	   	{
	   		printf("error setting socket options\n");
	   		exit;
	   	}

	   fromlen = sizeof(struct sockaddr_in);	// size of structure

	   
	   
		   // bzero: to "clean up" the buffer. The messages aren't always the same length...
		   bzero(buffer,MSG_SIZE);		// sets all values to zero. memset() could be used
*/
		   bzero(string_to_send, MSG_SIZE);
		   addr.sin_addr.s_addr = inet_addr("128.206.19.255");
		   n = sendto(sock, string_to_send, MSG_SIZE, 0, (struct sockaddr *)&addr, fromlen);
		   if(n < 0)
			   printf("nope\n");
	
	   
}










