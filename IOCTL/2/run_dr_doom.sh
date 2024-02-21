#!/bin/bash

## Building the applications
(cd Application; make)

## Build your ioctl driver and load it here
(cd Device; make)

device_name=doom
mode="444"
echo "Loading module $device_name"

/sbin/insmod "./Device/$device_name.ko" || exit 1

# retrieve major number
major=$(cat /proc/devices | grep "$device_name")
major_number=($major)
echo "Removing stale nodes"
rm -f /dev/${device_name}
echo "Replacing the old nodes"
mknod /dev/${device_name} c ${major_number[0]} 0
major=$(cat /proc/devices | grep "$device_name")


###############################################

# Launching the control station
./Application/control_station &
c_pid=$!
echo "Control station PID: $c_pid"

# sleep 1 # ensure control station runs first.

# Launching the soldier
./Application/soldier $c_pid &
echo "Soldier PID: $!"

sleep 2
kill -9 $c_pid

## Remove the driver here



