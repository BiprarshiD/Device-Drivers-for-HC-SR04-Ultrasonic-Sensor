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


static short int Device;

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

	unsigned long long int TSC;
	unsigned long long int size;
}hcsr04_fifo_bufs;

struct hcsr04_plt {

	char *name;
	struct platform_device hcsr04_plt_dev;
	int dev_minor;
	struct hcsr04_plt *next;
	int dev_num;
}hcsr04_device;


//Structure for the device defined
struct hcsr04_dev{
	
	struct miscdevice miscdevice;
	struct hcsr04_plt hcsr04_device;
	char name[20];
	struct hcsr04_dev *next;
	struct hcsr04_config hcsr04_configs;
	int c;
	int head;
	int counter;
	struct hcsr04_fifo_buf hcsr04_fifo_bufs[5];
	int m;
	int enable;
	int recent_distance;
	struct task_struct *t;
	struct list_head list;
	spinlock_t l;
	
} *hcsr04_devp;

//For appending and accessing multiple devices in the various functions
struct hcsr04_drv{

	struct hcsr04_dev *head;
	struct hcsr04_dev *curr;
}*hcsr04_drvp;


