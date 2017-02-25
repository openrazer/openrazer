#ifndef MODULE_H_
#define MODULE_H_

#include <stdio.h>
#include <Winusb.h>

#define MODULE_AUTHOR( __Declaration__ )
#define MODULE_DESCRIPTION( __Declaration__ )
#define MODULE_LICENSE( __Declaration__ )

#define USB_CTRL_SET_TIMEOUT    5000

#define USB_DIR_OUT                     0
#define USB_DIR_IN                      0x80

#define USB_TYPE_CLASS			(0x01 << 5)
#define USB_RECIP_INTERFACE	0x01

#define usb_sndctrlpipe(u,d) 0
#define usb_rcvctrlpipe(u,d) 0

#define PATH_MAX 512

struct usb_interface_descriptor {
    unsigned char  bInterfaceProtocol;
};

struct usb_host_interface {
	struct usb_interface_descriptor desc;
};

struct usb_interface {
    int num_altsetting;
	struct usb_device* dev;
	struct usb_host_interface *cur_altsetting;
};

struct usb_device_descriptor {
    unsigned short idVendor;
    unsigned short idProduct;
};

struct usb_device {
    char filename[PATH_MAX];
	void *dev;
    struct usb_device_descriptor descriptor;
};

inline int usb_control_msg(
	struct usb_device *usb_dev
	, int usb_pipe
	, unsigned int request
	, unsigned int request_type
	, unsigned int value
	, unsigned int report_index
	, unsigned char* buf, unsigned int size
	, unsigned int timeout)
{
	WINUSB_SETUP_PACKET packet;
	packet.RequestType = request_type;
	packet.Request = request;
	packet.Value = value;
	packet.Index = report_index; 
	packet.Length = size;
	ULONG cbSent = 0;
	if (!WinUsb_ControlTransfer(usb_dev->dev, packet, buf, size, &cbSent, 0))
		printf("\tfailed\tWinUsb_ControlTransfer\n\n");

	return cbSent;
}

inline struct usb_interface *to_usb_interface(struct usb_device *parent) {
//	parent->config->interface->dev = parent;
//	return parent->config->interface;
	return (struct usb_interface*)parent;
}

inline struct usb_device *interface_to_usbdev(struct usb_interface *intf) {
//	return intf->dev;
	return (struct usb_device*)intf;
}

inline void usb_disable_autosuspend(struct usb_device *usb_dev) {
	printf("usb_disable_autosuspend\n");
}

struct device_attribute {
	const char* name;
	ssize_t(*show)(struct device *dev, struct device_attribute *attr, char *buf);
	ssize_t(*store)(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
};

inline int device_create_file(struct device *device, struct device_attribute *entry) {
	printf("device_create_file %s\n", entry->name);
	return 0;
}

inline void device_remove_file(struct device *device, struct device_attribute *entry) {
	printf("device_remove_file %s\n", entry->name);
}

#define HID_USB_DEVICE(ven, prod) \
	  .vendor = (ven) \
	, .product = (prod)


#define __stringify(x)       #x

// Hack to turn Linux device macros into API calls
#define DEVICE_ATTR1(_device,_name, _mode, _show, _store)	\
	struct device_attribute dev_attr_##_name; \
	DLL_INTERNAL struct device_attribute dev##_device##_attr_##_name = {	\
          .name = __stringify(_name)				\
        , .show   = _show							\
        , .store  = _store							\
	};

#define MODULE_DEVICE_TABLE(type, name)

/*typedef struct hid_device_array_tag {
	unsigned int count;
	struct hid_device* hdev[];
} hid_device_array;*/

#define module_hid_driver(hdr) \
DLL_INTERNAL unsigned int init_##hdr## (struct hid_device** hdevo) {	\
	unsigned int numHdevs = 0; \
	struct hid_device* hdev = NULL; \
	openChromaDevice(&hdev, &numHdevs, hdr); \
	*hdevo = hdev; \
	return numHdevs; \
}

#endif /* MODULE_H_ */
