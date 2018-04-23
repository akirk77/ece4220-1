#obj-m += kthread_test.o
#obj-m += hrtimer_test.o
obj-m += lab6pt2.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
