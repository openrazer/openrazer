#!/bin/bash

SPEED=`printf '%x' $1`
R=`printf '%x' $2`
G=`printf '%x' $3`
B=`printf '%x' $4`

RAZER_BLACKWIDOW_CHROMA_DEVICES=`ls /sys/bus/hid/devices/ | grep "1532:0203"`
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
				echo -n -e "\x$SPEED\x$R\x$G\x$B" > $DEVPATH/mode_reactive
			fi
		done
	else
		#no input directories ? use .0003 as default and try that
		if [[ "$DEV" == *.0003 ]]; then
			DEVPATH=/sys/bus/hid/devices/$DEV
			echo -n -e "\x$SPEED\x$R\x$G\x$B" > $DEVPATH/mode_reactive
		fi
	fi
done

