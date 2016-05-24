#!/bin/sh

bind_device() {
	# Check if the device is already binded
	if [ -e "/sys/bus/hid/drivers/$1/$2" ]; then
		echo "Device already binded"
		return 0
	fi

	# No point unbinding the device if its already unbinded
	if [ -e "/sys/bus/hid/drivers/hid-generic/$2" ]; then
		echo "Unbinding device ($2) from hid-generic"
		echo -n "$2" > /sys/bus/hid/drivers/hid-generic/unbind 2> /dev/null
		if [ $? -ne 0 ]; then
			echo "Failed to unbind device"
			return -1
		fi
	fi

	# Bind to razerkbd
	echo "Binding device ($2) to $1"
	echo -n "$2" > /sys/bus/hid/drivers/$1/bind 2> /dev/null
	if [ $? -ne 0 ]; then
		echo "Failed to bind device"
		return -1
	fi

	echo "Bind Successful"
	return 0
}

unbind_device() {
	# Check if the device is already binded
	if [ -e "/sys/bus/hid/drivers/hid-generic/$2" ]; then
		echo "Device is not binded to razerkbd"
		return 0
	fi

	# No point unbinding the device if its already unbinded
	if [ -e "/sys/bus/hid/drivers/$1/$2" ]; then
		echo "Unbinding device ($2) from $1"
		echo -n "$2" > /sys/bus/hid/drivers/$1/unbind 2> /dev/null
		if [ $? -ne 0 ]; then
			echo "Failed to unbind device"
			return -1
		fi
	fi

	# Bind to hid
	echo "Binding device ($1) to hid-generic"
	echo -n "$2" > /sys/bus/hid/drivers/hid-generic/bind 2> /dev/null
	if [ $? -ne 0 ]; then
		echo "Failed to bind device"
		return -1
	fi

	echo "Bind Successful"
	return 0
}

# Create bind_all
# Create unbind_all

bind_all() {
	bind_all_device "razerkbd" "011A|011B|010D|0203|0205|0209|0214" "02" "input2"
	bind_all_device "razermouse" "0045|0046" "00" "input0"
	bind_all_device "razerfirefly" "0C00" "00" "input0"
}

unbind_all() {
	unbind_all_device "razerkbd" "011A|011B|010D|0203|0205|0209|0214"
	unbind_all_device "razermouse" "0045|0046"
	unbind_all_device "razerfirefly" "0C00"
}

bind_all_device() {
	exit_number=0

	for device in `ls /sys/bus/hid/devices/ | grep -P ".*1532:($2).*"`
	do
		device_id=$(basename "${device}")

		usb_interface_num=$(udevadm info "/sys/bus/hid/devices/${device_id}" | grep "ID_USB_INTERFACE_NUM" | sed -n "s/.*_NUM=\([0-9]\+\)/\1/p")
		# When passed to a VM the interface number isnt in the udevadm response :/
		phyiscal_device=$(udevadm info "/sys/bus/hid/devices/${device_id}" | grep "HID_PHYS" | sed -n "s/.*HID_PHYS.*\/\(input[0-9]\+\)/\1/p")

		# If its interface 2 then thats the device we want
		if [ "${usb_interface_num}" = "$3" ] || [ "${phyiscal_device}" = "$4" ]; then
			bind_device "$1" "${device_id}"
			if [ $? -ne 0 ]; then
				exit_number=1
			fi
		fi
	done

	return ${exit_number}
}


unbind_all_device() {
	for device in `ls /sys/bus/hid/drivers/$1/ | grep -P ".*1532:($2).*"`
	do
		device_id=$(basename "${device}")

		unbind_device $1 "${device_id}"
	done
}
