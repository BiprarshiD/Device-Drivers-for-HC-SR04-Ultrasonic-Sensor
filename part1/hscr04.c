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

#include "hcsr04_header.h"


//global variables declared
//ktime_t rising, falling;
int speed = 340;
long long unsigned distance = 0;
long period;
unsigned int irql;
unsigned long f;
int Devices;
//int flag_delay=1;
uint64_t rising, falling;


//For taking number of devices input from the user
static short int Device = 1;
module_param(Device, short, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);


//file operation functions declared
static int __init hcsr04_init(void);
static void __exit hcsr04_exit(void);
static int hcsr04_open(struct inode *, struct file *);
static int hcsr04_release(struct inode *, struct file *);
static int hcsr04_write(struct file *, const char *, size_t, loff_t *);
static ssize_t hcsr04_read(struct file *, char *, size_t, loff_t *);
static long hcsr04_ioctl(struct file *, unsigned int , unsigned long);

//For creating multiple devices
struct hcsr04_dev **hcsr04_devps;

//structure for echo, trigger, number of samples and sample period defined
struct hcsr04_config
{
	int echo;
	int echopin;
	int trigger;
	int triggerpin;
	int sample;
	int sample_period;
} hcsr04_configs;


//Structure for fifo buffer
struct hcsr04_fifo_buf {

	unsigned long long int TSC;        //time stamp counter
	unsigned long long int size;       //distance recorded 
}hcsr04_fifo_bufs;	

//Structure for the device defined
struct hcsr04_dev{
   /*
  struct miscdevice{
	int 	minor
  	const char * 	name
 	struct file_operations * 	fops
 	struct list_head 	list
 	struct device * 	parent
 	struct device * 	this_device
 	const char * 	nodename
 	umode_t 	mode
   }
   */
	struct miscdevice miscdevice;                           
	struct hcsr04_dev *next;
	struct hcsr04_config hcsr04_configs;
	int c;
	int head;
	int counter;
	struct hcsr04_fifo_buf hcsr04_fifo_bufs[5];
	int m;              //indicates distance calculation is ongoing
	spinlock_t l;
	
} *hcsr04_devp;

//For appending and accessing multiple devices in the various functions
struct hcsr04_drv{

	struct hcsr04_dev *head;
	//struct hcsr04_dev *curr;
}*hcsr04_drvp;


//Time stamp counter defined , this function returns the current timestamp
static __inline__ unsigned long long RDTSC(void)
{
  unsigned hi, lo;
  __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
  return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}


//Configuration setting for the pins,  this configures and sets up the gpio modes of echo and trigger pins as per application
void config_set( int gpioA, int gpioB, int direction, int f, int pinmuxA, int first, int pinmuxB, int last)
{
	if(!f)
	{
		if(pinmuxA != -1)
		{
			gpio_request(pinmuxA, "muxSelect1");
			gpio_set_value_cansleep(pinmuxA, first);
		}
		if(gpioB != -1)
		{
			gpio_request(gpioB, "shiftReg");
			if(direction)
				gpio_direction_input(gpioB);
			else
				gpio_direction_output(gpioB, direction);
		}
		if(pinmuxB != -1)
		{
			gpio_request(pinmuxB, "muxSelect2");
			gpio_set_value_cansleep(pinmuxB, last);
		}

		gpio_request(gpioA, "IOpin");
		if(direction)
		{
			gpio_direction_input(gpioA);
		}
		else
		{
			gpio_set_value_cansleep(gpioA, 0);
			gpio_direction_output(gpioA, direction);
		}
	}
	else
	{
		if(pinmuxA != -1)
			gpio_free(pinmuxA);
		if(gpioB != -1)
			gpio_free(gpioB);
		if(pinmuxB != -1)
			gpio_free(pinmuxB);

		gpio_free(gpioA);
	}
} 

//configures the gpio pins as per application ; pin multiplexing ; this configures and sets up the gpio modes of echo and trigger pins as per application
int config_pin(int io, int gpio_pin, int f)
{
	int pin;
	switch(gpio_pin)
	{
		case 0:
			config_set(11, 32, io, f, -1, -1, -1, -1);
			pin = 11;
			break;
		case 1:
			config_set(12, 28, io, f, 45, 0, -1, -1);
			pin = 12;
			break;
		case 2:
			config_set(13, 34, io, f, 77, 0, -1, -1);
			pin = 13;
			break;
		case 3:
			config_set(14, 16, io, f, 76, 0, 64, 0);
			pin = 14;
			break;
		case 4:
			config_set(6, 36, io, f, -1, -1, -1, -1);
			pin = 6;
			break;
		case 5:
			config_set(0, 18, io, f, 66, 0 , -1, -1);
			pin = 0;
			break;
		case 6:
			config_set(1, 20, io, f, 68, 0, -1, -1);
			pin = 1;
			break;
		case 7:
			config_set(38, -1, io, f, -1, -1, -1, -1);
			pin = 38;
			break;
		case 8:
			config_set(40, -1, io, f, -1, -1, -1, -1);
			pin = 40;
			break;
		case 9:
			config_set(4, 22, io, f, 70, 0, -1, -1);
			pin = 4;
			break;
		case 10:
			config_set(10, 26, io, f, 74, 0, -1, -1);
			pin = 10;
			break;
		case 11:
			config_set(5, 24, io, f, 44, 0, 72, 0);
			pin = 5;
			break;
		case 12:
			config_set(15, 42, io, f, -1, -1, -1, -1);
			pin = 15;
			break;
		case 13:
			config_set(7, 30, io, f, 46, 0, -1, -1);
			pin = 7;
			break;
		case 14:
			config_set(48, -1, io, f, -1, -1, -1, -1);
			pin = 48;
			break;
		case 15:
			config_set(50, -1, io, f, -1, -1, -1, -1);
			pin = 50;
			break;
		case 16:
			config_set(52, -1, io, f, -1, -1, -1, -1);
			pin = 52;
			break;
		case 17:
			config_set(54, -1, io, f, -1, -1, -1, -1);
			pin = 54;
			break;
		case 18:
			config_set(56, -1, io, f, 60, 1, 78, 1);
			pin = 56;
			break;
		case 19:
			config_set(58, -1, io, f, 60, 1, 79, 1);
			pin = 58;
			break;
		default:
			printk("HCSR04: Wrong pin number entered");
			pin = -1;
			break;
	}
	return pin;
} 


//To open the device driver
int hcsr04_open(struct inode *inode, struct file *file)
{
	int minor = iminor(inode);
	struct hcsr04_dev *hcsr04_devp = hcsr04_drvp->head;  //pointing to the first device
	const struct file_operations *hcsr04_fops = NULL;
	
	//Assign respective file operations for different devices
	while(hcsr04_devp->next != NULL)
	{
		if(hcsr04_devp->miscdevice.minor == minor)
		{
			hcsr04_fops = fops_get(hcsr04_devp->miscdevice.fops);
			break;
		}
		hcsr04_devp = hcsr04_devp->next;
	}
	//Store the current used device in the structure for easy access in the functions defined below
	//hcsr04_drvp->curr = hcsr04_devp;

	file->private_data = hcsr04_devp;
	printk("HCSR04: Device is opening\n");

	return 0;

}

//close the device driver , release is called when the device count becomes 0
int hcsr04_release(struct inode *inode, struct file *file)
{
	struct hcsr04_dev *hcsr04_devp = file->private_data;
	//Free the echo and trigger pins
	config_pin(-1, hcsr04_devp->hcsr04_configs.echopin, 1);
	config_pin(-1, hcsr04_devp->hcsr04_configs.triggerpin, 1);
	//Free irq
	free_irq(gpio_to_irq(hcsr04_devp->hcsr04_configs.echo), (void *)hcsr04_devp);

	printk("HCSR04: Device is closing\n");

	return 0;
}

//Function to write in the buffer;  write the Time Stamp and the distance to the Fifo buffer
void hcsr04_buf_write(unsigned long long int tsc, unsigned long long int size, struct hcsr04_dev *hcsr04_devp)
{
	//Access the current device structure
	//struct hcsr04_dev *hcsr04_devp = hcsr04_drvp->curr;
	
	int a;
	a = hcsr04_devp->c;  //access counter for the buffer , c is the counter of the fifo buffer
	a=a+1;
	//storing only latest 5 values in the fifo buffer
	if (a >= 5)
	{
		a=0;
	}
	if (a == hcsr04_devp->head)
	{
		hcsr04_devp->head++;
		if(hcsr04_devp->head >= 5)
			hcsr04_devp->head=0;
	}
	//printk("TSC inside write buffer:%llu\n",tsc);
	//Store the Time Stamp and distance in the buffer
	hcsr04_devp->hcsr04_fifo_bufs[hcsr04_devp->c].size=size;   //store the distance measured by distancecalc()
	hcsr04_devp->hcsr04_fifo_bufs[hcsr04_devp->c].TSC=tsc;     //store the timestamp
	hcsr04_devp->c=a;											//update the counter i.e fifo buffer index
}

//Retrieve the distance and Time Stamp from the buffer of the current device
int hcsr04_buf_read(struct hcsr04_fifo_buf *buf, struct hcsr04_dev *hcsr04_devp)
{
	//Access the current device structure
	//struct hcsr04_dev *hcsr04_devp = hcsr04_drvp->curr;
	int a;
	//printk("TSC outside if of inside read buffer:%llu\n",hcsr04_devp->hcsr04_fifo_bufs[hcsr04_devp->head].TSC);
	if (hcsr04_devp->head == hcsr04_devp->c)
	{
		//printk("TSC inside if of read buffer:%llu\n",hcsr04_devp->hcsr04_fifo_bufs[hcsr04_devp->head].TSC);
		return -1;
	}
	
	a = hcsr04_devp->head + 1;           //save the new head of fifo buffer for next read
	if(a >= 5)
	{
		a=0;
	}
	//printk("TSC inside read buffer:%llu\n",hcsr04_devp->hcsr04_fifo_bufs[hcsr04_devp->head].TSC);
	//Retrieve the distance and Time Stamp from the buffer
	buf->size=hcsr04_devp->hcsr04_fifo_bufs[hcsr04_devp->head].size;
	buf->TSC=hcsr04_devp->hcsr04_fifo_bufs[hcsr04_devp->head].TSC;
	hcsr04_devp->head = a;   //update head of fifo buffer after reading the value into buf
	return 0;
}

//Two options are given either you can use rdtsc or ktime to measure distance
//By Default rdtsc is taken to calculate the value
//To use ktime change the rising and falling variables to ktime datatype and  uncomment the ktime comments and comment out the remaining rdtsc operations.
static irq_handler_t hcsr04_irq_handler(unsigned int irq, void *dev_id)
{
	long long unsigned dist;
	long long unsigned hcsr04_period;
	//Access the current device structure
	struct hcsr04_dev* hcsr04_devp = (struct hcsr04_dev *)dev_id;
	//get the value of the echo pin
	int value=gpio_get_value(hcsr04_devp->hcsr04_configs.echo);
	//Rising edge
	if(value)
	{
		//ktime:
		//rising=ktime_set(0,0);
		//rising=ktime_get();
		rising = RDTSC();
		
	}
	//Falling edge
	else
	{
		falling = RDTSC();
		//ktime:
		//falling=ktime_set(0,0);
		//falling=ktime_get();
		hcsr04_period = falling - rising;
		
		spin_lock_irqsave(&hcsr04_devp->l,f);

		//period = (long)hcsr04_period;
		//ktime:
		//hcsr04_period = ktime_to_ms(ktime_sub(falling,rising));  //get the time in ms
		dist = hcsr04_period*speed;
		distance = div_u64(dist, 8000000);  //as board frequency is 400 MHz and we need to divide by 2 to measure the distance as echo travels back and forth
		//Ktime:
		//distance = div_u64(dist, 34);
		spin_unlock_irqrestore(&hcsr04_devp->l,f);
	}
	
	return (irq_handler_t) IRQ_HANDLED;
}

//Calculate the distance; trigger the HCSR04 and store the distance measured into the fifo buffer
void distance_calc(struct hcsr04_dev *hcsr04_devp)
{
	unsigned long long sum=0;
	unsigned long long first=0;
	unsigned long long last=80000000;
	int i;
	hcsr04_devp->m=1;      //m=1 denotes distance calculation starts
	//printk("HCSR04: Outside FOR");
	for (i=0; i<hcsr04_devp->hcsr04_configs.sample + 2; i++)
	{
		//printk("HCSR04: Inside FOR");
		//trigger the pin
		gpio_set_value_cansleep(hcsr04_devp->hcsr04_configs.trigger, 0);
		udelay(2);
		gpio_set_value_cansleep(hcsr04_devp->hcsr04_configs.trigger, 1);
		//Setting 1 to greater than 10 to triggger
		udelay(15);
		gpio_set_value_cansleep(hcsr04_devp->hcsr04_configs.trigger, 0);
		mdelay(1);
		sum = sum + distance;      // we get this distance from hcsr04_irq_handler
		//Find the first and the last values
		if(distance > first)
			first = distance;       //we ignore the first and last distance measured for smoothing
		if(distance < last)
			last = distance;		//we ignore the first and last distance measured for smoothing

		msleep(hcsr04_devp->hcsr04_configs.sample_period);        //delay between adjacent samples

	}

	sum = sum - first -last;  //exclude the first and the last values
	do_div(sum, hcsr04_devp->hcsr04_configs.sample);  //Find the average distance
	hcsr04_buf_write(RDTSC(), sum, hcsr04_devp); //write the Time Stamp and the distance to the Fifo buffer 
	hcsr04_devp->m=0;           //m=0 denotes distance calculation ends
	//printk("HCSR04: RDTSC:%llu\n",RDTSC());
	//printk("HCSR04: Distance Found\n");
}

//Read file operation
ssize_t hcsr04_read(struct file *file, char *buffer, size_t count, loff_t *ppos)
{
	int bytes_read = 0;
	struct hcsr04_fifo_buf *buf;
	struct hcsr04_dev *hcsr04_devp = file->private_data;
	struct hcsr04_fifo_buf *hcsr04_fifo_bufs = (struct hcsr04_fifo_buf * )kmalloc(sizeof(struct hcsr04_fifo_buf), GFP_KERNEL);  //Allocate memory for the Fifo buffer
	
	buf = (struct hcsr04_fifo_buf *)buffer;
	while(hcsr04_buf_read(hcsr04_fifo_bufs, hcsr04_devp) == -1); //read the Fifo buffer
	copy_to_user(buf, hcsr04_fifo_bufs, sizeof(struct hcsr04_fifo_buf)); //write the data to the user space
	kfree(hcsr04_fifo_bufs); //deallocate the memory for the Fifo buffer
	return bytes_read;
}

//Write File operation - called from user space
int hcsr04_write(struct file *file, const char *buffer, size_t count, loff_t *ppos)
{
	int * data;
	struct hcsr04_dev *hcsr04_devp = file->private_data;
	int arg;
	data = (int *)buffer;
	get_user(arg, data); //get the dat from the user space
	//If the calculation of distance is still going on return EINVAL
	if(hcsr04_devp->m == 1)
	{
		return -EINVAL;
	}
	else
	{
		if(arg != 0)  //If the data from the user space is non-zero then clear the buffer
		{
			hcsr04_devp->head=0;         //restore head of fifo buffer to 0
			hcsr04_devp->c=0;			//reset fifo index counter to 0
		}
		distance_calc(hcsr04_devp);          //trigger writing the fifo buffer based on fresh sensor readings
		//printk("HCSR04:Distance calculated\n");
	}
		return 0;
}

//IOCTL File operation
static long hcsr04_ioctl(struct file *file, unsigned int ioctl_num, unsigned long val)
{
	struct data *data1;
	struct hcsr04_dev *hcsr04_devp = file->private_data;
	int ret, output;
	data1 = (struct data* )val; 		//store the argument to ioctl fn call in data1
	switch(ioctl_num)
	{
		case SET_PARAMETERS:  //setting the parameters i.e number of samples and sampling time
		{
			if(data1->val2<=0 || data1->val1<=1) //return einval if the number of samples is less than or equal to 1 and the sample period is less than or equal to 0
			{
				return -EINVAL;
			}
			hcsr04_devp->hcsr04_configs.sample = data1->val1;               //store no. of samples in device pointer
			hcsr04_devp->hcsr04_configs.sample_period = data1->val2;		//store sampling time in device pointer
		}
		break;
		
		case CONFIG_PINS:   //Pin Multiplexing
		{
			if((data1->val1 >= 0 && data1->val1 <=19) && ((data1->val2 >= 2 && data1->val2 <=6) || (data1->val2 >= 13 && data1->val2 <=19) || data1->val2 == 9 || data1->val2 == 11))
			{
				hcsr04_devp->hcsr04_configs.echo = config_pin(1, data1->val2, 0);        //configure echo pin as input pin
				hcsr04_devp->hcsr04_configs.echopin = data1->val2;                       //store gpio pin no. for echo pin
				hcsr04_devp->hcsr04_configs.trigger = config_pin(0, data1->val1, 0);  	 //configure trigger pin as output pin
				hcsr04_devp->hcsr04_configs.triggerpin = data1->val1; 					 //store gpio pin no. for trigger pin
			}
			else
			{
				return -EINVAL;
			}

			if((irql = gpio_to_irq(hcsr04_devp->hcsr04_configs.echo)) < 0)  //get the irq from the echo pin
			{
				printk(KERN_INFO "HCSR04:IRQ Failed");
			}
			output = request_irq(irql, (irq_handler_t) hcsr04_irq_handler, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING , "irq", (void *)hcsr04_devp);  //request irq
			if(output < 0)
			{
				printk(KERN_INFO "HCSR04:IRQ Request Failed\n");
				if (output == -EBUSY)
					ret = output;
				else
					ret = -EINVAL;
				return ret;
			}
			

		}
		break;

		default:
			printk("HCSR04: Wrong Input\n");
	}
	return 0;
	
}
		
//File operations structure defined
static struct file_operations hcsr04_fops = {
	.owner	 	 = THIS_MODULE,
	.open	 	 = hcsr04_open,
	.release 	 = hcsr04_release,
	.write		 = hcsr04_write,
	.read		 = hcsr04_read,
	.unlocked_ioctl  = hcsr04_ioctl
};

//Initialization function
int __init hcsr04_init(void)
{
	int i;
	int ret;
	char hcsr04_name[15];
    char x[4];
	Devices = (int)Device;

	//allocating 2d array memory to the columns of the device structure
	hcsr04_devps = kmalloc(sizeof(struct hcsr04_dev *)*Devices, GFP_KERNEL);	
	//allocating memory
	hcsr04_drvp = kmalloc(sizeof(struct hcsr04_drv), GFP_KERNEL);
	if(!hcsr04_drvp) {
		printk(KERN_INFO "Bad Kmalloc for Driver\n");
	}

	for (i=0;i<Devices;i++)
	{
		spin_lock_init(&hcsr04_devps[i]->l);  //spinlock initialization
		sprintf(x, "%d", i);
		sprintf(hcsr04_name,"HCSR04_%s",x);
		hcsr04_devps[i] = kmalloc(sizeof(struct hcsr04_dev), GFP_KERNEL);  //allocating memory to rows for each device defined by the user
		if(!hcsr04_devps[i])
		{
			printk(KERN_INFO "Bad Kmalloc for device\n");
			return -ENOMEM;
		}
		
		memset(hcsr04_devps[i], 0, sizeof(struct hcsr04_dev));
		
		hcsr04_devps[i]->miscdevice.minor=MISC_DYNAMIC_MINOR;  //dynamically assign the minor number
		//sprintf(hcsr04_name, "HCSR04_%s", x);
		hcsr04_devps[i]->miscdevice.name=hcsr04_name;
		
		hcsr04_devps[i]->miscdevice.fops = &hcsr04_fops;   //assign the respective file operations

		/*
		Register a miscellaneous device with the kernel. If the minor number is set to MISC_DYNAMIC_MINOR a minor number is assigned and placed in the minor field of the structure. For other cases the minor number requested is used.

		The structure passed is linked into the kernel and may not be destroyed until it has been unregistered. By default, an open syscall to the device sets file->private_data to point to the structure. Drivers don't need open in fops for this.

		A zero is returned on success and a negative errno code for failure.
		*/
	
		ret = misc_register(&hcsr04_devps[i]->miscdevice);
		if(ret) {
			printk("Can't register device\n");
			return ret;
		}
		
		//initialize the device structure variables
		hcsr04_devps[i]->head = 0;
		hcsr04_devps[i]->c = 0;
		hcsr04_devps[i]->counter = 0;
		hcsr04_devps[i]->hcsr04_configs.triggerpin = 0;
		hcsr04_devps[i]->hcsr04_configs.echopin = 0;
		


		printk("%s initialized\n",hcsr04_name);
	}
	
		//append each device in the list structure
		i--;
		hcsr04_devps[i]->next = NULL;

		while(i)
		{
			hcsr04_devps[i-1]->next = hcsr04_devps[i];
			i=i-1;
		}
		hcsr04_drvp->head = hcsr04_devps[i];
		return 0;
}

void __exit hcsr04_exit(void)
{
	int i;
	int ret;
	for(i = Devices-1; i>=0; i--)
	{

		hcsr04_devps[i]->next = NULL;
		ret = misc_deregister(&hcsr04_devps[i]->miscdevice); //deregister each device

		if(ret) {
			printk("Can't deregister device\n");
		}

		kfree(hcsr04_devps[i]);  //deallocate memory
	}

	hcsr04_drvp->head = NULL;
	kfree(hcsr04_devps);
	kfree(hcsr04_drvp);
	printk("Disconnected\n");
}

module_init (hcsr04_init);
module_exit (hcsr04_exit);

MODULE_LICENSE("GPL");
