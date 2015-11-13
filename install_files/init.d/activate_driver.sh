#!/bin/bash
#udevadm info /sys/bus/hid/devices/0003:1532:0203.0011 | grep "ID_USB_INTERFACE_NUM"
modprobe razerkbd

bind_device() {
	# Check if the device is already bound
	if [ -a "/sys/bus/hid/drivers/razerkbd/$1" ]; then
		echo "Device already bound"
		return 0
	fi

	# No point unbinding the device if its already unbound
	if [ -a "/sys/bus/hid/drivers/hid-generic/$1" ]; then
		echo "Unbinding device ($1) from hid-generic"
		echo -n "$1" > /sys/bus/hid/drivers/hid-generic/unbind 2> /dev/null
		if [ $? -ne 0 ]; then
			echo "Failed to unbind device"
			return -1
		fi
	fi

	# Bind to razerkbd
	echo "Binding device ($1) to razerkbd"
	echo -n "$1" > /sys/bus/hid/drivers/razerkbd/bind 2> /dev/null
	if [ $? -ne 0 ]; then
		echo "Failed to bind device"
		return -1
	fi

	echo "Bind Successful"
	return 0
}



for device in /sys/bus/hid/devices/*:1532:020[39]*
do
	device_id=$(basename "${device}")
	usb_interface_num=$(udevadm info "/sys/bus/hid/devices/${device_id}" | grep "ID_USB_INTERFACE_NUM" | sed -n "s/.*_NUM=\([0-9]\+\)/\1/p")

	# If its interface 2 then thats the device we want
	if [ "${usb_interface_num}" == "02" ]; then
		bind_device "${device_id}"
	else
		# Worst case try the 3rd entry as that should be it
		if [[ "${device_id}" == *.0003 ]]; then
			bind_device "${device_id}"
		fi
	fi
done

