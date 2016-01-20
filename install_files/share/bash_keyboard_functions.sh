bind_keyboard() {
	# Check if the device is already binded
	if [ -e "/sys/bus/hid/drivers/razerkbd/$1" ]; then
		echo "Device already binded"
		return 0
	fi

	# No point unbinding the device if its already unbinded
	if [ -e "/sys/bus/hid/drivers/hid-generic/$1" ]; then
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

unbind_keyboard() {
	# Check if the device is already binded
	if [ -e "/sys/bus/hid/drivers/hid-generic/$1" ]; then
		echo "Device is not binded to razerkbd"
		return 0
	fi

	# No point unbinding the device if its already unbinded
	if [ -e "/sys/bus/hid/drivers/razerkbd/$1" ]; then
		echo "Unbinding device ($1) from razerkbd"
		echo -n "$1" > /sys/bus/hid/drivers/razerkbd/unbind 2> /dev/null
		if [ $? -ne 0 ]; then
			echo "Failed to unbind device"
			return -1
		fi
	fi

	# Bind to razerkbd
	echo "Binding device ($1) to hid-generic"
	echo -n "$1" > /sys/bus/hid/drivers/hid-generic/bind 2> /dev/null
	if [ $? -ne 0 ]; then
		echo "Failed to bind device"
		return -1
	fi

	echo "Bind Successful"
	return 0
}

bind_all_chromas() {
	exit_number=1

	for device in `ls /sys/bus/hid/devices/ | grep -P ".*1532:0(11A|203|209|214).*"`
	do
		device_id=$(basename "${device}")

		usb_interface_num=$(udevadm info "/sys/bus/hid/devices/${device_id}" | grep "ID_USB_INTERFACE_NUM" | sed -n "s/.*_NUM=\([0-9]\+\)/\1/p")
		# When passed to a VM the interface number isnt in the udevadm response :/
		phyiscal_device=$(udevadm info "/sys/bus/hid/devices/${device_id}" | grep "HID_PHYS" | sed -n "s/.*HID_PHYS.*\/\(input[0-9]\+\)/\1/p")

		# If its interface 2 then thats the device we want
		if [ "${usb_interface_num}" = "02" ] || [ "${phyiscal_device}" = "input2" ]; then
			bind_keyboard "${device_id}"
			if [ $? -eq 0 ]; then
				exit_number=0
			fi
		fi
	done

	return ${exit_number}
}

unbind_all_chromas() {
	for device in `ls /sys/bus/hid/drivers/razerkbd/ | grep -P ".*1532:0(11A|203|209|214).*"`
	do
		device_id=$(basename "${device}")

		unbind_keyboard "${device_id}"
	done
}
