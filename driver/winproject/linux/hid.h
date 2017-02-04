#ifndef HID_H_
#define HID_H_

#define HID_STAT_ADDED					1
#define HID_STAT_PARSED					2

#define HID_CONNECT_HIDINPUT            0x01
#define HID_CONNECT_HIDRAW              0x04
#define HID_CONNECT_HIDDEV              0x08
#define HID_CONNECT_FF                  0x20
#define HID_CONNECT_DEFAULT     (HID_CONNECT_HIDINPUT|HID_CONNECT_HIDRAW| HID_CONNECT_HIDDEV|HID_CONNECT_FF)

#define HID_REQ_GET_REPORT              0x01
#define HID_REQ_SET_REPORT				0x09

#define USB_INTERFACE_PROTOCOL_KEYBOARD 1
#define USB_INTERFACE_PROTOCOL_MOUSE    2

struct hid_input {
	struct input_dev *input;
};

struct hid_field {
	struct hid_input *hidinput;     /* associated input structure */
};

struct hid_usage {
	__u16     code;                 /* input driver code */
	__u8      type;                 /* input driver type */
};

struct hid_driver {
	char *name;
	const struct hid_device_id *id_table;
	int (*probe)(struct hid_device *dev, const struct hid_device_id *id);
	void (*remove)(struct hid_device *dev);
	int (*raw_event)(struct hid_device *hdev, struct hid_report *report, u8 *data, int size);
	int (*event)(struct hid_device *hdev, struct hid_field *field, struct hid_usage *usage, __s32 value);
	int (*input_configured)(struct hid_device *hdev,
					struct hid_input *hidinput);

	int (*input_mapping)(struct hid_device *hdev,
					struct hid_input *hidinput, struct hid_field *field,
					struct hid_usage *usage, unsigned long **bit, int *max);
};

struct hid_device_id {
	__u16 bus;
	__u32 vendor;
	__u32 product;
};

struct device_private {
	struct usb_dev_handle*	udev;
	void*					hPipe;
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

struct hid_ll_driver {
	int (*start)(struct hid_device *hdev);
	void (*stop)(struct hid_device *hdev);
	int (*parse)(struct hid_device *hdev);
};

inline int ll_start(struct hid_device *hdev) {
	printf("ll_start\n");
	return 0;
}

inline void ll_stop(struct hid_device *hdev) {
	printf("ll_stop\n");
}

inline int ll_parse(struct hid_device *hdev) {
	printf("ll_parse\n");
	return 0;
}

inline void dev_err(struct usb_device** dev, const char* msg) {
	printf("dev_err device=%s msg=%s", (*dev)->filename, msg);
}

inline void dev_info(struct usb_device** dev, const char* msg) {
	printf("dev_info device=%s msg=%s", (*dev)->filename, msg);
}

inline void *dev_get_drvdata(const struct device *dev) {
	return dev->driver_data;
}

inline void dev_set_drvdata(struct device *dev, void *data) {
	dev->driver_data = data;
}

inline int hid_connect(struct hid_device *hdev, unsigned int connect_mask) {
	printf("hid_connect\n");
	return 0;
}

inline int hid_parse(struct hid_device *hdev) {
	int ret;

	if (hdev->status & HID_STAT_PARSED)
		return 0;

	ret = hdev->ll_driver->parse(hdev);
	if (!ret)
		hdev->status |= HID_STAT_PARSED;

	return ret;
}

inline void *hid_get_drvdata(struct hid_device *hdev) {
	return dev_get_drvdata(&hdev->dev);
}

inline void hid_set_drvdata(struct hid_device *hdev, void *data) {
	dev_set_drvdata(&hdev->dev, data);
}

inline int hid_hw_start(struct hid_device *hdev, unsigned int connect_mask) {
	int ret = hdev->ll_driver->start(hdev);
	if (ret || !connect_mask)
		return ret;
	ret = hid_connect(hdev, connect_mask);
	if (ret)
		hdev->ll_driver->stop(hdev);
	return ret;
}

inline void hid_hw_stop(struct hid_device *hdev) {
	hdev->ll_driver->stop(hdev);
}

inline void hid_err(struct hid_device *hdev, const char* msg, ...) {
	va_list args;
	va_start(args, msg);
	printf("hid_err device=%s", hdev->dev.init_name);
	printf(msg, args);
	va_end(args);
}

inline void openChromaDevice(struct hid_device** hdev, unsigned int* numHdev, struct hid_driver hdr) {
	for (struct usb_bus* bus = usb_get_busses(); bus; bus = bus->next) {
		for (struct usb_device* dev = bus->devices; dev; dev = dev->next) {
			for (unsigned int  i = 0; hdr.id_table[i].vendor != 0; i++) {
				unsigned int vid = hdr.id_table[i].vendor;
				unsigned int pid = hdr.id_table[i].product;
				if (dev->descriptor.idVendor == vid && dev->descriptor.idProduct == pid) {
					struct usb_dev_handle* udev;
					if (udev = usb_open(dev)) {
						struct usb_config_descriptor* config_descriptor;
						if (dev->descriptor.bNumConfigurations && (config_descriptor = &dev->config[0])) {
							for (int intfIndex = 0; intfIndex < config_descriptor->bNumInterfaces; intfIndex++) {
								if (config_descriptor->interface[intfIndex].num_altsetting) {
									struct usb_interface_descriptor* intf = &config_descriptor->interface[intfIndex].altsetting[0];
									if (intf->bInterfaceNumber == 0 && intf->bAlternateSetting == 0) {
										printf("device %04X:%04X opened!\n", vid, pid);
										struct hid_device* h = (struct hid_device*)realloc(*hdev, (*numHdev+1) * sizeof(struct hid_device));
										if (!h) {
											printf("out of memory\n");
											return;
										}
										(*hdev) = h;
										struct hid_device h2;
										h2.dev.parent = dev;
										h2.dev.init_name = hdr.name;
										h2.status = 1;
										h2.driver = &hdr;
										dev->dev = udev; // use this for the handle
										/*
										h2.driver = (struct hid_driver*)malloc(sizeof(struct hid_driver));
										h2.driver->name = hdr.name;
										struct hid_device_id id;
										id.vendor = vid;
										id.product = pid;
										h2.driver->id_table = &id;
										*/
										/*
										h2.driver->probe = razer_probe;
										h2.ll_driver = (struct hid_ll_driver*)malloc(sizeof(struct hid_ll_driver));
										h2.ll_driver->parse = ll_parse;
										h2.ll_driver->start = ll_start;
										h2.ll_driver->stop = ll_stop;


										//hdev->driver->probe(hdev, id);
										*/


										/*
										config_descriptor->interface[intfIndex].cur_altsetting = (struct usb_cur_altsetting*)malloc(sizeof(struct usb_cur_altsetting));
										config_descriptor->interface[intfIndex].cur_altsetting->desc = config_descriptor->interface[intfIndex].altsetting[0];
										if (strcmp(h2.driver->name, "razerkbd")==0)
											config_descriptor->interface[intfIndex].cur_altsetting->desc.bInterfaceProtocol = USB_INTERFACE_PROTOCOL_MOUSE;
										h2.driver->probe(&h2, &hdr.id_table[i]);
										*/



										(*hdev)[*numHdev] = h2;
										(*numHdev)++;
									} else {
										usb_close(udev);
									}
								} else {
									usb_close(udev);
								}
							}
						} else {
							usb_close(udev);
						}
					}
				}
				if (!numHdev)
					printf("device %04X:%04X NOT opened!\n", vid, pid);
			}
		}
	}
}

#endif /* HID_H_ */
