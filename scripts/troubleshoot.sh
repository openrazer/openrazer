#!/bin/bash

dkms_name="openrazer-driver"  # TODO Rename when dkms module is renamed.

echo "This script will now try to determine why the razer driver isn't working correctly."

# Step 1 - check if there is a razer device connected.
lsusb=$(lsusb | grep '1532:')
if [ -z "$lsusb" ]; then
  echo "ERROR: Linux doesn't see any Razer device connected. Make sure it is connected! Exiting."
  exit 1
fi

# Step 2 - check if a module is already loaded.
lsmod=$(lsmod | grep razer)
if [ -z "$lsmod" ]; then
  echo "INFO: There is no kernel module loaded."
fi

# Step 3 - check if the module can load correctly
echo "Please enter your 'sudo' password for privileged operations."
module_name="razerkbd"
modprobe=$(sudo modprobe $module_name 2>&1)
if [ $? != 0 ]; then
  if [[ $modprobe == *"FATAL: Module $module_name not found"* ]]; then
    echo "ERROR: The kernel module wasn't found. Trying to find out why."
    dkms_status=$(sudo dkms status $dkms_name)
    if [ -z "$dkms_status" ]; then
      echo "ERROR: DKMS module isn't installed. Trying to install now."
      dkms_version=$(cat /usr/src/$dkms_name-*/dkms.conf | grep PACKAGE_VERSION | cut -d '"' -f2)
      # TODO Check error code on command "cat /usr/..."
      dkms_install=$(sudo dkms install $dkms_name/$dkms_version)
      if [ $? != 0 ]; then
        if [[ $dkms_install == *"Error! Could not find module source directory."* ]]; then
          echo "FATAL: Is the razer dkms package installed? Exiting."
          exit 1
        elif [[ $dkms_install == *"Module openrazer-driver/$dkms_version already installed on kernel"* ]]; then
          echo "FATAL: The dkms module is installed, but could not be loaded. Exiting."
          exit 1
        fi
      else
        echo "INFO: dkms module should be installed now. Output of command:"
        echo $dkms_install
        echo "INFO: Please call the script again to check further. Exiting."
        exit 0
      fi
    fi
  elif [[ $modprobe == *"ERROR: could not insert '$module_name': Required key not available"* ]]; then
    echo "ERROR: The module couldn't be loaded due to secure boot being enabled. Please disable secure boot in your BIOS/UEFI and try again. Exiting."
    exit 1
  else
    echo "FATAL: Unknown error. The output will be echo'ed below. Exiting."
    echo $modprobe
    exit 1
  fi
fi

# Step 4 - check the daemon
log_dir=/tmp/razer_logs
mkdir -p $log_dir
openrazer-daemon -r -v --log-dir=$log_dir
sleep 3
daemon_output=$(cat $log_dir/razer.log)
if [ ! -z "$(echo $daemon_output | grep 'PermissionError: [Errno 13] Permission denied')" ]; then
  echo "ERROR: The current user doesn't have permission to access the driver files. Checking why."
  file=$(echo $daemon_output | cut -d "'" -f4)
  owning_group=$(stat -c '%G' $file)
  if [ $owning_group != "plugdev" ]; then
    echo "FATAL: Because of some reason, the owner of the file isn't plugdev, as it should be. Exiting."
    exit 1
  fi
  group_plugdev=$(groups | grep plugdev)
  if [ -z "$group_plugdev" ]; then
    echo "FATAL: The current isn't member of the group 'plugdev'. Please fix this by running 'sudo gpasswd -a $USER plugdev'. Exiting."
    exit 1
  fi
fi

# Step 5 - check if the pylib sees the device(s)
pylib_len=$(python -c "from openrazer.client import DeviceManager; a = DeviceManager(); print(len(a.devices))")
if [ "$pylib_len" == 0 ]; then
  echo "FATAL: The device was not recognized by the python library. Exiting."
  exit 1
fi

# Everything alright (probably)
echo "The script didn't find an error. If you still think something's wrong, please create an issue on the GitHub page. Exiting."
exit 0
