/*   A test program for /dev/hcsr04  
	*/


#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <sys/ioctl.h>
#include <poll.h>

#include "hcsr04_header.h"

ioctl_data p;
//fifo buffer structure
struct hcsr04_fifo_buf {

	unsigned long long int TSC;
	unsigned long long int size;
} *hcsr04_fifo_bufs;

void read_and_print(int fd) // this function reads data from kernel and prints it.
{
	
	hcsr04_fifo_bufs = (struct hcsr04_fifo_buf*)malloc(sizeof(struct hcsr04_fifo_buf));  //allocate memory for the fifo buffer structure
	int ret = read(fd, hcsr04_fifo_bufs);  //read the data from the fifo buffer in the driver to the fifo buffer structure defined here
	printf("Measured Time Stamp = %llu \n", hcsr04_fifo_bufs->TSC); //get the time stamp
	printf("Measured Distance = %llu cm \n", hcsr04_fifo_bufs->size);  //get the distance
}

void * _inputData( void *file)
{
	int *f;
	int i,j;
	f = (int *)file; //cast to integer type pointer

	for(i=0; i<50; i++)
	{
		for(j=0; j<rand()%4; j++)
		{
			write(*f, i%3); //write random non zero numbers to acquire the distance and time stamp values
		}
		if(i%2 == 0)
		{
			read_and_print(*f); //read the distance and time stamp values
		}
	}
}

int main(int argc, char **argv)
{

	printf("Enter Trigger Pin from range 0-19\n");
	printf("Enter both edge interrupt enabled Echo Pin from 2-6 or 9 or 11 or 13-19\n");
	int hcsr04_dev[5];
	char name[20];
	char name1[20];
	int ret,i;
	int thread_id1, thread_id2; //thread id for each device
	pthread_t pthd1, pthd2; //threads defined for each device

	for(i=0; i<2; i++)
	{
		sprintf(name1, "HCSR04_%d", i);
		printf("Enter Trigger and Echo pins for the Device %d\n\n", i+1);
		//scanf("%d %d", &p.val1, &p.val2);
		printf("Enter Trigger pin for %s:",name1);
		scanf("%d", &p.val1);  //take the trigger pin input from the user
		printf("Enter Echo pin for %s:",name1);
		scanf("%d", &p.val2);  //take the echo pin input from the user
		
		sprintf(name, "/dev/HCSR04_%d", i); //define name for the particular device

		hcsr04_dev[i] = open(name, O_RDWR);  //open the particular device
		if(hcsr04_dev[i] == -1)
		{
			printf("No file\n");
			exit(-1);
		}

		ret = ioctl(hcsr04_dev[i], CONFIG_PINS, &p);  //cinfigure the echo and trigger pins
		if(ret == -1)
			exit(-1);

		sleep(1);
	}

	p.val1 = 2;
	p.val2 = 65;
	ret = ioctl(hcsr04_dev[0], SET_PARAMETERS, &p); //set the number of samples and sample period values for device 1
	if(ret == -1)
		printf("setting failed\n");

	p.val1 = 3;
	p.val2 = 80;
	ret = ioctl(hcsr04_dev[1], SET_PARAMETERS, &p); //set the number of samples and sample period values
	if(ret == -1)
		printf("setting failed\n");

	sleep(1);

	thread_id1=pthread_create(&pthd1, NULL, _inputData, &hcsr04_dev[0]);  //create the thread for device 1
	thread_id2=pthread_create(&pthd2, NULL, _inputData, &hcsr04_dev[1]);  //create the thread for device 2

	pthread_join(pthd1, NULL);  //terminate the thread for device 1
	pthread_join(pthd2, NULL);  //terminate the thread for device 2

	sleep(1); //wait
	
	close(hcsr04_dev[0]);  //close the device 1
	close(hcsr04_dev[1]);  //close the device 2

	return 0;
}

	


