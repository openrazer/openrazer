/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Should you need to contact me, the author, you can do so either by
 * e-mail - mail your message to <pez2001@voyagerproject.de>, or by paper mail:
 * Tim Theede, Am See 22, 24790 Schuelldorf, Germany
 */


#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>

#include "razerkbd.h"

/*
 * Version Information
 */
#define DRIVER_VERSION "0.2"
#define DRIVER_AUTHOR "Tim Theede <pez2001@voyagerproject.de>"
#define DRIVER_DESC "USB HID Razer BlackWidow Chroma"
#define DRIVER_LICENSE "GPL v2"

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE(DRIVER_LICENSE);

/*

    TODO:

        restore store rgb profile (helpful for event-animations etc)
        #coloritup update

    future todos:

        read keystroke stats etc.

*/

int razer_send_report(struct usb_device *usb_dev,void const *data)
{
	uint report_id = 0x300;
	uint value = HID_REQ_SET_REPORT;
	uint index = 0x02;
	uint size = RAZER_BLACKWIDOW_REPORT_LEN;
	char *buf;
	int len;

	buf = kmemdup(data, size, GFP_KERNEL);
	if (buf == NULL)
		return -ENOMEM;

	len = usb_control_msg(usb_dev, usb_sndctrlpipe(usb_dev, 0),
			value,
			USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_OUT,
			report_id,
            index, buf, size, USB_CTRL_SET_TIMEOUT);
    usleep_range(RAZER_BLACKWIDOW_CHROMA_WAIT_MIN_US,RAZER_BLACKWIDOW_CHROMA_WAIT_MAX_US);
    kfree(buf);
	return ((len < 0) ? len : ((len != size) ? -EIO : 0));
}

unsigned char razer_calculate_crc(struct razer_report *report) 
{
    /*second to last byte of report is a simple checksum*/
    /*just xor all bytes up with overflow and you are done*/
    unsigned char crc = 0;
    int i = 0;
    unsigned char *_report = (unsigned char*)report;
    for(i=2;i<0x58;i++)
    {
    	crc ^= _report[i];
    }
    return(crc);
}

void razer_prepare_report(struct razer_report *report)
{
   /*fill static fields of report*/
   memset(report,0,sizeof(struct razer_report));
   report->id = 0xFF;
   report->reserved2 = 0x03;
}

int razer_set_wave_mode(struct usb_device *usb_dev,unsigned char direction)
{
    int retval;
    struct razer_report report;
    razer_prepare_report(&report);
    report.parameter_bytes_num = 2;
    report.command = RAZER_BLACKWIDOW_CHROMA_CHANGE_EFFECT; /*change effect command id*/
    report.sub_command = RAZER_BLACKWIDOW_CHROMA_EFFECT_WAVE;/*wave mode id*/
    report.command_parameters[0] = direction;/*direction 2=left / 1=right*/
    report.crc = razer_calculate_crc(&report);
    retval = razer_send_report(usb_dev,&report);
    return retval;
}

int razer_set_none_mode(struct usb_device *usb_dev)
{
    int retval;
    struct razer_report report;
    razer_prepare_report(&report);
    report.parameter_bytes_num = 1;
    report.command = RAZER_BLACKWIDOW_CHROMA_CHANGE_EFFECT; /*change effect command id*/
    report.sub_command = 0;/*none mode id*/
    //report.command_parameters[0] = 0x01; /*profile index? active ?*/
    report.crc = razer_calculate_crc(&report);
    retval = razer_send_report(usb_dev,&report);
    return retval;
}

int razer_set_reactive_mode(struct usb_device *usb_dev,struct razer_rgb *color,unsigned char speed)
{
    int retval;
    struct razer_report report;
    razer_prepare_report(&report);
    report.parameter_bytes_num = 5;
    report.command = RAZER_BLACKWIDOW_CHROMA_CHANGE_EFFECT; /*change effect command id*/
    report.sub_command = RAZER_BLACKWIDOW_CHROMA_EFFECT_REACTIVE;/*reactive mode id*/
    report.command_parameters[0] = speed;/*identified by Oleg Finkelshteyn*/
    report.command_parameters[1] = color->r; /*rgb color definition*/
    report.command_parameters[2] = color->g;
    report.command_parameters[3] = color->b;
    report.crc = razer_calculate_crc(&report);
    retval = razer_send_report(usb_dev,&report);
    return retval;
}

int razer_set_breath_mode(struct usb_device *usb_dev,struct razer_rgb *colors,unsigned char num_cols) //num_cols ? really ??
{
    int retval;
    struct razer_report report;
    razer_prepare_report(&report);
    //report.parameter_bytes_num = (sizeof(struct razer_rgb)*num_cols)+1;
    report.parameter_bytes_num = 8;
    report.command = RAZER_BLACKWIDOW_CHROMA_CHANGE_EFFECT; /*change effect command id*/
    report.sub_command = RAZER_BLACKWIDOW_CHROMA_EFFECT_BREATH;/*breath mode id*/
    report.command_parameters[0] = num_cols;
    memcpy(&report.command_parameters[1],colors,sizeof(struct razer_rgb)*num_cols);
    /*report.command_parameters[1] = first_color->r; //first rgb color definition
    report.command_parameters[2] = first_color->g;
    report.command_parameters[3] = first_color->b;
    report.command_parameters[4] = second_color->r; //second rgb color definition
    report.command_parameters[5] = second_color->g;
    report.command_parameters[6] = second_color->b;*/
    report.crc = razer_calculate_crc(&report);
    retval = razer_send_report(usb_dev,&report);
    return retval;
}

int razer_set_spectrum_mode(struct usb_device *usb_dev)
{
    int retval;
    struct razer_report report;
    razer_prepare_report(&report);
    report.parameter_bytes_num = 1;
    report.command = RAZER_BLACKWIDOW_CHROMA_CHANGE_EFFECT; /*change effect command id*/
    report.sub_command = RAZER_BLACKWIDOW_CHROMA_EFFECT_SPECTRUM;/*spectrum mode id*/
    report.crc = razer_calculate_crc(&report);
    retval = razer_send_report(usb_dev,&report);
    return retval;
}

int razer_set_custom_mode(struct usb_device *usb_dev)
{
    int retval;
    struct razer_report report;
    razer_prepare_report(&report);
    report.parameter_bytes_num = 2;
    report.command = RAZER_BLACKWIDOW_CHROMA_CHANGE_EFFECT; /*change effect command id*/
    report.sub_command = RAZER_BLACKWIDOW_CHROMA_EFFECT_CUSTOM;/*custom mode id*/
    report.command_parameters[0] = 0x01; /*profile index? active ?*/
    report.crc = razer_calculate_crc(&report);
    retval = razer_send_report(usb_dev,&report);
    return retval;
}

int razer_set_static_mode(struct usb_device *usb_dev,struct razer_rgb *color)
{
    int retval;
    struct razer_report report;
    razer_prepare_report(&report);
    report.parameter_bytes_num = 4;
    report.command = RAZER_BLACKWIDOW_CHROMA_CHANGE_EFFECT; /*change effect command id*/
    report.sub_command = RAZER_BLACKWIDOW_CHROMA_EFFECT_STATIC;/*static mode id*/
    report.command_parameters[0] = color->r; /*rgb color definition*/
    report.command_parameters[1] = color->g;
    report.command_parameters[2] = color->b;
    report.crc = razer_calculate_crc(&report);
    retval = razer_send_report(usb_dev,&report);
    return retval;
}

int razer_temp_clear_row(struct usb_device *usb_dev,unsigned char row_index)
{
    int retval;
    struct razer_report report;
    razer_prepare_report(&report);
    report.parameter_bytes_num = 2;
    report.command = RAZER_BLACKWIDOW_CHROMA_CHANGE_EFFECT; /*change effect command id*/
    report.sub_command = RAZER_BLACKWIDOW_CHROMA_EFFECT_CLEAR_ROW;/*clear_row mode id*/
    report.command_parameters[0] = row_index; /*line number starting from top*/
    report.crc = razer_calculate_crc(&report);
    retval = razer_send_report(usb_dev,&report);
    return retval;
}

int razer_set_key_row(struct usb_device *usb_dev,unsigned char row_index,struct razer_row_rgb *row_cols)
{
    //sets the whole row at once -> old values need to be buffered or read from device    printk(KERN_ALERT "setting mode to: Set Keys\n");    
    //0 1=ESC 3=F1 PAUSE=17 20=LOGO
    //1 0=M1 MINUS=21
    //2 = 21
    //3 = 20
    //4 = 21
    //5 = 20 4,5,6,7,8,9,10,12=UNUSED
    int retval;
    struct razer_report report;
    razer_prepare_report(&report);
    //report.parameter_bytes_num = 0x46;//70
    report.parameter_bytes_num = (RAZER_BLACKWIDOW_CHROMA_ROW_LEN+1)*3+4;
    report.command = 0x0B; /*set keys command id*/
    report.sub_command = 0xFF;/*set keys mode id*/
    report.command_parameters[0] = row_index; /*row number*/
    report.command_parameters[1] = 0x0; /*unknown always 0*/
    report.command_parameters[2] = RAZER_BLACKWIDOW_CHROMA_ROW_LEN; /*number of keys in row always 21*/
    memcpy(&report.command_parameters[3],row_cols,sizeof(struct razer_row_rgb));
    report.crc = razer_calculate_crc(&report);
    retval = razer_send_report(usb_dev,&report);
    return retval;
}

int razer_reset(struct usb_device *usb_dev)
{
    int retval;
    struct razer_report report;
    razer_prepare_report(&report);
    report.parameter_bytes_num = 3;
    report.command = 0x0; /*reset command id*/
    report.sub_command = 0x01;/*unknown*/
    report.command_parameters[0] = 8;/*unknown*/
    report.command_parameters[1] = 0;/*unknown*/
    report.crc = razer_calculate_crc(&report);
    retval = razer_send_report(usb_dev,&report);
    return retval;
}

int razer_set_brightness(struct usb_device *usb_dev,unsigned char brightness)
{
    int retval;
    struct razer_report report;
    razer_prepare_report(&report);
    report.parameter_bytes_num = 3;
    report.command = 0x3; /*set brightness command id*/
    report.sub_command = 0x01;/*unknown*/
    report.command_parameters[0] = 5;/*unknown (not speed)*/
    report.command_parameters[1] = brightness;
    report.crc = razer_calculate_crc(&report);
    retval = razer_send_report(usb_dev,&report);
    return retval;
}


int razer_activate_macro_keys(struct usb_device *usb_dev)
{
    int retval;
    struct razer_report report;
    razer_prepare_report(&report);
    report.parameter_bytes_num = 2;
    report.command = 0x4; /*reset command id*/
    report.reserved2 = 0x0;
    report.sub_command = 0x02;/*unknown*/
    report.crc = razer_calculate_crc(&report);
    retval = razer_send_report(usb_dev,&report);
    return retval;
}

//TESTS

int razer_test(struct usb_device *usb_dev)
{
    int retval;
    struct razer_report report;
    razer_prepare_report(&report);
    report.parameter_bytes_num = 2;
    report.command = 0x0; /*init command id ?*/
    report.sub_command = 0x04;/*unknown*/
    //report.command_parameters[0] = 8;/*unknown*/
    //report.command_parameters[1] = 0;/*unknown*/
    report.crc = razer_calculate_crc(&report);
    retval = razer_send_report(usb_dev,&report);
    return retval;
}

void razer_change_effect(struct usb_device *usb_dev,uint effect_id)
{
    struct razer_report report;
    razer_prepare_report(&report);

	switch(effect_id)
	{
        case RAZER_BLACKWIDOW_CHROMA_EFFECT_UNKNOWN3:
            printk(KERN_ALERT "setting mode to: Unknown3\n");//most likely stats profile change    
            report.parameter_bytes_num = 3;
            report.command = 0x01; /*reset command id*/
            report.sub_command = 0x05;/*unknown*/
            report.command_parameters[0] = 0xff;/*unknown*/
            report.command_parameters[0] = 0x01;/*unknown*/
            report.crc = razer_calculate_crc(&report);
            razer_send_report(usb_dev,&report);
            break;
        case RAZER_BLACKWIDOW_CHROMA_EFFECT_UNKNOWN:
            printk(KERN_ALERT "setting mode to: Unknown\n");//most likely stats profile change    
            report.parameter_bytes_num = 3;
            report.command = 0x02; /*reset command id*/
            report.sub_command = 0x00;/*unknown*/
            report.command_parameters[0] = 7;/*unknown*/
            report.command_parameters[1] = 0;/*unknown*/
            report.crc = razer_calculate_crc(&report);
            razer_send_report(usb_dev,&report);
            break;
        case RAZER_BLACKWIDOW_CHROMA_EFFECT_UNKNOWN2:
            printk(KERN_ALERT "setting mode to: Unknown2\n");    
            report.parameter_bytes_num = 3;
            report.command = 0x00; /*reset command id*/
            report.sub_command = 0x00;/*unknown*/
            report.command_parameters[0] = 7;/*unknown*/
            report.command_parameters[1] = 0;/*unknown*/
            report.crc = razer_calculate_crc(&report);
            razer_send_report(usb_dev,&report);
            break;
        case RAZER_BLACKWIDOW_CHROMA_EFFECT_UNKNOWN4:
            printk(KERN_ALERT "setting mode to: Unknown4\n");    
            report.parameter_bytes_num = 3;
            report.command = 0x00; /*reset command id*/
            report.sub_command = 0x01;/*unknown*/
            report.command_parameters[0] = 5;/*unknown*/
            report.command_parameters[1] = 1;/*unknown*/
            report.crc = razer_calculate_crc(&report);
            razer_send_report(usb_dev,&report);
            break;
            //1,8,0 
	}
}

static int razer_raw_event(struct hid_device *hdev,
	struct hid_report *report, u8 *data, int size)
{
    struct usb_interface *intf = to_usb_interface(hdev->dev.parent);
    //struct razer_kbd_device *widow = hid_get_drvdata(hdev);

    if (intf->cur_altsetting->desc.bInterfaceProtocol
	    != USB_INTERFACE_PROTOCOL_MOUSE)
    	return 0;

    return 0;
}

static ssize_t razer_attr_read_test(struct device *dev, struct device_attribute *attr,
                char *buf)                  
{                                   
    //struct usb_interface *intf = to_usb_interface(dev->parent);     
    //struct razer_kbd_device *widow = usb_get_intfdata(intf);           
    return sprintf(buf, "%d\n", 0);            
}

static ssize_t razer_attr_write_test(struct device *dev, struct device_attribute *attr,
               const char *buf, size_t count)       
{                                   
    struct usb_interface *intf = to_usb_interface(dev->parent);     
    //struct razer_kbd_device *widow = usb_get_intfdata(intf);           
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    razer_test(usb_dev);
    return count;                           
}                                   
static ssize_t razer_attr_read_reset(struct device *dev, struct device_attribute *attr,
                char *buf)                  
{                                   
    //struct usb_interface *intf = to_usb_interface(dev->parent);     
    //struct razer_kbd_device *widow = usb_get_intfdata(intf);           
    return sprintf(buf, "%d\n", 0);            
}

static ssize_t razer_attr_write_reset(struct device *dev, struct device_attribute *attr,
               const char *buf, size_t count)       
{                                   
    struct usb_interface *intf = to_usb_interface(dev->parent);     
    //struct razer_kbd_device *widow = usb_get_intfdata(intf);           
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    razer_reset(usb_dev);
    return count;                           
}                                   

static ssize_t razer_attr_read_macro_keys(struct device *dev, struct device_attribute *attr,
                char *buf)                  
{                                   
    //struct usb_interface *intf = to_usb_interface(dev->parent);     
    //struct razer_kbd_device *widow = usb_get_intfdata(intf);           
    return sprintf(buf, "%d\n", 0);            
}

static ssize_t razer_attr_write_macro_keys(struct device *dev, struct device_attribute *attr,
               const char *buf, size_t count)       
{                                   
    struct usb_interface *intf = to_usb_interface(dev->parent);     
    //struct razer_kbd_device *widow = usb_get_intfdata(intf);           
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    //int temp = simple_strtoul(buf, NULL, 10);           
    razer_activate_macro_keys(usb_dev);
    return count;                           
}                                   

static ssize_t razer_attr_read_mode_wave(struct device *dev, struct device_attribute *attr,
                char *buf)                  
{                                   
    //struct usb_interface *intf = to_usb_interface(dev->parent);     
    //struct razer_kbd_device *widow = usb_get_intfdata(intf);           
    return sprintf(buf, "%d\n", 0);            
}

static ssize_t razer_attr_write_mode_wave(struct device *dev, struct device_attribute *attr,
               const char *buf, size_t count)       
{                                   
    struct usb_interface *intf = to_usb_interface(dev->parent);     
    //struct razer_kbd_device *widow = usb_get_intfdata(intf);           
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    int temp = simple_strtoul(buf, NULL, 10);           
    razer_set_wave_mode(usb_dev,temp);
    return count;                           
}                                   

static ssize_t razer_attr_read_mode_spectrum(struct device *dev, struct device_attribute *attr,
                char *buf)                  
{                                   
    //struct usb_interface *intf = to_usb_interface(dev->parent);     
    //struct razer_kbd_device *widow = usb_get_intfdata(intf);           
    return sprintf(buf, "%d\n", 0);            
}

static ssize_t razer_attr_write_mode_spectrum(struct device *dev, struct device_attribute *attr,
               const char *buf, size_t count)       
{                                   
    struct usb_interface *intf = to_usb_interface(dev->parent);     
    //struct razer_kbd_device *widow = usb_get_intfdata(intf);           
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    razer_set_spectrum_mode(usb_dev);
    return count;                           
}                                   

static ssize_t razer_attr_read_mode_none(struct device *dev, struct device_attribute *attr,
                char *buf)                  
{                                   
    //struct usb_interface *intf = to_usb_interface(dev->parent);     
    //struct razer_kbd_device *widow = usb_get_intfdata(intf);           
    return sprintf(buf, "%d\n", 0);            
}

static ssize_t razer_attr_write_mode_none(struct device *dev, struct device_attribute *attr,
               const char *buf, size_t count)       
{                                   
    struct usb_interface *intf = to_usb_interface(dev->parent);     
    //struct razer_kbd_device *widow = usb_get_intfdata(intf);           
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    razer_set_none_mode(usb_dev);
    return count;                           
}                                   

static ssize_t razer_attr_read_set_brightness(struct device *dev, struct device_attribute *attr,
                char *buf)                  
{                                   
    //struct usb_interface *intf = to_usb_interface(dev->parent);     
    //struct razer_kbd_device *widow = usb_get_intfdata(intf);           
    return sprintf(buf, "%d\n", 0);            
}

static ssize_t razer_attr_write_set_brightness(struct device *dev, struct device_attribute *attr,
               const char *buf, size_t count)       
{                                   
    struct usb_interface *intf = to_usb_interface(dev->parent);     
    //struct razer_kbd_device *widow = usb_get_intfdata(intf);           
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    int temp = simple_strtoul(buf, NULL, 10);           
    razer_set_brightness(usb_dev,(unsigned char)temp);
    return count;                           
}                                   


static ssize_t razer_attr_read_mode_reactive(struct device *dev, struct device_attribute *attr,
                char *buf)                  
{                                   
    //struct usb_interface *intf = to_usb_interface(dev->parent);     
    //struct razer_kbd_device *widow = usb_get_intfdata(intf);           
    return sprintf(buf, "%d\n", 0);            
}

static ssize_t razer_attr_write_mode_reactive(struct device *dev, struct device_attribute *attr,
               const char *buf, size_t count)       
{                                   
    struct usb_interface *intf = to_usb_interface(dev->parent);     
    //struct razer_kbd_device *widow = usb_get_intfdata(intf);           
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    //struct razer_rgb color;
    unsigned char speed;
    if(count==4)
    {
        speed = (unsigned char)buf[0];
        //color.r = buf[1];
        //color.g = buf[2];
        //color.b = buf[3];
        //razer_set_reactive_mode(usb_dev,&color,speed);
        razer_set_reactive_mode(usb_dev,(struct razer_rgb*)&buf[1],speed);
    }
    return count;                           
}                                   

static ssize_t razer_attr_read_mode_breath(struct device *dev, struct device_attribute *attr,
                char *buf)                  
{                                   
    //struct usb_interface *intf = to_usb_interface(dev->parent);     
    //struct razer_kbd_device *widow = usb_get_intfdata(intf);           
    return sprintf(buf, "%d\n", 0);            
}

static ssize_t razer_attr_write_mode_breath(struct device *dev, struct device_attribute *attr,
               const char *buf, size_t count)       
{                                   
    struct usb_interface *intf = to_usb_interface(dev->parent);     
    //struct razer_kbd_device *widow = usb_get_intfdata(intf);           
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    unsigned char num_cols;
    if(count)
        num_cols = (unsigned char)buf[0];
    if(count==((num_cols*3)+1))
    {
        razer_set_breath_mode(usb_dev,(struct razer_rgb*)&buf[1],num_cols);
    }
    return count;                           
}                                   

static ssize_t razer_attr_read_mode_custom(struct device *dev, struct device_attribute *attr,
                char *buf)                  
{                                   
    //struct usb_interface *intf = to_usb_interface(dev->parent);     
    //struct razer_kbd_device *widow = usb_get_intfdata(intf);           
    return sprintf(buf, "%d\n", 0);            
}

static ssize_t razer_attr_write_mode_custom(struct device *dev, struct device_attribute *attr,
               const char *buf, size_t count)       
{                                   
    struct usb_interface *intf = to_usb_interface(dev->parent);     
    //struct razer_kbd_device *widow = usb_get_intfdata(intf);           
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    razer_set_custom_mode(usb_dev);
    return count;                           
}                                   

static ssize_t razer_attr_read_mode_static(struct device *dev, struct device_attribute *attr,
                char *buf)                  
{                                   
    //struct usb_interface *intf = to_usb_interface(dev->parent);     
    //struct razer_kbd_device *widow = usb_get_intfdata(intf);           
    return sprintf(buf, "%d\n", 0);            
}

static ssize_t razer_attr_write_mode_static(struct device *dev, struct device_attribute *attr,
               const char *buf, size_t count)       
{                                   
    struct usb_interface *intf = to_usb_interface(dev->parent);     
    //struct razer_kbd_device *widow = usb_get_intfdata(intf);           
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_rgb color;
    if(count==3)
    {
        color.r = buf[0];
        color.g = buf[1];
        color.b = buf[2];
        razer_set_static_mode(usb_dev,&color);
    }
    return count;                           
}                                   

static ssize_t razer_attr_read_temp_clear_row(struct device *dev, struct device_attribute *attr,
                char *buf)                  
{                                   
    //struct usb_interface *intf = to_usb_interface(dev->parent);     
    //struct razer_kbd_device *widow = usb_get_intfdata(intf);           
    return sprintf(buf, "%d\n", 0);            
}

static ssize_t razer_attr_write_temp_clear_row(struct device *dev, struct device_attribute *attr,
               const char *buf, size_t count)       
{                                   
    struct usb_interface *intf = to_usb_interface(dev->parent);     
    //struct razer_kbd_device *widow = usb_get_intfdata(intf);           
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    int temp = simple_strtoul(buf, NULL, 10);           
    razer_temp_clear_row(usb_dev,temp);
    return count;                           
}                                   

static ssize_t razer_attr_read_set_key_row(struct device *dev, struct device_attribute *attr,
                char *buf)                  
{                                   
    //struct usb_interface *intf = to_usb_interface(dev->parent);     
    //struct razer_kbd_device *widow = usb_get_intfdata(intf);           
    return sprintf(buf, "%d\n", 0);            
}

static ssize_t razer_attr_write_set_key_row(struct device *dev, struct device_attribute *attr,
               const char *buf, size_t count)       
{                                   
    struct usb_interface *intf = to_usb_interface(dev->parent);     
    //struct razer_kbd_device *widow = usb_get_intfdata(intf);           
    struct usb_device *usb_dev = interface_to_usbdev(intf);
    size_t buf_size = (RAZER_BLACKWIDOW_CHROMA_ROW_LEN+1)*3 + 1;
    //printk(KERN_ALERT "sizeof(razer_row_rgb): %d\n",sizeof(struct razer_row_rgb));
    size_t offset = 0;
    while(offset<count)
    {
        unsigned char row_index = (unsigned char)buf[offset];
        if(count-offset < buf_size)
        {
            printk(KERN_ALERT "Wrong Amount of RGB data provided: %d of %d\n",(int)count,(int)buf_size);
            return 0;
        }
        razer_set_key_row(usb_dev,row_index,(struct razer_row_rgb*)&buf[offset+1]);
        offset += buf_size;
    }
    return count;                           
}                                   




static DEVICE_ATTR(mode_wave, S_IWUSR | S_IWGRP | S_IRUGO, razer_attr_read_mode_wave, razer_attr_write_mode_wave);
static DEVICE_ATTR(mode_spectrum, S_IWUSR | S_IWGRP | S_IRUGO, razer_attr_read_mode_spectrum, razer_attr_write_mode_spectrum);
static DEVICE_ATTR(mode_none, S_IWUSR | S_IWGRP | S_IRUGO, razer_attr_read_mode_none, razer_attr_write_mode_none);
static DEVICE_ATTR(mode_reactive, S_IWUSR | S_IWGRP | S_IRUGO, razer_attr_read_mode_reactive, razer_attr_write_mode_reactive);
static DEVICE_ATTR(mode_breath, S_IWUSR | S_IWGRP | S_IRUGO, razer_attr_read_mode_breath, razer_attr_write_mode_breath);
static DEVICE_ATTR(mode_custom, S_IWUSR | S_IWGRP | S_IRUGO, razer_attr_read_mode_custom, razer_attr_write_mode_custom);
static DEVICE_ATTR(mode_static, S_IWUSR | S_IWGRP | S_IRUGO, razer_attr_read_mode_static, razer_attr_write_mode_static);
static DEVICE_ATTR(temp_clear_row, S_IWUSR | S_IWGRP | S_IRUGO, razer_attr_read_temp_clear_row, razer_attr_write_temp_clear_row);
static DEVICE_ATTR(set_key_row, S_IWUSR | S_IWGRP | S_IRUGO, razer_attr_read_set_key_row, razer_attr_write_set_key_row);
static DEVICE_ATTR(reset, S_IWUSR | S_IWGRP | S_IRUGO, razer_attr_read_reset, razer_attr_write_reset);
static DEVICE_ATTR(macro_keys, S_IWUSR | S_IWGRP | S_IRUGO, razer_attr_read_macro_keys, razer_attr_write_macro_keys);
static DEVICE_ATTR(set_brightness, S_IWUSR | S_IWGRP | S_IRUGO, razer_attr_read_set_brightness, razer_attr_write_set_brightness);
static DEVICE_ATTR(test, S_IWUSR | S_IWGRP | S_IRUGO, razer_attr_read_test, razer_attr_write_test);



static int razer_kbd_probe(struct hid_device *hdev,
			 const struct hid_device_id *id)
{
    int retval;
    struct usb_interface *intf = to_usb_interface(hdev->dev.parent);
    //struct usb_device *usb_dev = interface_to_usbdev(intf);
    struct razer_kbd_device *dev = NULL;

    dev = kzalloc(sizeof(struct razer_kbd_device),GFP_KERNEL);
    if(dev == NULL) {
        dev_err(&intf->dev, "out of memory\n");
        retval = -ENOMEM;
        goto exit;
    }
    
    retval = device_create_file(&hdev->dev, &dev_attr_mode_wave);
    if (retval)
        goto exit_free;
    retval = device_create_file(&hdev->dev, &dev_attr_mode_spectrum);
    if (retval)
        goto exit_free;
    retval = device_create_file(&hdev->dev, &dev_attr_mode_none);
    if (retval)
        goto exit_free;
    retval = device_create_file(&hdev->dev, &dev_attr_mode_reactive);
    if (retval)
        goto exit_free;
    retval = device_create_file(&hdev->dev, &dev_attr_mode_breath);
    if (retval)
        goto exit_free;
    retval = device_create_file(&hdev->dev, &dev_attr_mode_custom);
    if (retval)
        goto exit_free;
    retval = device_create_file(&hdev->dev, &dev_attr_mode_static);
    if (retval)
        goto exit_free;
    retval = device_create_file(&hdev->dev, &dev_attr_temp_clear_row);
    if (retval)
        goto exit_free;
    retval = device_create_file(&hdev->dev, &dev_attr_set_key_row);
    if (retval)
        goto exit_free;
    retval = device_create_file(&hdev->dev, &dev_attr_reset);
    if (retval)
        goto exit_free;
    retval = device_create_file(&hdev->dev, &dev_attr_macro_keys);
    if (retval)
        goto exit_free;
    retval = device_create_file(&hdev->dev, &dev_attr_set_brightness);
    if (retval)
        goto exit_free;
    retval = device_create_file(&hdev->dev, &dev_attr_test);
    if (retval)
        goto exit_free;

    hid_set_drvdata(hdev, dev);

    retval = hid_parse(hdev);
    if(retval)	{
    	hid_err(hdev,"parse failed\n");
	   goto exit_free;
    }
    retval = hid_hw_start(hdev,HID_CONNECT_DEFAULT);
    if (retval) {
    	hid_err(hdev,"hw start failed\n");
	   goto exit_free;
    }

    //razer_reset(usb_dev);
    //razer_activate_macro_keys(usb_dev);
    //msleep(3000);
    return 0;
exit:
    return retval;	
exit_free:
    kfree(dev);
    return retval;
}

static void razer_kbd_disconnect(struct hid_device *hdev)
{
    struct razer_kbd_device *dev;
    struct usb_interface *intf = to_usb_interface(hdev->dev.parent);
    dev = hid_get_drvdata(hdev);
    device_remove_file(&hdev->dev, &dev_attr_mode_wave);
    device_remove_file(&hdev->dev, &dev_attr_mode_spectrum);
    device_remove_file(&hdev->dev, &dev_attr_mode_none);
    device_remove_file(&hdev->dev, &dev_attr_mode_reactive);
    device_remove_file(&hdev->dev, &dev_attr_mode_breath);
    device_remove_file(&hdev->dev, &dev_attr_mode_custom);
    device_remove_file(&hdev->dev, &dev_attr_mode_static);
    device_remove_file(&hdev->dev, &dev_attr_temp_clear_row);
    device_remove_file(&hdev->dev, &dev_attr_set_key_row);
    device_remove_file(&hdev->dev, &dev_attr_reset);
    device_remove_file(&hdev->dev, &dev_attr_macro_keys);
    device_remove_file(&hdev->dev, &dev_attr_set_brightness);
    device_remove_file(&hdev->dev, &dev_attr_test);

    hid_hw_stop(hdev);
    kfree(dev);
    dev_info(&intf->dev, "Razer Chroma Device disconnected\n");
}

static const struct hid_device_id razer_devices[] = {
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_BLACKWIDOW_CHROMA) },
    { HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_FIREFLY) },
    { }
};

MODULE_DEVICE_TABLE(hid, razer_devices);

static struct hid_driver razer_kbd_driver = {
	.name =		"razerkbd",
	.id_table =	razer_devices,
	.probe =	razer_kbd_probe,
	.remove =	razer_kbd_disconnect,
	.raw_event = razer_raw_event
};

module_hid_driver(razer_kbd_driver);


