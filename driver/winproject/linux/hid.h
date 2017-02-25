#ifndef HID_H_
#define HID_H_

#include <SetupAPI.h>
#include <cfgmgr32.h>
#include <Winusb.h>

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "Winusb.lib")

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

static const GUID GUID_DEVINTERFACE = { 0xDEE824EF, 0x729B, 0x4A0E, 0x9C, 0x14, 0xB7, 0x11, 0x7D, 0x33, 0xA8, 0x17 };

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
//	printf("dev_err device=%s msg=%s", (*dev)->filename, msg);
}

inline void dev_info(struct usb_device** dev, const char* msg) {
//	printf("dev_info device=%s msg=%s", (*dev)->filename, msg);
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
	HDEVINFO hDevInfo = SetupDiGetClassDevs(&GUID_DEVINTERFACE, 0, 0, DIGCF_DEVICEINTERFACE);
    if (hDevInfo == INVALID_HANDLE_VALUE) {
		printf("SetupDiGetClassDevs failed\n");
		return;
	}

	for (unsigned int  i = 0; hdr.id_table[i].vendor != 0; i++) {
		SP_DEVINFO_DATA deviceData = { 0 };
		deviceData.cbSize = sizeof(SP_DEVINFO_DATA);

		for (unsigned int j = 0; SetupDiEnumDeviceInfo(hDevInfo, j, &deviceData); ++j) {
			char deviceID[MAX_DEVICE_ID_LEN];
			if (CM_Get_Device_ID(deviceData.DevInst, deviceID, MAX_DEVICE_ID_LEN, 0))
				continue;

			char* vid = strstr(deviceID, "VID_");
			if (!vid || hdr.id_table[i].vendor != strtoul(vid+4, NULL, 16))
				continue;

			char* pid = strstr(deviceID, "PID_");
			if (!pid || hdr.id_table[i].product != strtoul(pid+4, NULL, 16))
				continue;

			/*char* mi = strstr(deviceID, "MI_");
			if (mi) {
				printf("vid found (%s)\n", vid+4);
				printf("pid found (%s)\n", pid+4);
				printf("mi found (%s)\n", mi+3);
			}*/

			SP_INTERFACE_DEVICE_DATA interfaceData = { 0 };
			interfaceData.cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);
			if (!SetupDiEnumDeviceInterfaces(hDevInfo, &deviceData, &GUID_DEVINTERFACE, 0, &interfaceData))
				continue;

			DWORD dwRequiredSize = 0;
			SetupDiGetDeviceInterfaceDetail(hDevInfo, &interfaceData, 0, 0, &dwRequiredSize, 0);
			SP_INTERFACE_DEVICE_DETAIL_DATA* pData = (SP_INTERFACE_DEVICE_DETAIL_DATA*)malloc(dwRequiredSize);
			pData->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
			if (!SetupDiGetDeviceInterfaceDetail(hDevInfo, &interfaceData, pData, dwRequiredSize, 0, 0)) {
				free(pData);
				continue;
			}

			HANDLE hDevice = CreateFile(pData->DevicePath
				, GENERIC_READ | GENERIC_WRITE
				, FILE_SHARE_READ | FILE_SHARE_WRITE
				, 0
				, OPEN_EXISTING
				, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED
				, 0);
			if (hDevice == INVALID_HANDLE_VALUE) {
				free(pData);
				continue;
			}

			WINUSB_INTERFACE_HANDLE hWinUSBHandle;
			if (!WinUsb_Initialize(hDevice, &hWinUSBHandle))
				continue;

			SetupDiDestroyDeviceInfoList(hDevInfo);

			printf("CM_Get_Device_ID (%s)\n", deviceID);
			printf("device %04X:%04X opened!\n", hdr.id_table[i].vendor, hdr.id_table[i].product);

			*hdev = (struct hid_device*)realloc(*hdev, (*numHdev+1) * sizeof(struct hid_device));
			if (!*hdev) {
				printf("out of memory\n");
				continue;
			}

			(*hdev)[*numHdev].dev.parent = (struct usb_device*)malloc(sizeof(struct usb_device));
			(*hdev)[*numHdev].dev.parent->descriptor.idVendor = hdr.id_table[i].vendor;
			(*hdev)[*numHdev].dev.parent->descriptor.idProduct = hdr.id_table[i].product;
			(*hdev)[*numHdev].dev.parent->filename;
			(*hdev)[*numHdev].dev.parent->dev = hWinUSBHandle; // use this for the handle
			(*hdev)[*numHdev].dev.init_name = hdr.name;
			(*hdev)[*numHdev].status = 1;
			(*hdev)[*numHdev].driver = &hdr;

			(*numHdev)++;
		}
		if (!numHdev)
			printf("device %04X:%04X NOT opened!\n", hdr.id_table[i].vendor, hdr.id_table[i].product);
	}
}

#endif /* HID_H_ */
