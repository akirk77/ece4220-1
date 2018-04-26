/* Based on code found at https://gist.github.com/maggocnx/5946907
   Modified and commented by: Luis Rivera
   
   Compile using the Makefile
*/

#ifndef MODULE
#define MODULE
#endif
#ifndef __KERNEL__
#define __KERNEL__
#endif
   
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/string.h>

#define MSG_SIZE 50
#define CDEV_NAME "Lab6"

MODULE_LICENSE("GPL");

unsigned long timer_interval_ns = 750000;	// timer interval length (nano sec part)
static struct hrtimer hr_timer;			// timer structure
static int count = 0, dummy = 0;

int mydev_id;
unsigned long * selpin;
unsigned long * GP_SET;
unsigned long * GP_CLEAR;
unsigned long * GP_AREN;
unsigned long * GP_PUD;
unsigned long * GP_EVENT;

static int major;
static char msg[MSG_SIZE];
static char send[MSG_SIZE];

void changeSound( char newNote )
{
	static char lastNote;
	
//	printk( "\nChanging Note! %c -> %c", lastNote, newNote );

	if( newNote != lastNote )
	{

	        printk( "\nChanging Note! %c -> %c", lastNote, newNote );

                switch( newNote )
                {
                        case 'A': {
                                //A3
				if( timer_interval_ns != 4545454 ) {
                                        timer_interval_ns = 4545454;
                                        printk( "\nPlaying A3" );
				} break;
			}
                        case 'B': {
                                //B3
                                if( timer_interval_ns != 4048582 ) {
                                        timer_interval_ns = 4048582;
                                        printk( "\nPlaying B3" );
                                } break;
                        }
                        case 'C': {
                                //C3
                                if( timer_interval_ns != 3816794 ) {
                                        timer_interval_ns = 3816794;
                                        printk( "\nPlaying C4" );
                                } break;
                        }
                        case 'D': {
                                //D3
                                if( timer_interval_ns != 3000000 ) {
                                        timer_interval_ns = 3000000;
                                        printk( "\nPlaying D4" );
                                } break;
                        }
                        case 'E': {
                                //E3
                                if( timer_interval_ns != 2500000 ) {
                                        timer_interval_ns = 2500000;
                                	printk( "\nPlaying E4" );
                       		} break;
	                }
		}
	}

	lastNote = newNote;
}

static ssize_t device_read( struct file *filp, char __user * buffer, size_t length, loff_t *offset )
{
	ssize_t dummy = copy_to_user( buffer, send, length );

//	printk( "\nSent message to user: %s", buffer );

	send[0] = '\0';

	return length;

}

static ssize_t device_write( struct file * filp, const char __user *buff, size_t len, loff_t *off )
{

	ssize_t dummy;

	if( len > MSG_SIZE )
		return -EINVAL;

	dummy = copy_from_user( msg, buff, len );

	if( len == MSG_SIZE )
	{
		msg[len-1] = '\0';
	} else {
		msg[len] = '\0';
	}

	printk( "\nMessage from user space: %s", msg );

	char lastchar;

	if( msg[0] == '@' )
	{
		changeSound( msg[1] ); 
	}

	return len;

}

static struct file_operations fops =
{
        .read = device_read,
        .write = device_write,
};

//Interrupt Service
static irqreturn_t button_isr( int irq, void * dev_id )
{
	disable_irq_nosync( 79 );

	//Figure out which buttonw as pressed and handle sound

	unsigned long btn = *GP_EVENT & 0x1F0000 ;
	switch( btn )
	{
		case 65536:
			//A3
			timer_interval_ns = 4545454;
			strcpy( send, "@A" );
			printk( "\nPlaying A2" );
			break;
                case 131072:
                        //B3
                        timer_interval_ns = 4048582;
			strcpy( send, "@B" );
                        printk( "\nPlaying B3" );
                        break;
                case 262144:
                        //C4
                        timer_interval_ns = 3816794;
			strcpy( send, "@C" );
                        printk( "\nPlaying C4" );
                        break;
                case 524288:
                        //D4
                        timer_interval_ns = 3000000;
			strcpy( send, "@D" );
                        printk( "\nPlaying D4" );
                        break;
                case 1048576:
                        //E4
                        timer_interval_ns = 2500000;
			strcpy( send, "@E" );
                        printk( "\nPlaying E4" );
                        break;

			
	}

	//Clear Event Status Register
	iowrite32( ( *GP_EVENT | 0x1F0000 ), GP_EVENT );

	printk( "\nInterrupt Handled" );
	enable_irq( 79 );

	return IRQ_HANDLED;
}

// Timer callback function: this executes when the timer expires
enum hrtimer_restart timer_callback(struct hrtimer *timer_for_restart)
{

	//Keep an iteration count
	static int iter = 0;
	iter++;

	//Turn sound off/on depdending on odd/even iteration
	if( (iter % 2) == 1 )
	{
		iowrite32( ( *GP_SET | 0x00000044 ), GP_SET );
	} 
	else
	{
		iowrite32( ( *GP_CLEAR | 0x00000044 ), GP_CLEAR );
	}


  	ktime_t currtime, interval;	// time type, in nanoseconds
	unsigned long overruns = 0;
	
	// Re-configure the timer parameters (if needed/desired)
  	currtime  = ktime_get();
  	interval = ktime_set(0, timer_interval_ns); // (long sec, long nano_sec)
	
	// Advance the expiration time to the next interval. This returns how many
	// intervals have passed. More than 1 may happen if the system load is too high.
  	overruns = hrtimer_forward(timer_for_restart, currtime, interval);
	
	
	// The following printk only executes once every 1000 cycles.
	if(dummy == 0){
		printk("Count: %d, overruns: %ld\n", ++count, overruns);
	}
	dummy = (dummy + 1)%1000;
	
	
	return	 HRTIMER_RESTART;	// Return this value to restart the timer.
							// If you don't want/need a recurring timer, return
							// HRTIMER_NORESTART (and don't forward the timer).
}

int timer_init(void)
{
	int interrupt = 0;

	//Setup GPIO Stuff
	selpin = (unsigned long * ) ioremap( ( 0x3f200000 ), 4096 );
	GP_SET = selpin + 7;
	GP_CLEAR = selpin + 10;
	GP_AREN = selpin + 31;
	GP_PUD = selpin + 37;
	GP_EVENT = selpin + 16;

	//Prepare Pins for writing
	iowrite32( ( *selpin | 0x00040000 ), selpin );

	//Configure Button Pins as input
		iowrite32( ( *( selpin + 1 ) | 0x00000000 ), ( selpin + 1 ) );
		iowrite32( ( *( selpin + 2 ) | 0x00000000 ), ( selpin + 2 ) );
	//Configure Button Pins as input
		//iowrite32( 0x00000000, (selpin + 1) );
		//iowrite32( 0x00000000, (selpin + 2) );

	//Configure Pull-up/down control
	iowrite32( ( *GP_PUD | 0x155 ), GP_PUD );
	udelay( 100 ); // Wait time for setup of control signal

	//Set PUD clock
	iowrite32( ( *( GP_PUD + 1 ) | 0x001F0000 ), ( GP_PUD + 1 ) );
	udelay( 100 );

	//Undo the stuffs
	iowrite32( ( *GP_PUD & ~(0x155) ), GP_PUD );
	iowrite32( ( *( GP_PUD + 1 ) & ~(0x1F0000) ), ( GP_PUD + 1 ) );
	
	//Enable Async Rising Edge for buttons
	iowrite32( ( *GP_AREN | 0x1F0000 ), GP_AREN );

	//Setup the interrupt
	//int interrupt = 0;
	interrupt = request_irq( 79, button_isr, IRQF_SHARED, "Button_handler", &mydev_id );
	if( interrupt < 0 )
	{
		printk( "Interrupt failed to register" );
	}

	//SETUP the Characted Device
	major = register_chrdev( 0, CDEV_NAME, &fops );
	if( major < 0 ) 
	{
		printk( "Couldn't registeer the character device: %d\n", major );
		return major;
	}
	printk( "Create char device (node) with: sudo mknod /dev/%s c %d 0\n", CDEV_NAME, major );

	//Starting Message
	printk( "We're all setup and running!!!" );

	// Configure and initialize timer
	ktime_t ktime = ktime_set(0, timer_interval_ns); // (long sec, long nano_sec)
	
	// CLOCK_MONOTONIC: always move forward in time, even if system time changes
	// HRTIMER_MODE_REL: time relative to current time.
	hrtimer_init(&hr_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	
	// Attach callback function to the timer
	hr_timer.function = &timer_callback;
	
	// Start the timer
 	hrtimer_start(&hr_timer, ktime, HRTIMER_MODE_REL);
	
	return 0;
}

void timer_exit(void)
{

	//Disable ASync Rising Edge
	//iowrite32( ( *GP_AREN | 0x00000000 ), GP_AREN );
	//iowrite32( 0x00000000, GP_AREN );

	//Get rid of the interrupt handler
	free_irq( 79, &mydev_id );

	//Unregister Characted Device
	unregister_chrdev( major, CDEV_NAME );

	int ret;
  	ret = hrtimer_cancel(&hr_timer);	// cancels the timer.
  	if(ret)
		printk("The timer was still in use...\n");
	else
		printk("The timer was already canceled...\n");	// if not restarted or
														// canceled before
	
  	printk("HR Timer module uninstalling\n");
	
}

// Notice this alternative way to define your init_module()
// and cleanup_module(). "timer_init" will execute when you install your
// module. "timer_exit" will execute when you remove your module.
// You can give different names to those functions.
module_init(timer_init);
module_exit(timer_exit);
