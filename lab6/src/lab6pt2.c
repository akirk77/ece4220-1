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
#include <asm/io.h>

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

//Interrupt Service
static irqreturn_t button_isr( int irq, void * dev_id )
{
	disable_irq_nosync( 79 );

	//Figure out which buttonw as pressed and handle sound

	printk( "Interrupt Handled" );
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
	
	
	return HRTIMER_RESTART;	// Return this value to restart the timer.
							// If you don't want/need a recurring timer, return
							// HRTIMER_NORESTART (and don't forward the timer).
}

int timer_init(void)
{
	//Setup GPIO Stuff
	selpin = (unsigned long * ) ioremap( ( 0x3f200000 ), 4096 );
	GP_SET = selpin + 7;
	GP_CLEAR = selpin + 10;
	GP_AREN = selpin + 31;
	GP_PUD = selpin + 37;

	//Prepare Pins for writing
	iowrite32( ( *selpin | 0x00040040 ), selpin );
	iowrite32( ( *selpin | 0x00000044 ), selpin );

	//Configure Button Pins as input
	iowrite32( ( *( selpin + 1 ) | 0x00000000 ), ( selpin + 1 ) );
	iowrite32( ( *( selpin + 2 ) | 0x00000000 ), ( selpin + 2 ) );

	//Configure Pull-up/down control
	iowrite32( ( *GP_PUD | 0x00000001 ), GP_PUD );
	udelay( 100 ); // Wait time for setup of control signal
	//Set PUD clock
	iowrite32( ( *( GP_PUD + 1 ) | 0x001F0000 ), ( GP_PUD + 1 ) );
	udelay( 100 );
	iowrite32( ( *GP_PUD | 0x00000000 ), GP_PUD );
	iowrite32( ( *( GP_PUD + 1 ) | 0x00000000 ), ( GP_PUD + 1 ) );
	
	//Enable Async Rising Edge for buttons
	iowrite32( ( *GP_AREN | 0x001F0000 ), GP_AREN );

	//Setup the interrupt
	int interrupt = 0;
	interrupt = request_irq( 79, button_isr, IRQF_SHARED, "Button_handler", &mydev_id );

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
	//Get rid of the interrupt handler
	free_irq( 79, &mydev_id );

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
