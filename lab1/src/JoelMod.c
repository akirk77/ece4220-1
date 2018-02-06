#include <linux/printk.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/io.h>

#ifndef JOELMOD
#define JOELMOD
#endif

#ifndef __KERNEL__
#define __KERNEL__
#endif

MODULE_LICENSE( "GPL" );
MODULE_AUTHOR( "Joel Abshier" );


int joelmod_init( void )
{
	//Map the location of the GPIO pins in memory
	unsigned long * selpin = (unsigned long*) ioremap( (0x3F200000), 4096 );
	
	//Set the pin modes to output for each LED
	iowrite32( (*selpin | 0x00049240) , selpin );

	//Move the Pin pointer forward 7 * (4 bits) to the pins for setting 1 
	selpin = selpin + 7;
	
	//Set the write pin to 1
   	iowrite32( (0x0000003C), selpin );

	//Output niceness
    	printk( "JoelMod Installed\n\n" );

    	return 0;
}

void joelmod_cleanup( void )
{
	//Map the location of the GPIO pins in memory
	unsigned long * selpin = (unsigned long*) ioremap( (0x3F200000), 4096 );
    	
	//Set the pin modes to output for each LED
	iowrite32( (*selpin | 0x00049240) , selpin );

	//Move the pin pointer forware 10 * (4 bits) to the pins for clearing 1
    	selpin = (selpin + 10);
	
	//Set the clear pin to 0
    	iowrite32( 0x0000003C, selpin );

	//Output niceness
    	printk( "JoelMod Removed\n\n" );
}

//Make the mode init and exit functions link to mine.
module_init( joelmod_init );
module_exit( joelmod_cleanup );
