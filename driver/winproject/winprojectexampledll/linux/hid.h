#ifndef HID_H_
#define HID_H_

struct device_private {
	struct usb_dev_handle*	udev;
};

struct device {
	struct usb_device				*parent;
	struct device_private			p;
	const char						*init_name;
	void							*driver_data;
};

struct hid_device {                                                     /* device report descriptor */
	struct device dev;                                              /* device */
	struct hid_ll_driver *ll_driver;
	unsigned int status;                                            /* see STAT flags above */
	struct hid_driver *driver;
};


#endif /* HID_H_ */
