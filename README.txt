NAME: RAUNAK
ASU ID: 1217240245

#################################################################################
###################################### PART 1 ###################################

1. Run the make file to build the modules.

2. Connect the board to the PC by following the setting_up_galileo_gen2 pdf uploaded on the canvas.

3. Transfer the modules, hcsr04_tester3 and hscr04.ko to the board through the scp command.

e.g. sudo scp -r raunak@192.168.1.233:/home/raunak/eosi/gpio/assign2/hscr04.ko root@192.168.1.5:/home/hcsrf

4. Once the modules are transferred, do insmod without parameter to install the hscr04.ko module and create one hscr04_0 device.

e.g. insmod hscr04.ko

5. To create two devices, specify the parameter in Device field after the insmod.

e.g. insmod hscr04.ko Device=2

6. once the two devices are created run the hcsr04_tester3.

7. It will ask for the trigger pin for device 1, enter the trigger pin for device 1.
   Then, It will ask for echo pin for the device 1, enter the echo pin for the device 1. 
Note: Enter the bothe edge enabled pins as input as specified in the message printed above.
Similarly, enter the trigger and echo pins for the next device, HCSR04_1;

8. Observe the timestamp and distance values by placing obstacles in the path of the each sensor.
Note: Placing obstacles too close to the hscr04 sensor can give erroneous results. So, maintain a considerable distance to observe the changes.

9. After the observation, unload the module by doing rmmod.
e.g. rmmod hscr04.ko

10.run the make clean command to remove the executables on the pc.


#################################################################################
################################### OUTPUT ######################################
Enter Trigger Pin from range 0-19
Enter both edge interrupt enabled Echo Pin from 2-6 or 9 or 11 or 13-19
Enter Trigger and Echo pins for the Device 1

Enter Trigger pin for HCSR04_0:9
Enter Echo pin for HCSR04_0:4
Enter Trigger and Echo pins for the Device 2

Enter Trigger pin for HCSR04_1:10
Enter Echo pin for HCSR04_1:5
Measured Time Stamp = 72803991174 
Measured Distance = 290 cm 
Measured Time Stamp = 75202438738 
Measured Distance = 2245 cm 
Measured Time Stamp = 151589573354 
Measured Distance = 309 cm 
Measured Time Stamp = 151513751206 
Measured Distance = 293 cm 
Measured Time Stamp = 151781131430 
Measured Distance = 301 cm 
Measured Time Stamp = 151765168082 
Measured Distance = 293 cm 
Measured Time Stamp = 152547364964 
Measured Distance = 301 cm 
Measured Time Stamp = 152519423540 
Measured Distance = 293 cm 
Measured Time Stamp = 152738917248 
Measured Distance = 309 cm 
Measured Time Stamp = 153022257900 
Measured Distance = 293 cm 
Measured Time Stamp = 153505139604 
Measured Distance = 156 cm 
Measured Time Stamp = 153888250826 
Measured Distance = 17 cm 
Measured Time Stamp = 153776510922 
Measured Distance = 17 cm 
Measured Time Stamp = 154271365640 
Measured Distance = 17 cm 
Measured Time Stamp = 154279348006 
Measured Distance = 17 cm 
Measured Time Stamp = 154530796536 
Measured Distance = 17 cm 
Measured Time Stamp = 155229147534 
Measured Distance = 17 cm 
Measured Time Stamp = 155612259154 
Measured Distance = 225 cm 
Measured Time Stamp = 155536431964 
Measured Distance = 130 cm 
Measured Time Stamp = 156186928560 
Measured Distance = 293 cm 
Measured Time Stamp = 156378478774 
Measured Distance = 291 cm 
Measured Time Stamp = 156039272110 
Measured Distance = 130 cm 
Measured Time Stamp = 157144710492 
Measured Distance = 91 cm 
Measured Time Stamp = 157044944014 
Measured Distance = 240 cm 
Measured Time Stamp = 157296362160 
Measured Distance = 24 cm 
Measured Time Stamp = 157547779630 
Measured Distance = 25 cm 
Measured Time Stamp = 157527830570 
Measured Distance = 23 cm 
Measured Time Stamp = 157719374970 
Measured Distance = 290 cm 
Measured Time Stamp = 158102495352 
Measured Distance = 290 cm 
Measured Time Stamp = 158302024974 
Measured Distance = 293 cm 
Measured Time Stamp = 158677165986 
Measured Distance = 290 cm 
Measured Time Stamp = 158804871138 
Measured Distance = 293 cm 
Measured Time Stamp = 159443390978 
Measured Distance = 294 cm 
Measured Time Stamp = 159634945120 
Measured Distance = 309 cm 
Measured Time Stamp = 159826503156 
Measured Distance = 302 cm 
Measured Time Stamp = 160061958412 
Measured Distance = 293 cm 
Measured Time Stamp = 160313366782 
Measured Distance = 294 cm 
Measured Time Stamp = 160401172918 
Measured Distance = 301 cm 
Measured Time Stamp = 160592729914 
Measured Distance = 301 cm 
Measured Time Stamp = 161167407456 
Measured Distance = 301 cm 
Measured Time Stamp = 161067623258 
Measured Distance = 294 cm 
Measured Time Stamp = 161358953668 
Measured Distance = 309 cm 
Measured Time Stamp = 161933623084 
Measured Distance = 309 cm 
Measured Time Stamp = 161821883536 
Measured Distance = 294 cm 
Measured Time Stamp = 162324709872 
Measured Distance = 293 cm 
Measured Time Stamp = 163078973150 
Measured Distance = 293 cm 
Measured Time Stamp = 163581809274 
Measured Distance = 308 cm 
Measured Time Stamp = 163833227068 
Measured Distance = 308 cm 
Measured Time Stamp = 164336060072 
Measured Distance = 309 cm 
Measured Time Stamp = 165090319870 
Measured Distance = 308 cm 

#################################################################################
################################### PART 2 ######################################

1. Run the make file to build the modules.

Note: main.c is not required but is added for the testing convenience. so, hcsr04_tester3 will be an extra executable created in the process of make. 

2. Connect the board to the PC by following the setting_up_galileo_gen2 pdf uploaded on the canvas.

3. make the run.sh script executable through,

   chmod +x run.sh

4. Connect the trigger and echo pins for first hcsr04 device to 9 and 4 respectively.
   Connect the trigger and echo pins for second hcsr04 device to 10 and 5 respectively.

Note: Connect before switching on the galileo board for safe operation and to avoid malfunctioning of the board. 

5. Transfer the modules, hcsr04plat.ko, hscr04plat_drv.ko and run.sh to the board through the scp command.

e.g. sudo scp -r raunak@192.168.1.233:/home/raunak/eosi/gpio/assign2part2/hcsr04plat_drv.ko root@192.168.1.5:/home/hcsr2

6. Once the modules are transferred, do insmod to load the modules in the following order:
A. insmod hscr04plat_drv.ko
B. insmod hcsr04plat.ko Device=2

7. Check the device being created in the sysfs directory by the following command,
   ls /sys/class
   ls /sys/class/HCSR04
   ls /sys/class/HCSR04/HCSR04_0
   ls /sys/class/HCSR04/HCSR04_1

8. Now, run the run.sh script through the following command,
   ./run.sh

9. Number of samples and sample period for first hcsr04 device is configured as 3 and 50 respectively
   Number of samples and sample period for second hcsr04 device is configured as 4 and 70 respectively.

10. Observe the trigger store, show, echo store, show, sample store, show, sample_period store, show, enable store, show and distance in the output as well as the concurrent operation of both the devices.

11. After the observation unload the modules through rmmod,
    rmmod hscr04plat_drv.ko
    rmmod hcsr04plat.ko

12 run the make clean command to remove the executables on the pc.

#################################################################################
################################## Output #######################################
Trigger of HCSR04_0 : 
9
Echo of HCSR04_0 : 
4
Samples of HCSR04_0 : 
3
Sample Period of HCSR04_0 : 
50
Trigger of HCSR04_1 : 
10
Echo of HCSR04_1 : 
5
Samples of HCSR04_1 : 
4
Sample Period of HCSR04_1 : 
70
Enable of HCSR04_0 : 
1
Enable of HCSR04_0 : 
0
Distance measured by HCSR04_0 : 
294
Enable of HCSR04_1 : 
1
Enable of HCSR04_1 : 
0
Distance measured by HCSR04_1 : 
307
Concurrent Distance measured by HCSR04_0 : 
294
Concurrent Distance measured by HCSR04_1 : 
307

