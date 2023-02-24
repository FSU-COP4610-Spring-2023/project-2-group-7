#!/bin/bash
sudo rmmod my_timer
sudo insmod my_timer.ko

cat /proc/timer

sleep 1
cat /proc/timer

sleep 2
cat /proc/timer

sleep 3
cat /proc/timer

sleep 4
cat /proc/timer

exit