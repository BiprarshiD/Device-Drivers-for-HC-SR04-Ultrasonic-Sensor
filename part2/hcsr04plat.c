#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/semaphore.h>
#include<linux/init.h>
#include<linux/moduleparam.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>    						
#include <linux/gpio.h>      						
#include <linux/interrupt.h> 						
#include <linux/time.h>
#include <linux/unistd.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/jiffies.h>
#include <asm/div64.h>
#include <asm/delay.h>
#include <linux/stat.h>
#include <linux/math64.h>
#include <linux/unistd.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/ktime.h>
#include <linux/hrtimer.h>
#include <linux/platform_device.h>


#include "hcsr04.h"

//For taking number of devices input from the user
static short int Device = 1;
module_param(Device, short, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
int Devices;

//file operation functions declared
static int __init hcsr04_plat_init(void);
static void __exit hcsr04_plat_exit(void);


//For creating multiple devices
struct hcsr04_dev **hcsr04_devps;

static void hcsr04_plat_release(struct device *dev)
{

}

//initialize the platform device structure
int hcsr04_plat_init(void)
{
	int i;
	//int ret;
	char hcsr04_name[15];
	char hcsr04_plt_name[15];
        char x[4];
	Devices = (int)Device;

	
	//allocate memory 
	hcsr04_devps = kmalloc(sizeof(struct hcsr04_dev *)*Devices, GFP_KERNEL);	

	hcsr04_drvp = kmalloc(sizeof(struct hcsr04_drv), GFP_KERNEL);
	if(!hcsr04_drvp) {
		printk(KERN_INFO "Bad Kmalloc for Driver\n");
	}

	for (i=0;i<Devices;i++)
	{
		spin_lock_init(&hcsr04_devps[i]->l);  //initialize the spin lock
		sprintf(x, "%d", i);
		sprintf(hcsr04_name,"HCSR04_%s",x);
		sprintf(hcsr04_plt_name,"HCSR04_plt_%s", x);
		hcsr04_devps[i] = kmalloc(sizeof(struct hcsr04_dev), GFP_KERNEL);  //allocate memory to the device structure
		if(!hcsr04_devps[i])
		{
			printk(KERN_INFO "Bad Kmalloc for device\n");
			return -ENOMEM;
		}
		
		memset(hcsr04_devps[i], 0, sizeof(struct hcsr04_dev));

		hcsr04_devps[i]->hcsr04_device.name = hcsr04_plt_name;  //define name for the device
		hcsr04_devps[i]->hcsr04_device.hcsr04_plt_dev.name = hcsr04_name;  //define name for the platform
		hcsr04_devps[i]->hcsr04_device.hcsr04_plt_dev.id = -1;
		hcsr04_devps[i]->hcsr04_device.dev_minor = -1;
		hcsr04_devps[i]->hcsr04_device.dev_num = i+10;
		hcsr04_devps[i]->hcsr04_device.hcsr04_plt_dev.dev.release = hcsr04_plat_release;
		
		platform_device_register(&hcsr04_devps[i]->hcsr04_device.hcsr04_plt_dev);	//register the platform device
 

		printk("%s initialized\n",hcsr04_plt_name);
	}
		//link the platform devices in the list

		i--;
		hcsr04_devps[i]->hcsr04_device.next = NULL;

		while(i)
		{
			hcsr04_devps[i-1]->hcsr04_device.next = &hcsr04_devps[i]->hcsr04_device;
			i=i-1;
		}
		hcsr04_drvp->head = &hcsr04_devps[i]->hcsr04_device;
		return 0;
}


//exit function for the platform device
void hcsr04_plat_exit(void)
{
	int i;

	for(i=Device-1; i>=0; i--)
	{
		platform_device_unregister(&hcsr04_devps[i]->hcsr04_device.hcsr04_plt_dev);		//unregister the platform device
		hcsr04_devps[i]->hcsr04_device.next = NULL;  //empty the list of devices
		
	}

	hcsr04_drvp->head = NULL;  //empty the head 
	kfree(hcsr04_devps);
	kfree(hcsr04_drvp);  //free the hcsr04_drv structure

}

module_init(hcsr04_plat_init);
module_exit(hcsr04_plat_exit);
MODULE_LICENSE("GPL");



