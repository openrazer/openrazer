#!/bin/bash

RAZER_BLACKWIDOW_CHROMA_DEVICES=`ls /sys/bus/hid/devices/ | grep "1532:0C00"`
for DEV in $RAZER_BLACKWIDOW_CHROMA_DEVICES
do 
	if [ -d "/sys/bus/hid/devices/$DEV/input" ]; then
		INPUT_DEVS=`ls /sys/bus/hid/devices/$DEV/input`
		for INPUT_DEV in $INPUT_DEVS
		do
			MOUSE=`ls /sys/bus/hid/devices/$DEV/input/$INPUT_DEV/ | grep "mouse"`
			if [ $MOUSE ]; then
				#echo "Found Razer LED Device : $DEV"
				DEVPATH=/sys/bus/hid/devices/$DEV
			fi
		done
	else
		#no input directories ? use .0003 as default and try that
		if [[ "$DEV" == *.0003 ]]; then
			DEVPATH=/sys/bus/hid/devices/$DEV
		fi
	fi
done

echo -n "1" > $DEVPATH/reset
echo -n "1" > $DEVPATH/mode_none
echo -n "1" > $DEVPATH/mode_custom
while [ /bin/true ];
do
		#echo -n "1" > $DEVPATH/reset
		#echo -n "1" > $DEVPATH/mode_none
		echo -n -e "\x0\x255\x255\x255\x255\x255\x255\x255\x255\x0\x255\x0\x255\x255\x255\x255\x255\x255\x255\x255\x0\x255\x255\x255\x255\x255\x255\x255\x0\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255" > $DEVPATH/set_key_row
		#echo -n -e "\x0\x255\x255\x255" > $DEVPATH/set_key_row
		#echo -n -e "\x0\x0\x0" > $DEVPATH/set_key_row
		echo -n "1" > $DEVPATH/mode_custom
		#sleep 1
		#echo -n "1" > $DEVPATH/reset
		#echo -n "1" > $DEVPATH/mode_none
		#echo -n -e "\x0\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255\x255" > $DEVPATH/set_key_row
		#echo -n "1" > $DEVPATH/mode_custom
		#echo -n -e "\x0\x255\x255\x255\x255\x255\x255\x255\x255\x0\x255\x255\x255\x0\x0\x255\x255\x255\x255\x255\x255\x255\x0\x255\x0\x255\x255\x255\x255\x255\x0\x255\x255\x255\x0\x255\x255\x255\x0\x255\x255\x255\x255\x255\x255\x255" > $DEVPATH/set_key_row
		sleep 1
		#echo -n "1" > $DEVPATH/mode_custom
done


