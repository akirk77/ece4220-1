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
	unsigned long * selpin = (unsigned long*) ioremap( (0x3F200000), 4096 );
	iowrite32( (*selpin | 0x00049240) , selpin );

	selpin = selpin + 7;
    iowrite32( (0x0000003C), selpin );

    printk( "JoelMod Installed\n\n" );

    return 0;
}

void joelmod_cleanup( void )
{
	unsigned long * selpin = (unsigned long*) ioremap( (0x3F200000), 4096 );
    iowrite32( (*selpin | 0x00049240) , selpin );

    selpin = (selpin + 10);
    iowrite32( 0x0000003C, selpin );

    printk( "JoelMod Removed\n\n" );
}

module_init( joelmod_init );
module_exit( joelmod_cleanup );
