#ifndef MODULE_H_
#define MODULE_H_

#include "lusb0_usb.h"

struct device_attribute {
	const char              *name;
	ssize_t(*show)(struct device *dev, struct device_attribute *attr, char *buf);
	ssize_t(*store)(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
};


#endif /* MODULE_H_ */
