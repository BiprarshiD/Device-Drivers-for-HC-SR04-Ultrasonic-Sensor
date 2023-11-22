#include <linux/ioctl.h>


//IOCTL Data structure
typedef struct data
{
	int val1;
	int val2;
}ioctl_data;

#define IOC 'k'
#define CONFIG_PINS _IOW(IOC, 0, ioctl_data)
#define SET_PARAMETERS _IOW(IOC, 1, ioctl_data)
