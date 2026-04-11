// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2015 Terry Cain <terrys-home.co.uk>
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>
#include <linux/random.h>

#include "razernari_driver.h"
#include "razercommon.h"

/*
 * Version Information
 */
#define DRIVER_DESC "Razer Nari Headset Device Driver"

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE(DRIVER_LICENSE);

#define NARI_DEBUG_REPORTS 0

/**
 * Print report to syslog
 */
static void print_nari_report(unsigned char* report)
{
    pr_info("Razer Nari received GET_REPORT response:");
    for (int i = 0; i < RAZER_NARI_REPORT_LEN; i++) {
        if (i % 16 == 0) {
            pr_cont("\n");
            pr_info("\t");
        }
        pr_cont("%02x ", report[i]);
    }
    pr_cont("\n");
}

/**
 * @brief Request a report from device.
 * 
 * @param usb_dev 
 * @return int 
 */
static int razer_nari_send_request_report_msg(struct hid_device *hdev)
{
    int ret;
    unsigned char* report;
    report = kzalloc(RAZER_NARI_REPORT_LEN, GFP_KERNEL);

    ret = hid_hw_raw_request(hdev, 0xFF, report, RAZER_NARI_REPORT_LEN, HID_FEATURE_REPORT, HID_REQ_GET_REPORT);
    if (ret != RAZER_NARI_REPORT_LEN) {
        pr_err("Failed to send GET_REPORT request: %d\n", ret);
        return ret;
    }

    // Print the received response to the kernel logs
    /*pr_info("Received GET_REPORT response: ");
    for (int i = 0; i < ret; i++)
        pr_cont("%02X ", report[i]);
    pr_cont("\n");*/
    struct razer_nari_device *device = dev_get_drvdata(&hdev->dev);
    memcpy(&device->data[0], report, RAZER_NARI_REPORT_LEN);
    print_nari_report(&device->data[0]);

    return 0;
}

/**
 * @brief Send a report to device in order to change some setting.
 * 
 * @param usb_dev 
 * @param report 
 * @param skip 
 * @return int 
 */
static int razer_nari_send_control_msg(struct usb_device *usb_dev,struct razer_nari_request_report* report, unsigned char skip)
{
    uint request = HID_REQ_SET_REPORT; // 0x09
    uint request_type = USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_OUT; // 0x21
    uint value = 0x03ff; //fixed for almost all messages. Let's start that way.
    uint index = 5;
    uint size = 64;
    char *buf;
    int len;

    buf = kmemdup(report, size, GFP_KERNEL);
    if (buf == NULL)
        return -ENOMEM;

    // Send usb control message
    len = usb_control_msg(usb_dev, usb_sndctrlpipe(usb_dev, 0),
                          request,      // Request      U8
                          request_type, // RequestType  U8
                          value,        // Value        U16
                          index,        // Index        U16
                          buf,          // Data         void* data
                          size,         // Length       U16
                          USB_CTRL_SET_TIMEOUT); //     Int

    // Wait
    if(skip != 1) {
        msleep(report->length * 15);
    }

    kfree(buf);
    if(len!=size)
        printk(KERN_WARNING "razer driver: Device data transfer failed.\n");

    return ((len < 0) ? len : ((len != size) ? -EIO : 0));
}

/**
 * Get a request report
 */
static struct razer_nari_request_report get_nari_request_report(void)
{
    struct razer_nari_request_report report;
    memset(&report, 0, sizeof(struct razer_nari_request_report));

    report.length = 64;
    report.arguments[0] = 0xFF; //this is always fixed FF.
    report.arguments[1] = 0x0A; //this is mostly 0A. Some differ. Hm.
    report.arguments[2] = 0x00; //this seems to be 0 always, too.
    report.arguments[3] = 0xFF; //this is ff for most request, fd for some, like setting sleep time.
    report.arguments[4] = 0x04; //seems fixed, too.

    report.arguments[5] = 0x12;
    report.arguments[6] = 0xF1;

    return report;
}

/**
 * Get a color / brighntess request report
 */
static struct razer_nari_request_report get_nari_brightness_request_report(unsigned short brightness)
{
    struct razer_nari_request_report report = get_nari_request_report();

    //set some fixed header. Always this for brightness settings:
    report.arguments[7] = 0x03;
    report.arguments[8] = 0x71;

    //now set brightness:
    report.arguments[9] = brightness;

    return report;
}

/**
 * Get a request report for color settings
 */
static struct razer_nari_request_report get_nari_color_request_report(unsigned short red, unsigned short green, unsigned short blue)
{
    struct razer_nari_request_report report = get_nari_request_report();

    //set some fixed header. Always this for color settings:
    report.arguments[7] = 0x05;
    report.arguments[8] = 0x72;

    //now set color:
    report.arguments[9] = red;
    report.arguments[10] = green;
    report.arguments[11] = blue;

    return report;
}

/**
 * Read device file "version"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_version(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%s\n", DRIVER_VERSION);
}

/**
 * Read device file "device_type"
 *
 * Returns friendly string of device type
 */
static ssize_t razer_attr_read_device_type(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);

    char *device_type;

    switch (device->usb_pid) {
    case USB_DEVICE_ID_RAZER_NARI_ULTIMATE_WIRELESS:
    case USB_DEVICE_ID_RAZER_NARI_ULTIMATE_USB:
        device_type = "Razer Nari Ultimate\n";
        break;

    case USB_DEVICE_ID_RAZER_NARI_WIRELESS:
    case USB_DEVICE_ID_RAZER_NARI_USB:
        device_type = "Razer Nari\n";
        break;

    default:
        device_type = "Unknown Device\n";
    }

    return sprintf(buf, device_type);
}

/**
 * Write device file "test"
 *
 * Does nothing
 */
static ssize_t razer_attr_write_test(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    return count;
}

/**
 * Read device file "test"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_test(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "\n");
}

/**
 * @brief Set the brigthness on device
 * 
 * @param device 
 * @param brightness 
 */
static void set_brigthness(struct razer_nari_device *device, unsigned char brightness) {
    device->brigthness = brightness;
    struct razer_nari_request_report report = get_nari_brightness_request_report(device->brigthness);

    // Lock access to sending USB as adhering to the razer len*15ms delay
    mutex_lock(&device->lock);
    razer_nari_send_control_msg(device->usb_dev, &report, device->brigthness);
    mutex_unlock(&device->lock);
}

/**
 * Write device file "brightness"
 *
 * Set brightness from this file
 */
static ssize_t razer_attr_write_matrix_brightness(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);
    unsigned char brightness = (unsigned char)simple_strtoul(buf, NULL, 10);

    set_brigthness(device, brightness);
    
    return count;
}

/**
 * Write device file "mode_none"
 *
 * None effect mode is activated whenever the file is written to
 */
static ssize_t razer_attr_write_matrix_effect_none(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);
    set_brigthness(device, 0); //none always means 0 brightness.

    return count;
}

/**
 * Write device file "mode_static"
 *
 * Static effect mode is activated whenever the file is written to with 3 bytes
 */
static ssize_t razer_attr_write_matrix_effect_static(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);

    if (count != 3 && count != 4) {
        printk(KERN_WARNING "razernari: Static mode only accepts RGB (3byte) or RGB with intensity (4byte)\n");
        return -EINVAL;
    }

    struct razer_nari_request_report rgb_report = get_nari_color_request_report(buf[0], buf[1], buf[2]);

    //I don't think this will work.. but should not hurt, right?
    if(count == 4) {
        rgb_report.arguments[12] = buf[3];
    }

    // Lock sending of the 2 commands
    mutex_lock(&device->lock);
    razer_nari_send_control_msg(device->usb_dev, &rgb_report, 0);
    mutex_unlock(&device->lock);

    return count;
}

/**
 * Read device file "mode_static"
 *
 * Returns 4 bytes for config
 */
static ssize_t razer_attr_read_matrix_effect_static(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);
    //device does not report color in reports, it seems... :-/ -> tested, not even synapse can do this. So it seems there is nothing we can do.
    buf[0] = device->red;
    buf[1] = device->green;
    buf[2] = device->blue;
    return 3;
}

static ssize_t razer_attr_read_matrix_brightness(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);
    return sprintf(buf, "%d\n", device->brigthness);
}

/**
 * Read device file "serial"
 *
 * Returns a string
 */
static ssize_t razer_attr_read_device_serial(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);
    return sprintf(buf, "%s\n", device->name); //we can not jet read the serial from reports... so just return the name for now.
}


#if NARI_DEBUG_REPORTS
/**
 * @brief Write device file "request_report"
 * 
 * Requests updated status from device. Will update charging status and battery level. //TODO!!
 */
static ssize_t razer_attr_write_request_report(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);
    mutex_lock(&device->lock);
    razer_nari_send_request_report_msg(device->hid_dev);
    mutex_unlock(&device->lock);

    return count;
};


/**
 * Write device file "request_battery_report"
 *
 * Requests updated status from device. Will update charging status and battery level. //TODO!!
 */
static ssize_t razer_attr_write_request_battery_report(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct razer_nari_device *device = dev_get_drvdata(dev);
    mutex_lock(&device->lock);
    struct razer_nari_request_report report = get_nari_request_report();
    report.arguments[3] = 0xFD; //this is the battery report request.
    report.arguments[4] = 0x04; //second try. Without that, it just repeated the request.
    report.arguments[7] = 0x02;
    report.arguments[8] = 0x05;
    razer_nari_send_control_msg(device->usb_dev, &report, 0);

    razer_nari_send_request_report_msg(device->hid_dev);
    mutex_unlock(&device->lock);

    return count;
};
#endif

/**
 * Set up the device driver files

 *
 * Read only is 0444
 * Write only is 0220
 * Read and write is 0664
 */

static DEVICE_ATTR(test,                    0660, razer_attr_read_test,                       razer_attr_write_test);
static DEVICE_ATTR(version,                 0440, razer_attr_read_version,                    NULL);
static DEVICE_ATTR(device_type,             0440, razer_attr_read_device_type,                NULL);
static DEVICE_ATTR(device_serial,           0440, razer_attr_read_device_serial,              NULL);
#if NARI_DEBUG_REPORTS
//static DEVICE_ATTR(firmware_version,        0440, razer_attr_read_firmware_version,           NULL);
static DEVICE_ATTR(request_report,          0220, NULL,                                       razer_attr_write_request_report);
static DEVICE_ATTR(request_battery_report,  0220, NULL,                                       razer_attr_write_request_battery_report);
#endif

static DEVICE_ATTR(matrix_brightness,       0660, razer_attr_read_matrix_brightness,          razer_attr_write_matrix_brightness);
static DEVICE_ATTR(matrix_effect_none,      0220, NULL,                                       razer_attr_write_matrix_effect_none);
static DEVICE_ATTR(matrix_effect_static,    0660, razer_attr_read_matrix_effect_static,       razer_attr_write_matrix_effect_static);

static void razer_nari_init(struct razer_nari_device *dev, struct usb_interface *intf, struct hid_device *hdev)
{
    struct usb_device *usb_dev = interface_to_usbdev(intf);

    // Initialise mutex
    mutex_init(&dev->lock);
    // Setup values
    dev->usb_dev = usb_dev;
    dev->usb_interface_protocol = intf->cur_altsetting->desc.bInterfaceProtocol;
    dev->usb_vid = usb_dev->descriptor.idVendor;
    dev->usb_pid = usb_dev->descriptor.idProduct;
    dev->hid_dev = hdev;

    switch(dev->usb_pid) { //custom setup. Not needed until now.
    case USB_DEVICE_ID_RAZER_NARI_ULTIMATE_WIRELESS:
    case USB_DEVICE_ID_RAZER_NARI_ULTIMATE_USB:
        dev->name = "Razer Nari Ultimate";
        break;
    case USB_DEVICE_ID_RAZER_NARI_USB:
    case USB_DEVICE_ID_RAZER_NARI_WIRELESS:
        dev->name = "Razer Nari";
        break;
    default:
        dev->name = "Unknown Device";
        break;
    }
}

/**
 * Probe method is ran whenever a device is binded to the driver
 */
static int razer_nari_probe(struct hid_device *hdev, const struct hid_device_id *id)
{
    int retval = 0;
    struct usb_interface *intf = to_usb_interface(hdev->dev.parent);
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_nari_device *dev = NULL;

    dev = kzalloc(sizeof(struct razer_nari_device), GFP_KERNEL);
    if(dev == NULL) {
        dev_err(&intf->dev, "out of memory\n");
        return -ENOMEM;
    }

    // Init data
    razer_nari_init(dev, intf, hdev);

    if(dev->usb_interface_protocol == USB_INTERFACE_PROTOCOL_NONE) {
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_version);                               // Get driver version
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_test);                                  // Test mode
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_type);                           // Get string of device type
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_device_serial);                         // Get serial of device
#if NARI_DEBUG_REPORTS
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_request_report);                        // Request report from device
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_request_battery_report);                // Request battery report from device
#endif
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_brightness);                     // Set brightness of logo led
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_none);                    // No effect
        CREATE_DEVICE_FILE(&hdev->dev, &dev_attr_matrix_effect_static);                  // Static effect
    }

    dev_set_drvdata(&hdev->dev, dev);

    if(hid_parse(hdev)) {
        hid_err(hdev, "parse failed\n");
        goto exit_free;
    }

    if (hid_hw_start(hdev, HID_CONNECT_DEFAULT)) {
        hid_err(hdev, "hw start failed\n");
        goto exit_free;
    }

    razer_nari_send_request_report_msg(hdev);

    usb_disable_autosuspend(usb_dev);

    return 0;

exit_free:
    kfree(dev);
    return retval;
}

/**
 * Unbind function
 */
static void razer_nari_disconnect(struct hid_device *hdev)
{
    struct razer_nari_device *dev;
    struct usb_interface *intf = to_usb_interface(hdev->dev.parent);

    dev = hid_get_drvdata(hdev);

    if(dev->usb_interface_protocol == USB_INTERFACE_PROTOCOL_NONE) {
        device_remove_file(&hdev->dev, &dev_attr_version);                               // Get driver version
        device_remove_file(&hdev->dev, &dev_attr_test);                                  // Test mode
        device_remove_file(&hdev->dev, &dev_attr_device_type);                           // Get string of device type
        device_remove_file(&hdev->dev, &dev_attr_device_serial);                         // Get serial of device
#if NARI_DEBUG_REPORTS
        device_remove_file(&hdev->dev, &dev_attr_request_report);                        // Request report from device
        device_remove_file(&hdev->dev, &dev_attr_request_battery_report);                // Request battery report from device
#endif
        device_remove_file(&hdev->dev, &dev_attr_matrix_brightness);                     // Set brightness of logo led
        device_remove_file(&hdev->dev, &dev_attr_matrix_effect_none);                    // No effect
        device_remove_file(&hdev->dev, &dev_attr_matrix_effect_static);                  // Static effect
    }

    hid_hw_stop(hdev);
    kfree(dev);
    dev_info(&intf->dev, "Razer Device disconnected\n");
}

/**
 * Device ID mapping table
 */
static const struct hid_device_id razer_devices[] = {
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_NARI_ULTIMATE_WIRELESS) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_NARI_ULTIMATE_USB) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_NARI_WIRELESS) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_NARI_USB) },
    { 0 }
};

MODULE_DEVICE_TABLE(hid, razer_devices);

/**
 * Describes the contents of the driver
 */
static struct hid_driver razer_nari_driver = {
    .name = "razernari",
    .id_table = razer_devices,
    .probe = razer_nari_probe,
    .remove = razer_nari_disconnect
};

module_hid_driver(razer_nari_driver);
