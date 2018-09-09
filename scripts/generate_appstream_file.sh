#!/bin/sh -e

output_appstream_file="install_files/appstream/io.github.openrazer.openrazer.metainfo.xml"

# First part of AppStream file
echo '<?xml version="1.0" encoding="UTF-8"?>
<component>
  <id>io.github.openrazer.openrazer</id>
  <metadata_license>CC0-1.0</metadata_license>
  <project_license>GPL-2.0+</project_license>
  <name>OpenRazer</name>
  <summary>Drivers for Razer peripherals on GNU/Linux</summary>
  <description>
    <p>
      OpenRazer is a collection of GNU/Linux drivers for the Razer devices.
      Supported devices include keyboards, mice, mouse-mats, headsets and
      various other devices. OpenRazer provides kernel drivers, DBus services
      and python bindings to interact with the DBus interface.
    </p>
  </description>
  <url type="homepage">https://openrazer.github.io/</url>
  <provides>' > ${output_appstream_file}

# Autogeneration of supported devices list
grep -rh "USB_PID = " daemon/openrazer_daemon/hardware/ | cut -d '=' -f2 | grep -v None | sort -f | sed -E 's| 0x([0-9A-Fa-f]{4})|    <modalias>usb:v1532p\1d*</modalias>|g' >> ${output_appstream_file}

# First last of AppStream file
echo '  </provides>
</component>' >> ${output_appstream_file}
