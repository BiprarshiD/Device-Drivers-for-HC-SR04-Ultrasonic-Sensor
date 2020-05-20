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
#include <linux/ioctl.h>
#include <linux/version.h>

#include "hcsr04_header.h"
#include "hcsr04.h"

#define CLASS_NAME "HCSR04"
#define DRIVER_NAME "hcsr04_platform_driver"

static struct device *hcsr04_devices;
static struct class *hcsr04_class;
static dev_t hcsr04_d;


//global variables declared
//ktime_t rising, falling;
int speed = 340;
long long unsigned distance = 0;
long period;
unsigned int irql;
unsigned long f;
int Devices;
int flag_trig=0;
int flag_delay=1;
static int init=1;
static LIST_HEAD(list);
uint64_t rising, falling;

//static DEFINE_MUTEX(hcsr04_mutex);

//For taking number of devices input from the user
static short int Device = 1;
module_param(Device, short, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);


//file operation functions declared
static int hcsr04_open(struct inode *, struct file *);
static int hcsr04_release(struct inode *, struct file *);
static int hcsr04_write(struct file *, const char *, size_t, loff_t *);
static ssize_t hcsr04_read(struct file *, char *, size_t, loff_t *);
static long hcsr04_ioctl(struct file *, unsigned int , unsigned long);



static const struct platform_device_id hcsr04_id_table[] = {
         { "HCSR04_0", 0 },
         { "HCSR04_1", 0 },
         { "HCSR04_2", 0 },
	 { "HCSR04_3", 0 },
         { "HCSR04_4", 0 },
         { "HCSR04_5", 0 },	
	 { },
};

//Time stamp counter defined
static __inline__ unsigned long long RDTSC(void)
{
  unsigned hi, lo;
  __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
  return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

//Configuration setting for the pins
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

//pin multiplexing
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
	struct hcsr04_dev *hcsr04_devp;  
	const struct file_operations *hcsr04_fops = NULL;
	
	
	//Assign respective file operations for different devices
	list_for_each_entry(hcsr04_devp, &list, list)
	{
		if(hcsr04_devp->miscdevice.minor == minor)
		{
			hcsr04_fops = fops_get(hcsr04_devp->miscdevice.fops);
			break;
		}
		
	}
	//Store the current used device in the structure for easy access in the functions defined below
	

	file->private_data = hcsr04_devp;
	printk("HCSR04: Device is opening\n");

	return 0;

}

//close the device driver
int hcsr04_release(struct inode *inode, struct file *file)
{
	struct hcsr04_dev *hcsr04_devp = file->private_data;
	//Free the echo and trigger pins
	config_pin(-1, hcsr04_devp->hcsr04_configs.echopin, 1);
	config_pin(-1, hcsr04_devp->hcsr04_configs.triggerpin, 1);
	//Free irq
	free_irq(gpio_to_irq(hcsr04_devp->hcsr04_configs.echo), NULL);

	printk("HCSR04: Device is closing\n");
	
		

	return 0;
}

//Function to write in the buffer
void hcsr04_buf_write(unsigned long long int tsc, unsigned long long int size, struct hcsr04_dev *hcsr04_devp)
{
	
	int a;
	a = hcsr04_devp->c;  //access counter for the buffer
	a=a+1;
	//storing only latest 5 values
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
	hcsr04_devp->hcsr04_fifo_bufs[hcsr04_devp->c].size=size;
	hcsr04_devp->hcsr04_fifo_bufs[hcsr04_devp->c].TSC=tsc;
	hcsr04_devp->c=a;
}

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
	
	a = hcsr04_devp->head + 1;
	if(a >= 5)
	{
		a=0;
	}
	//printk("TSC inside read buffer:%llu\n",hcsr04_devp->hcsr04_fifo_bufs[hcsr04_devp->head].TSC);
	//Retrieve the distance and Time Stamp from the buffer
	buf->size=hcsr04_devp->hcsr04_fifo_bufs[hcsr04_devp->head].size;
	buf->TSC=hcsr04_devp->hcsr04_fifo_bufs[hcsr04_devp->head].TSC;
	hcsr04_devp->head = a;
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
	}
	
	return (irq_handler_t) IRQ_HANDLED;
}

//Calculate the distance
void distance_calc(struct hcsr04_dev *hcsr04_devp)
{
	unsigned long long sum=0;
	unsigned long long first=0;
	unsigned long long last=80000000;
	int i;
	hcsr04_devp->m=1;
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
		sum = sum + distance; 
		//Find the first and the last values
		if(distance > first)
			first = distance;
		if(distance < last)
			last = distance;

		msleep(hcsr04_devp->hcsr04_configs.sample_period);
		
	}

	sum = sum - first -last;  //exclude the first and the last values
	do_div(sum, hcsr04_devp->hcsr04_configs.sample);  //Find the average distance
	hcsr04_buf_write(RDTSC(), sum, hcsr04_devp); //write the Time Stamp and the distance to the Fifo buffer 
	hcsr04_devp->m=0;
	//printk("HCSR04: RDTSC:%llu\n",RDTSC());
	//printk("HCSR04: Distance Found\n");
	hcsr04_devp->recent_distance=sum;
	spin_unlock_irqrestore(&hcsr04_devp->l,f);
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

//Write File operation
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
			hcsr04_devp->head=0;
			hcsr04_devp->c=0;
		}
		distance_calc(hcsr04_devp);
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
	//data= (struct ioctl_data *)val;
	data1 = (struct data* )val;
	switch(ioctl_num)
	{
		case SET_PARAMETERS:  //setting the parameters
		{
			if(data1->val2<=0 || data1->val1<=1) //return einval if the number of samples is less than or equal to 1 and the sample period is less than or equal to 0
			{
				return -EINVAL;
			}
			hcsr04_devp->hcsr04_configs.sample = data1->val1;
			hcsr04_devp->hcsr04_configs.sample_period = data1->val2;
		}
		break;
		
		case CONFIG_PINS:   //Pin Multiplexing
		{
			if((data1->val1 >= 0 && data1->val1 <=19) && ((data1->val2 >= 2 && data1->val2 <=6) || (data1->val2 >= 13 && data1->val2 <=19) || data1->val2 == 9 || data1->val2 == 11))
			{
				hcsr04_devp->hcsr04_configs.echo = config_pin(1, data1->val2, 0);
				hcsr04_devp->hcsr04_configs.echopin = data1->val2;
				hcsr04_devp->hcsr04_configs.trigger = config_pin(0, data1->val1, 0);
				hcsr04_devp->hcsr04_configs.triggerpin = data1->val1;
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
		
//file operations structure defined
static struct file_operations hcsr04_fops = {
	.owner	 	 = THIS_MODULE,
	.open	 	 = hcsr04_open,
	.release 	 = hcsr04_release,
	.write		 = hcsr04_write,
	.read		 = hcsr04_read,
	.unlocked_ioctl  = hcsr04_ioctl
	
	
};

//show the trigger pin
static ssize_t trigger_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct hcsr04_dev *hcsr04_devp = dev_get_drvdata(dev);
	return snprintf(buf, PAGE_SIZE, "%d\n", hcsr04_devp->hcsr04_configs.triggerpin);
}
//show the echo pin
static ssize_t echo_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct hcsr04_dev *hcsr04_devp = dev_get_drvdata(dev);
	return snprintf(buf, PAGE_SIZE, "%d\n",hcsr04_devp->hcsr04_configs.echopin);
}
//show the number of samples
static ssize_t sample_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct hcsr04_dev *hcsr04_devp = dev_get_drvdata(dev);
	return snprintf(buf, PAGE_SIZE, "%d\n",hcsr04_devp->hcsr04_configs.sample);
}
//show the sample period
static ssize_t sample_period_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct hcsr04_dev *hcsr04_devp = dev_get_drvdata(dev);
	return snprintf(buf, PAGE_SIZE, "%d\n",hcsr04_devp->hcsr04_configs.sample_period);
}
//show the enable value
static ssize_t enable_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct hcsr04_dev *hcsr04_devp = dev_get_drvdata(dev);
	return snprintf(buf, PAGE_SIZE, "%d\n",hcsr04_devp->enable);
}

//show the recent measured distance
static ssize_t distance_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct hcsr04_dev *hcsr04_devp = dev_get_drvdata(dev);
	return snprintf(buf, PAGE_SIZE, "%d\n",hcsr04_devp->recent_distance);
}

//set the trigger pin
static ssize_t trigger_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	struct hcsr04_dev *hcsr04_devp = dev_get_drvdata(dev);
	int trig;
	sscanf(buf, "%d", &trig);
	hcsr04_devp->hcsr04_configs.triggerpin = trig;
	hcsr04_devp->hcsr04_configs.trigger = config_pin(0, trig, 0);
	return size;
}

//set the echo pin
static ssize_t echo_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t size)
{

	struct hcsr04_dev *hcsr04_devp = dev_get_drvdata(dev);
	int echo, output, ret;
	sscanf(buf, "%d", &echo);
	hcsr04_devp->hcsr04_configs.echopin = echo;
	hcsr04_devp->hcsr04_configs.echo = config_pin(1, echo, 0);	

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
	return size;
}

//set the number of samples
static ssize_t sample_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	struct hcsr04_dev *hcsr04_devp = dev_get_drvdata(dev);
	int sample;
	sscanf(buf, "%d", &sample);
	hcsr04_devp->hcsr04_configs.sample = sample;
	return size;
}

//set the sample period
static ssize_t sample_period_store(struct device *dev, struct device_attribute *attr,const char *buf, size_t size)
{
	struct hcsr04_dev *hcsr04_devp = dev_get_drvdata(dev);
	int sample_period;
	sscanf(buf, "%d", &sample_period);
	hcsr04_devp->hcsr04_configs.sample_period = sample_period;
	return size;
}

//set the enable value and calculate the distance
static ssize_t enable_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t size)
{
	struct hcsr04_dev *hcsr04_devp = dev_get_drvdata(dev);
	int enable;
	sscanf(buf, "%d", &enable);
	hcsr04_devp->enable = enable;
	if(enable == 1)
	{
		
			distance_calc(hcsr04_devp);
		
	}
	else if(enable == 0)
	{
			return size;
			
	}
	else
	{
		return -EINVAL;
	}
	return size;
}

static DEVICE_ATTR(trigger, S_IRWXU, trigger_show, trigger_store);
static DEVICE_ATTR(echo, S_IRWXU, echo_show, echo_store);
static DEVICE_ATTR(sample, S_IRWXU, sample_show, sample_store);
static DEVICE_ATTR(sample_period, S_IRWXU, sample_period_show, sample_period_store);
static DEVICE_ATTR(enable, S_IRWXU, enable_show, enable_store);
static DEVICE_ATTR(distance, S_IRUSR , distance_show, NULL);

//probe function defined
static int hcsr04_probe(struct platform_device *dev)
{
	int ret;	
	const char *name;
	struct hcsr04_plt *plat_dev;
	struct hcsr04_dev *hcsr04_devp;
	struct hcsr04_fifo_buf *fifo;

	//accesss the platform device structure
	plat_dev = container_of(dev, struct hcsr04_plt, hcsr04_plt_dev);
	hcsr04_devp = container_of(plat_dev, struct hcsr04_dev, hcsr04_device);

	name = kmalloc(15, GFP_KERNEL);	

	if(init==1)
	{
		hcsr04_class = class_create(THIS_MODULE, CLASS_NAME);  //create the hcsr04 class
		
	}
	init=0;

	hcsr04_devp->hcsr04_device = *plat_dev;  //set the platform device

	hcsr04_devp->miscdevice.minor = MISC_DYNAMIC_MINOR;  //allocate the minor number dynamically
	name = hcsr04_devp->hcsr04_device.hcsr04_plt_dev.name; //get the name of platform device
	hcsr04_devp->miscdevice.name = name;  //get the name of misc device
	hcsr04_devp->miscdevice.fops = &hcsr04_fops;   //link the file operations
	ret = misc_register(&hcsr04_devp->miscdevice);  //register the misc device

	if(ret) {
			printk("Can't register device\n");
			return ret;
		}
		
	memset(hcsr04_devp->hcsr04_fifo_bufs, 0, 5*sizeof(fifo));  //initialize the fifo buffer
	hcsr04_devp->head = 0;
	hcsr04_devp->c = 0;
	hcsr04_devp->counter = 0;


	INIT_LIST_HEAD(&hcsr04_devp->list);  //add the head to the list
	list_add(&hcsr04_devp->list, &list);  //add further devices to the list

	hcsr04_d = 1;

	hcsr04_devices = device_create(hcsr04_class, NULL, hcsr04_d, hcsr04_devp, name);  //create the device


	device_create_file(hcsr04_devices, &dev_attr_trigger);
	device_create_file(hcsr04_devices, &dev_attr_echo);
	device_create_file(hcsr04_devices, &dev_attr_sample);
	device_create_file(hcsr04_devices, &dev_attr_sample_period);
	device_create_file(hcsr04_devices, &dev_attr_enable);
	device_create_file(hcsr04_devices, &dev_attr_distance);

	hcsr04_devp->hcsr04_device.dev_minor = hcsr04_devp->miscdevice.minor;
	printk("Initialized");
	return 0;
}

//remove the device
static int hcsr04_remove(struct platform_device *dev)
{
	struct hcsr04_dev *hcsr04_devp;
	struct hcsr04_dev *hcsr04_devc;
	struct hcsr04_plt *plat_dev;
	//access the platform device structure
	plat_dev = container_of(dev, struct hcsr04_plt, hcsr04_plt_dev);
	hcsr04_devp = container_of(plat_dev, struct hcsr04_dev, hcsr04_device);
	//loop for each entry in the list
	list_for_each_entry(hcsr04_devc, &list, list)
	{
		if(hcsr04_devp->hcsr04_device.dev_num == plat_dev->dev_num)
		{
		
			list_del(&hcsr04_devp->list); //remove the devices from the list
			//free the pins
			config_pin(-1, hcsr04_devp->hcsr04_configs.triggerpin, 1);
			config_pin(-1, hcsr04_devp->hcsr04_configs.echopin, 1);
			free_irq(gpio_to_irq(hcsr04_devp->hcsr04_configs.echo), 0);  //free the irq
			kfree(hcsr04_devp);
			misc_deregister(&hcsr04_devp->miscdevice);  //unregister the misc device
			device_destroy(hcsr04_class, hcsr04_devp->miscdevice.minor);  //destroy the device structure
			
			  
		}
	}
	//loop while the list is empty
	if(list_empty(&list))
	{
		class_destroy(hcsr04_class);  //destroy the hcsr04 class		
		class_unregister(hcsr04_class);  //unregister the hcsr04 class
		
	}
	printk("removed");
	return 0;
}

//define the platform device file operations
static struct platform_driver plat_fops = {

	.driver		= {
	.name		=	DRIVER_NAME,
	.owner		=	THIS_MODULE,
	},
	.probe		=	hcsr04_probe,
	.remove		=	hcsr04_remove,
	.id_table	=	hcsr04_id_table,
};


module_platform_driver(plat_fops);

MODULE_LICENSE("GPL");
