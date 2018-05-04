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


#define BUTTON1 27			//sets pins of buttons, leds, and switches on auxilliary board
#define BUTTON2 0
#define LED1 8
#define LED2 9
#define LED3 7
#define SWITCH1 26
#define SWITCH2 23

void* button1(void* ptr);
void* button2(void* ptr);
void* wait_for_message(void* ptr);
double get_adc_value(void);
void send_to_socket(char IP[], char event[], double time, double voltage);
void* check_adc(void* ptr);
void* switch1(void* ptr);
void* switch2(void* ptr);
void LED_error_message_on(void);
void LED_error_message_off(void);
//void get_min_time(char IP[]);

int port = 0;
int sock, length, n;		//global variables to be used by the socket
int boolval = 1;			//this is so the socket can be used in multiple functions/threads
socklen_t fromlen;
struct sockaddr_in server;
struct sockaddr_in addr;
char buffer[MSG_SIZE];



int main(int argc, char* argv[]){
	
	if (argc < 2)				//make sure the port number is provided
	{
	   fprintf(stderr, "ERROR, no port provided\n");
	   return 3;
	}
	port = atoi(argv[1]);

	
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
	//stop getting ip address



	//set up socket
	sock = socket(AF_INET, SOCK_DGRAM, 0); // Creates socket. Connectionless.
	if (sock < 0)
		printf("did not open socket\n");

	length = sizeof(server);			// length of structure
	bzero(&server,length);			// sets all values to zero. memset() could be used
	server.sin_family = AF_INET;		// symbol constant for Internet domain
	server.sin_addr.s_addr = INADDR_ANY;		// IP address of the machine on which
	server.sin_port = htons(port);	// the server is running

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
		printf("error receiving\n");
	//socket is set up

	
	//wiring pi setup
	wiringPiSetup();
	
	pinMode(BUTTON1, INPUT);
	pinMode(BUTTON2, INPUT);
	pinMode(LED1, OUTPUT);
	pinMode(LED2, OUTPUT);
	pinMode(LED3, OUTPUT);
	pinMode(SWITCH1, INPUT);
	pinMode(SWITCH2, INPUT);
	
	pullUpDnControl(BUTTON1, PUD_DOWN);
	pullUpDnControl(BUTTON2, PUD_DOWN);
	digitalWrite(LED1, LOW);
	digitalWrite(LED2, LOW);
	digitalWrite(LED3, LOW);
	
	//get_min_time(IP);			//function used to find minimum time between events

	

	//threads to check for events
	pthread_t button1_thread;
	pthread_t button2_thread;
	pthread_t receive_thread;
	pthread_t check_adc_thread;
	pthread_t switch1_thread;
	pthread_t switch2_thread;
	
	pthread_create(&button1_thread, NULL, button1, (void*)&IP);
	pthread_create(&button2_thread, NULL, button2, (void*)&IP);
	pthread_create(&receive_thread, NULL, wait_for_message, (void*)&IP);
	pthread_create(&check_adc_thread, NULL, check_adc, (void*)&IP);
	pthread_create(&switch1_thread, NULL, switch1, (void*)&IP);
	pthread_create(&switch2_thread, NULL, switch2, (void*)&IP);
	
	pthread_join(button1_thread, NULL);
	pthread_join(button2_thread, NULL);
	pthread_join(receive_thread, NULL);
	pthread_join(check_adc_thread, NULL);
	pthread_join(switch1_thread, NULL);
	pthread_join(switch2_thread, NULL);
			
return 0;
}

/*void get_min_time(char address[]){		//funtion used to find the minimum time allowed
											//between events
	char IP[13];							//this is the last testing scenario in the document
	strcpy(IP, address);
	double timestamp = 0;
	double ADC = 0;
	char event_type1[5] = "1__1";
	char event_type2[5] = "2__2";

	while(1){

		gettimeofday(&tv, NULL);
		timestamp = (double)(tv.tv_sec + 0.000001 * tv.tv_usec);

		ADC = get_adc_value();

		send_to_socket(IP, event_type1, timestamp, ADC);

		usleep(1000);

		gettimeofday(&tv, NULL);
		timestamp = (double)(tv.tv_sec + 0.000001 * tv.tv_usec);

		ADC = get_adc_value();

		send_to_socket(IP, event_type2, timestamp, ADC);

		usleep(1000);


	}

}
*/

void* button1(void* Darth_Vader){		//thread that checks button 1

	int thing1 = 0;						//dummy variable
	double timestamp = 0;
	char event_type[5] = "BTN1";		//sets event unique to button 1
	double ADC = 0;
	
	char* IP = (char*)Darth_Vader;		//gets IP from main
	
	while(1){
		while(digitalRead(BUTTON1) == 1){		//if you hold the button down, do nothing
												//after the first event is sent
		}
		while(thing1 == 0){					//waits for button to be pushed
			thing1 = digitalRead(BUTTON1);
		}
		if(thing1 == 1){					//when button is pushed, the event is recorded

			//gets time of event
			gettimeofday(&tv, NULL);
			timestamp = (double)(tv.tv_sec + 0.000001 * tv.tv_usec);	//combines into one variable
			
			//get adc value
			ADC = get_adc_value();
			
			//send to socket
			send_to_socket(IP, event_type, timestamp, ADC);
			
			thing1 = 0;						//resets the dummy variable
		}
	}
	pthread_exit(0);
}

void* button2(void* Yoda){

	int thing2 = 0;
	double timestamp = 0;
	char event_type[5] = "BTN2";
	double ADC = 0;

	char* IP = (char*)Yoda;
	
	while(1){
		while(digitalRead(BUTTON2) == 1){

		}
		while(thing2 == 0){
			thing2 = digitalRead(BUTTON2);
		}
		if(thing2 == 1){

			//get time of day
			gettimeofday(&tv, NULL);
			timestamp = (double)(tv.tv_sec + 0.000001 * tv.tv_usec);

			ADC = get_adc_value();

			send_to_socket(IP, event_type, timestamp, ADC);


			thing2 = 0;
		}
	}
	pthread_exit(0);
}

void* switch1(void* Darth_Tyrannus){

	char* IP = (char*)Darth_Tyrannus;
	int initial = digitalRead(SWITCH1);		//checks initial switch value to determine
	int final = initial;					//if it was changed
	double timestamp = 0;
	double ADC = 0;
	char event_type[5] = "SW_1";			//sets event unique to switch 1

	//printf("%d %d\n", initial, final);

	while(1){
		while(final == initial){				//waits for the switch to be flipped
			final = digitalRead(SWITCH1);
		}
		if(final != initial){				//when the switch changes, record the event

			//get timestamp
			gettimeofday(&tv, NULL);
			timestamp = (double)(tv.tv_sec + 0.000001 * tv.tv_usec);

			//get adc value
			ADC = get_adc_value();

			//send information through socket
			send_to_socket(IP, event_type, timestamp, ADC);

		}
		initial = digitalRead(SWITCH1);			//gets new initial value for comparison
		final = initial;
	}
}

void* switch2(void* Darth_Maul){

	char* IP = (char*)Darth_Maul;
	int initial = digitalRead(SWITCH2);
	int final = initial;
	double timestamp = 0;
	double ADC = 0;
	char event_type[5] = "SW_2";


	while(1){
		while(final == initial){
			final = digitalRead(SWITCH2);
		}
		if(final != initial){

			gettimeofday(&tv, NULL);
			timestamp = (double)(tv.tv_sec + 0.000001 * tv.tv_usec);

			ADC = get_adc_value();

			send_to_socket(IP, event_type, timestamp, ADC);

		}
		initial = digitalRead(SWITCH2);
		final = initial;
	}
}

void* check_adc(void* Darth_Sidious){

	char* IP = (char*)Darth_Sidious;
	int x = 0;
	double value[3];
	int no_power = 0;
	int high_power = 0;
	double value2 = 0;
	char event_type1[5] = "NO_V";
	char event_type2[5] = "HI_V";
	double timestamp = 0;
	double adc_to_send = 0;

	while(1){					//constantly get three values of adc
		for(x=0;x<3;x++){
			value[x] = get_adc_value();
			usleep(10000);
		}
		x = x - 1;

		//if there is no power
		if(value[x] == value[x-1] && value[x-1] == value[x-2]){
			//printf("no power dude\n");

			//this is the event
			gettimeofday(&tv, NULL);
			timestamp = (double)(tv.tv_sec + 0.000001 * tv.tv_usec);

			send_to_socket(IP, event_type1, timestamp, adc_to_send);

			while(no_power != 1){				//waits for power to be turned back on
				value2 = get_adc_value();
				if(value2 != value[x]){
					//printf("power is back on\n");
					no_power = 1;
				}
			}
		}
		no_power = 0;

		//if power is too high (value > 2volts)
		if(value[x] > 2 && value[x-1] > 2 && value[x-2] > 2){
			//printf("power is too high\n");

			//event
			while(high_power != 1){
				value2 = get_adc_value();
				if(value2 < 2){
					//printf("power is back in range\n");
					high_power = 1;
				}
			}
		}
		high_power = 0;

	}
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
		ADCvalue = (double)ADCvalue / (double)1023;		//converts adc value to voltage
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

															//receives event information to send to socket
void send_to_socket(char IP[], char event[], double time, double voltage){
	
	char string_to_send[MSG_SIZE];		//creates new string
	bzero(string_to_send,MSG_SIZE);		//clears string. just in case
	
	//put all information in one string
	sprintf(string_to_send, "$ %s %s %lf %lf", IP, event, time, voltage);
	

	//send string through socket
	n = sendto(sock, string_to_send, MSG_SIZE, 0, (struct sockaddr *)&addr, fromlen);
	//check if string was sent
	if(n < 0){
		printf("error sending event string\n");
	}
	   
}

void* wait_for_message(void* tattooine){

	char message[MSG_SIZE];
	char event_type[5];
	double timestamp = 0;
	double ADC = 0;
	int flag = 0;

	char* IP = (char*)tattooine;



	//printf("%c%c", IP[11], IP[12]);

	while(1){
		bzero(message, MSG_SIZE);
		n = recvfrom(sock, message, MSG_SIZE, 0, (struct sockaddr *)&addr, &fromlen);
		if (n < 0)
			printf("error receiving\n");

		if(message[0] == '#'){

			if(message[2] == IP[11] && message[3] == IP[12]){
				flag = 0;

				if(message[8] == '1'){		//red LED

					if(message[10] == 'O' && message[11] == 'N'){		//if message is "on"
						if(digitalRead(LED1) == 0){						//make sure led is off
							digitalWrite(LED1, HIGH);					//turn on led
							gettimeofday(&tv, NULL);					//record event
							timestamp = (double)(tv.tv_sec + 0.000001 * tv.tv_usec);
							ADC = get_adc_value();
							strcpy(event_type, "RED1");
							flag = 1;
						}
						else if(digitalRead(LED1) == 1){			//if already on
							LED_error_message_on();					//send error message
						}
					}
					if(message[10] == 'O' && message[11] == 'F' && message[11] == 'F'){	//if message is "off"
						if(digitalRead(LED1) == 1){						//make sure led is on
							digitalWrite(LED1, LOW);					//turn off led
							gettimeofday(&tv, NULL);
							timestamp = (double)(tv.tv_sec + 0.000001 * tv.tv_usec);
							ADC = get_adc_value();
							strcpy(event_type, "RED0");
							flag = 1;
						}
						else if(digitalRead(LED1) == 0){				//if already off
							LED_error_message_off();					//send other error message
						}
					}
					if(flag == 1){									//if event was recorded
						send_to_socket(IP, event_type, timestamp, ADC);	//send event through socket
					}
				}
				if(message[8] == '2'){
					//yellow LED
					if(message[10] == 'O' && message[11] == 'N'){
						if(digitalRead(LED2) == 0){
							digitalWrite(LED2, HIGH);
							gettimeofday(&tv, NULL);
							timestamp = (double)(tv.tv_sec + 0.000001 * tv.tv_usec);
							ADC = get_adc_value();
							strcpy(event_type, "YEL1");
							flag = 1;
						}
						else if(digitalRead(LED2) == 1){
							LED_error_message_on();
						}
					}
					if(message[10] == 'O' && message[11] == 'F' && message[11] == 'F'){
						if(digitalRead(LED2) == 1){
							digitalWrite(LED2, LOW);
							gettimeofday(&tv, NULL);
							timestamp = (double)(tv.tv_sec + 0.000001 * tv.tv_usec);
							ADC = get_adc_value();
							strcpy(event_type, "YEL0");
							flag = 1;
						}
						else if(digitalRead(LED2) == 0){
							LED_error_message_off();
						}
					}
					if(flag == 1){
						send_to_socket(IP, event_type, timestamp, ADC);
					}
				}
				if(message[8] == '3'){
					//green LED
					if(message[10] == 'O' && message[11] == 'N'){
						if(digitalRead(LED3) == 0){
							digitalWrite(LED3, HIGH);
							gettimeofday(&tv, NULL);
							timestamp = (double)(tv.tv_sec + 0.000001 * tv.tv_usec);
							ADC = get_adc_value();
							strcpy(event_type, "GRE1");
							flag = 1;
						}
						else if(digitalRead(LED3) == 1){
							LED_error_message_on();
						}
					}
					if(message[10] == 'O' && message[11] == 'F' && message[11] == 'F'){
						if(digitalRead(LED3) == 1){
							digitalWrite(LED3, LOW);
							gettimeofday(&tv, NULL);
							timestamp = (double)(tv.tv_sec + 0.000001 * tv.tv_usec);
							ADC = get_adc_value();
							strcpy(event_type, "GRE0");
							flag = 1;
						}
						else if(digitalRead(LED3) == 0){
							LED_error_message_off();
						}
					}
					if(flag == 1){
						send_to_socket(IP, event_type, timestamp, ADC);
					}
				}
			}
		}
		usleep(100000);
	}
	pthread_exit(0);
}

void LED_error_message_on(void){			//sending message if led is already on

	char string_to_send[MSG_SIZE] = "! LED is already on\n";		//sets string to send

	//sends string through socket
	n = sendto(sock, string_to_send, MSG_SIZE, 0, (struct sockaddr *)&addr, fromlen);

	//checks of string was sent
	if(n < 0)
		printf("error sending on string\n");
}

void LED_error_message_off(void){			//send message if led is already off

	char string_to_send[MSG_SIZE] = "! LED is already off\n";

	n = sendto(sock, string_to_send, MSG_SIZE, 0, (struct sockaddr *)&addr, fromlen);
	if(n < 0)
		printf("error sending off\n");
}









