# SPDX-License-Identifier: GPL-2.0-or-later

# Whether hid_report_raw_event() takes 6 parameters compared to the original 5 parameters.
# See "HID: pass the buffer size to hid_report_raw_event"
HAS_HID_BUFSIZE := $(shell grep -q 'size_t bufsize' $(srctree)/include/linux/hid.h && echo y || echo n)
ifeq ($(HAS_HID_BUFSIZE),y)
    ccflags-y += -DLINUX_HID_REPORT_RAW_EVENT_WITH_BUFFER_SIZE
endif

obj-m := razerkbd.o razermouse.o razerkraken.o razeraccessory.o

razerkbd-y := razerkbd_driver.o razercommon.o razerchromacommon.o
razermouse-y := razermouse_driver.o razercommon.o razerchromacommon.o
razerkraken-y := razerkraken_driver.o razercommon.o
razeraccessory-y := razeraccessory_driver.o razercommon.o razerchromacommon.o
