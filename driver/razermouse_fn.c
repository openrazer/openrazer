
//#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/device.h>
#include <linux/hid.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/usb/input.h>

#include "razermouse_driver.h"
#include "razercommon.h"

#define RAZER_FN_KEY 194 // 194 = KEY_F24
#define RAZER_MACRO_KEY 188 // 188 = KEY_F18
#define RAZER_GAME_KEY 189 // 189 = KEY_F19

#define KEY_FLAG_BLOCK 0b00000001

// M1 = F13
// M2 = F14
// M3 = F15
// M4 = F16
// M5 = F17


struct razer_sc {
	unsigned int fn_on;
};

struct razer_key_translation {
	u16 from;
	u16 to;
	u8 flags;
};

static const struct razer_key_translation chroma_keys[] = {
	{ KEY_F1,	KEY_MUTE },
	{ KEY_F2,	KEY_VOLUMEDOWN },
	{ KEY_F3,	KEY_VOLUMEUP },
	
	{ KEY_F5,	KEY_PREVIOUSSONG },
	{ KEY_F6,   KEY_PLAYPAUSE },
    { KEY_F7,   KEY_NEXTSONG },
	
	{ KEY_F9,   RAZER_MACRO_KEY },
    { KEY_F10,  RAZER_GAME_KEY },
	{ KEY_F11,  0, KEY_FLAG_BLOCK },
    { KEY_F12,  0, KEY_FLAG_BLOCK },
    
    { KEY_PAUSE, KEY_SLEEP },
    
    // Custom bind
    { KEY_KPENTER, KEY_CALC },
	{ }
};

static const struct razer_key_translation *find_translation(const struct razer_key_translation *key_table, u16 from) {
	const struct razer_key_translation *result;
	
	for (result = key_table; result->from; result++) {
		if (result->from == from) {
			return result;
		}
	}
	
	return NULL;
}



static int apple_event(struct hid_device *hdev, struct hid_field *field,
		struct hid_usage *usage, __s32 value)
{
	struct razer_sc *asc = hid_get_drvdata(hdev);
	const struct razer_key_translation *translation;

	printk(KERN_WARNING "razermouse_fn: Usage:'%u', Type:'%u', Value: '%d", usage->code, usage->type, value);
	
	/*
    
    if (usage->code == RAZER_FN_KEY) {
		asc->fn_on = !!value;
		
		input_event(field->hidinput->input, usage->type, usage->code, value);
		//printk(KERN_WARNING "razerkbd_fn: FN: %u", asc->fn_on);
		return 1;
	}
	
	// Do translation, currently translation is only when FN is pressed
	if (asc->fn_on) {
		// Look at https://github.com/torvalds/linux/blob/master/drivers/hid/hid-apple.c#L206 for reversing the FN keys, though blade does that in h/w
		
		translation = find_translation(chroma_keys, usage->code);
		
		if (translation) {
			// translate != NULL, aka a translation is found
			
			if (!(translation->flags & KEY_FLAG_BLOCK)) {
				input_event(field->hidinput->input, usage->type, translation->to, value);
			}
			
			return 1;
		}
	}*/
    
    
    

	return 0;
}

static int apple_input_mapping(struct hid_device *hdev, struct hid_input *hi,
		struct hid_field *field, struct hid_usage *usage,
		unsigned long **bit, int *max)
{
	//const struct razer_key_translation *trans;
	
	//if (usage->hid == (HID_UP_CUSTOM | 0x0003)) {
		/* The fn key on Apple USB keyboards */
	set_bit(EV_REP, hi->input->evbit);
	//	hid_map_usage_clear(hi, usage, bit, max, EV_KEY, RAZER_FN_KEY);
	//	for (trans = chroma_keys; trans->from; trans++) {
	//		set_bit(trans->to, hi->input->keybit);
	//	}
		
	//	return 1;
	//}

	/* we want the hid layer to go through standard path (set and ignore) */
	return 0;
}

static int razer_probe(struct hid_device *hdev, const struct hid_device_id *id)
{
	struct razer_sc *asc;
	unsigned int connect_mask = HID_CONNECT_DEFAULT;
	int ret;

	asc = devm_kzalloc(&hdev->dev, sizeof(*asc), GFP_KERNEL);
	
	//struct usb_interface *intf = to_usb_interface(hdev->dev.parent);
    //printk(KERN_WARNING "razerkbd_fn: Dev: %u", intf->altsetting->desc.bInterfaceNumber);
	
	if (asc == NULL) {
		hid_err(hdev, "can't alloc apple descriptor\n");
		return -ENOMEM;
	}

	hid_set_drvdata(hdev, asc);

	ret = hid_parse(hdev);
	if (ret) {
		hid_err(hdev, "parse failed\n");
		return ret;
	}

	ret = hid_hw_start(hdev, connect_mask);
	if (ret) {
		hid_err(hdev, "hw start failed\n");
		return ret;
	}

	return 0;
}

static const struct hid_device_id razer_devices[] = {
	{ HID_USB_DEVICE(USB_VENDOR_ID_RAZER,USB_DEVICE_ID_RAZER_MAMBA) },
	{ }
};
MODULE_DEVICE_TABLE(hid, razer_devices);

static struct hid_driver razer_driver = {
	.name = "razermouse_fn",
	.id_table = razer_devices,
	.probe = razer_probe,
	.event = apple_event,
	.input_mapping = apple_input_mapping,
};
module_hid_driver(razer_driver);

MODULE_LICENSE("GPL");
