#!/bin/bash

#HCSR04_0 configuration of trigger, echo, sample and sample period.

echo -n "9" > /sys/class/HCSR04/HCSR04_0/trigger
echo -n "4" > /sys/class/HCSR04/HCSR04_0/echo
echo -n "3" > /sys/class/HCSR04/HCSR04_0/sample
echo -n "50" > /sys/class/HCSR04/HCSR04_0/sample_period

echo "Trigger of HCSR04_0 : " | cat - /sys/class/HCSR04/HCSR04_0/trigger
echo "Echo of HCSR04_0 : " | cat - /sys/class/HCSR04/HCSR04_0/echo
echo "Samples of HCSR04_0 : " | cat - /sys/class/HCSR04/HCSR04_0/sample
echo "Sample Period of HCSR04_0 : " | cat - /sys/class/HCSR04/HCSR04_0/sample_period


#HCSR04_1 configuration of trigger, echo, sample and sample period.

echo -n "10" > /sys/class/HCSR04/HCSR04_1/trigger
echo -n "5" > /sys/class/HCSR04/HCSR04_1/echo
echo -n "4" > /sys/class/HCSR04/HCSR04_1/sample
echo -n "70" >/sys/class/HCSR04/HCSR04_1/sample_period

echo "Trigger of HCSR04_1 : " | cat - /sys/class/HCSR04/HCSR04_1/trigger
echo "Echo of HCSR04_1 : " | cat - /sys/class/HCSR04/HCSR04_1/echo
echo "Samples of HCSR04_1 : " | cat - /sys/class/HCSR04/HCSR04_1/sample
echo "Sample Period of HCSR04_1 : " | cat - /sys/class/HCSR04/HCSR04_1/sample_period


#starting measurement for HCSR04_0
echo -n "1" > /sys/class/HCSR04/HCSR04_0/enable

echo "Enable of HCSR04_0 : " | cat - /sys/class/HCSR04/HCSR04_0/enable

sleep 5

echo -n "0" >/sys/class/HCSR04/HCSR04_0/enable


echo "Enable of HCSR04_0 : " | cat - /sys/class/HCSR04/HCSR04_0/enable
#printing the distance
echo "Distance measured by HCSR04_0 : " | cat - /sys/class/HCSR04/HCSR04_0/distance

#starting measurement for HCSR04_1
echo -n "1" > /sys/class/HCSR04/HCSR04_1/enable

echo "Enable of HCSR04_1 : " | cat - /sys/class/HCSR04/HCSR04_1/enable

sleep 5

echo -n "0" >/sys/class/HCSR04/HCSR04_1/enable

echo "Enable of HCSR04_1 : " | cat - /sys/class/HCSR04/HCSR04_1/enable

#printing the distance
echo "Distance measured by HCSR04_1 : " | cat - /sys/class/HCSR04/HCSR04_1/distance

#Concurrent measurement
#starting measurement for HCSR04_0
echo -n "1" > /sys/class/HCSR04/HCSR04_0/enable

#starting measurement for HCSR04_1
echo -n "1" > /sys/class/HCSR04/HCSR04_1/enable

#printing the distance
echo "Concurrent Distance measured by HCSR04_0 : " | cat - /sys/class/HCSR04/HCSR04_0/distance

#printing the distance
echo "Concurrent Distance measured by HCSR04_1 : " | cat - /sys/class/HCSR04/HCSR04_1/distance

