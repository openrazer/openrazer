// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Razer Wolverine V3 Pro USB Gamepad Driver
 * 
 * Driver for Razer Wolverine V3 Pro gaming controller with full input support.
 * The controller uses a vendor-specific USB interface for gamepad input.
 * 
 * For wireless mode (dongle), the input device is only registered when the
 * controller is actually powered on and sending data, preventing Steam and
 * other applications from seeing a phantom controller.
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb.h>
#include <linux/usb/input.h>
#include <linux/input.h>
#include <linux/device.h>
#include <linux/jiffies.h>

#include "razerwolverine_driver.h"

MODULE_AUTHOR("OpenRazer Team");
MODULE_DESCRIPTION("Razer Wolverine V3 Pro USB Gamepad Driver");
MODULE_LICENSE("GPL");
MODULE_SOFTDEP("pre: xpad");

/* USB device ID table 
 * Match the entire device (all interfaces) to ensure we get priority over xpad
 */
static const struct usb_device_id wolverine_table[] = {
	{ USB_DEVICE(USB_VENDOR_ID_RAZER, USB_DEVICE_ID_RAZER_WOLVERINE_V3_PRO_WIRED) },
	{ USB_DEVICE(USB_VENDOR_ID_RAZER, USB_DEVICE_ID_RAZER_WOLVERINE_V3_PRO_WIRELESS) },
	{ }
};
MODULE_DEVICE_TABLE(usb, wolverine_table);

/* Forward declarations */
static int wolverine_setup_input(struct wolverine_device *wv);

/**
 * Work handler - register input device when controller starts sending data
 * Called from workqueue context (safe to sleep/block)
 */
static void wolverine_connect_work(struct work_struct *work)
{
	struct wolverine_device *wv = container_of(work, struct wolverine_device, connect_work);
	int error;
	
	mutex_lock(&wv->reg_lock);
	
	if (wv->shutting_down || wv->input_registered) {
		mutex_unlock(&wv->reg_lock);
		return;
	}
	
	error = input_register_device(wv->input);
	if (error) {
		dev_err(&wv->intf->dev, "Failed to register input device: %d\n", error);
		mutex_unlock(&wv->reg_lock);
		return;
	}
	
	wv->input_registered = true;
	dev_info(&wv->intf->dev, "Razer Wolverine V3 Pro controller connected\n");
	
	/* Start monitoring for disconnect */
	schedule_delayed_work(&wv->disconnect_work, WOLVERINE_DISCONNECT_TIMEOUT);
	
	mutex_unlock(&wv->reg_lock);
}

/**
 * Delayed work handler - check if controller has stopped sending data
 * Called from workqueue context (safe to sleep/block)
 */
static void wolverine_disconnect_work(struct work_struct *work)
{
	struct wolverine_device *wv = container_of(work, struct wolverine_device,
						   disconnect_work.work);
	
	mutex_lock(&wv->reg_lock);
	
	if (wv->shutting_down || !wv->input_registered) {
		mutex_unlock(&wv->reg_lock);
		return;
	}
	
	/* Check if we haven't received data for the timeout period */
	if (time_after(jiffies, wv->last_packet_time + WOLVERINE_DISCONNECT_TIMEOUT)) {
		/* Controller stopped sending - unregister input device */
		input_unregister_device(wv->input);
		wv->input_registered = false;
		atomic_set(&wv->controller_connected, 0);
		
		/* Allocate a new input device for next connection */
		wv->input = input_allocate_device();
		if (wv->input) {
			wv->input->name = "Microsoft X-Box 360 pad";
			wv->input->phys = wv->phys;
			wv->input->dev.parent = &wv->intf->dev;
			wv->input->id.bustype = BUS_USB;
			wv->input->id.vendor = USB_VENDOR_ID_MICROSOFT;
			wv->input->id.product = USB_DEVICE_ID_XBOX360_CONTROLLER;
			wv->input->id.version = 0x0110;
			input_set_drvdata(wv->input, wv);
			wolverine_setup_input(wv);
		}
		
		dev_info(&wv->intf->dev, "Razer Wolverine V3 Pro controller disconnected\n");
	} else {
		/* Still receiving data - reschedule check */
		schedule_delayed_work(&wv->disconnect_work, WOLVERINE_DISCONNECT_TIMEOUT);
	}
	
	mutex_unlock(&wv->reg_lock);
}

/**
 * URB completion callback - processes incoming USB interrupt data
 */
static void wolverine_irq(struct urb *urb)
{
	struct wolverine_device *wv = urb->context;
	struct input_dev *input;
	unsigned char *data = wv->data;
	int status;

	switch (urb->status) {
	case 0:
		/* success */
		break;
	case -ECONNRESET:
	case -ENOENT:
	case -ESHUTDOWN:
		/* urb was unlinked/killed */
		return;
	default:
		printk(KERN_INFO "wolverine: URB error status=%d\n", urb->status);
		goto resubmit;
	}

	/* Process gamepad input packets (header: 00 14) */
	if (wv->data_size == 20 && data[0] == 0x00 && data[1] == 0x14) {
		/* Update last packet time (atomic, no lock needed) */
		wv->last_packet_time = jiffies;
		
		/* If controller wasn't connected, schedule registration work */
		if (!atomic_read(&wv->controller_connected)) {
			atomic_set(&wv->controller_connected, 1);
			schedule_work(&wv->connect_work);
		}
		
		/* Only process input if device is registered */
		if (!wv->input_registered)
			goto resubmit;
		
		input = wv->input;
		
		/* Special screenshot button uses multi-bit pattern in data[2]:
		 * 0x09 = Screenshot button (impossible D-pad combo)
		 * NOTE: SELECT is just bit 5 (0x20), Razer button is in byte 3 bit 2
		 */
		int screenshot_btn = (data[2] == 0x09);
		/* Byte 2 button mapping - VERIFIED through systematic testing */
		if (!screenshot_btn) {
			input_report_key(input, BTN_DPAD_UP, data[2] & 0x01);      /* D-pad UP */
			input_report_key(input, BTN_DPAD_DOWN, data[2] & 0x02);    /* D-pad DOWN */
			input_report_key(input, BTN_DPAD_LEFT, data[2] & 0x04);    /* D-pad LEFT */
			input_report_key(input, BTN_DPAD_RIGHT, data[2] & 0x08);   /* D-pad RIGHT */
			input_report_key(input, BTN_START, data[2] & 0x10);        /* START */
			input_report_key(input, BTN_SELECT, data[2] & 0x20);       /* SELECT */
			input_report_key(input, BTN_THUMBL, data[2] & 0x40);       /* L3 - Left stick click */
			input_report_key(input, BTN_THUMBR, data[2] & 0x80);       /* R3 - Right stick click */
		} else {
			/* Clear all byte2 buttons when screenshot button active */
			input_report_key(input, BTN_DPAD_UP, 0);
			input_report_key(input, BTN_DPAD_DOWN, 0);
			input_report_key(input, BTN_DPAD_LEFT, 0);
			input_report_key(input, BTN_DPAD_RIGHT, 0);
			input_report_key(input, BTN_START, 0);
			input_report_key(input, BTN_SELECT, 0);
			input_report_key(input, BTN_THUMBL, 0);
			input_report_key(input, BTN_THUMBR, 0);
		}
		
		/* Special screenshot button */
		input_report_key(input, BTN_TRIGGER_HAPPY1, screenshot_btn);  /* Screenshot */
		
		/* Byte 3 button mapping (verified via testing) */
		input_report_key(input, BTN_TL, data[3] & 0x01);     /* LB - Left Bumper */
		input_report_key(input, BTN_TR, data[3] & 0x02);     /* RB - Right Bumper */
		input_report_key(input, BTN_MODE, data[3] & 0x04);   /* Razer logo button */
		input_report_key(input, BTN_A, data[3] & 0x10);      /* A button */
		input_report_key(input, BTN_B, data[3] & 0x20);      /* B button */
		input_report_key(input, BTN_X, data[3] & 0x40);      /* X button */
		input_report_key(input, BTN_Y, data[3] & 0x80);      /* Y button */
		
		/* Analog sticks - bytes 6-9 contain stick data (16-bit little-endian) */
		/* Y-axis is inverted - negate to match expected orientation */
		input_report_abs(input, ABS_X, (s16)(data[6] | (data[7] << 8)));        /* Left stick X */
		input_report_abs(input, ABS_Y, -(s16)(data[8] | (data[9] << 8)));       /* Left stick Y - inverted */
		
		/* Right stick - bytes 10-13 */
		input_report_abs(input, ABS_RX, (s16)(data[10] | (data[11] << 8)));     /* Right stick X */
		input_report_abs(input, ABS_RY, -(s16)(data[12] | (data[13] << 8)));    /* Right stick Y - inverted */
		
		input_report_abs(input, ABS_Z, data[4]);   /* Left trigger */
		input_report_abs(input, ABS_RZ, data[5]);  /* Right trigger */
		
		input_sync(input);
	}

resubmit:
	status = usb_submit_urb(urb, GFP_ATOMIC);
	if (status)
		dev_err(&wv->usbdev->dev, "Failed to resubmit URB: %d\n", status);
}

/**
 * Setup input device with all button and axis capabilities
 */
static int wolverine_setup_input(struct wolverine_device *wv)
{
	struct input_dev *input = wv->input;
	
	/* Standard gamepad buttons - ORDER MATTERS for /dev/input/js0 mapping!
	 * This order matches Xbox controllers so Steam recognizes buttons correctly:
	 * js0: 0=A, 1=B, 2=X, 3=Y, 4=LB, 5=RB, 6=SELECT, 7=START, 8=L3, 9=R3, 10=MODE
	 */
	input_set_capability(input, EV_KEY, BTN_A);        /* js0 button 0 */
	input_set_capability(input, EV_KEY, BTN_B);        /* js0 button 1 */
	input_set_capability(input, EV_KEY, BTN_X);        /* js0 button 2 */
	input_set_capability(input, EV_KEY, BTN_Y);        /* js0 button 3 */
	input_set_capability(input, EV_KEY, BTN_TL);       /* js0 button 4 - LB */
	input_set_capability(input, EV_KEY, BTN_TR);       /* js0 button 5 - RB */
	input_set_capability(input, EV_KEY, BTN_SELECT);   /* js0 button 6 - View/Back */
	input_set_capability(input, EV_KEY, BTN_START);    /* js0 button 7 - Menu/Start */
	input_set_capability(input, EV_KEY, BTN_THUMBL);   /* js0 button 8 - L3 */
	input_set_capability(input, EV_KEY, BTN_THUMBR);   /* js0 button 9 - R3 */
	input_set_capability(input, EV_KEY, BTN_MODE);     /* js0 button 10 - Razer logo */
	input_set_capability(input, EV_KEY, BTN_TL2);      /* js0 button 11 - LT digital */
	input_set_capability(input, EV_KEY, BTN_TR2);      /* js0 button 12 - RT digital */
	
	/* D-pad */
	input_set_capability(input, EV_KEY, BTN_DPAD_UP);
	input_set_capability(input, EV_KEY, BTN_DPAD_DOWN);
	input_set_capability(input, EV_KEY, BTN_DPAD_LEFT);
	input_set_capability(input, EV_KEY, BTN_DPAD_RIGHT);
	
	/* Razer-specific buttons */
	input_set_capability(input, EV_KEY, BTN_TRIGGER_HAPPY1);  /* Screenshot */
	input_set_capability(input, EV_KEY, BTN_TRIGGER_HAPPY2);  /* M button */
	
	/* Programmable M1-M6 buttons (future support) */
	input_set_capability(input, EV_KEY, BTN_TRIGGER_HAPPY3);  /* M1 */
	input_set_capability(input, EV_KEY, BTN_TRIGGER_HAPPY4);  /* M2 */
	input_set_capability(input, EV_KEY, BTN_TRIGGER_HAPPY5);  /* M3 */
	input_set_capability(input, EV_KEY, BTN_TRIGGER_HAPPY6);  /* M4 */
	input_set_capability(input, EV_KEY, BTN_TRIGGER_HAPPY7);  /* M5 */
	input_set_capability(input, EV_KEY, BTN_TRIGGER_HAPPY8);  /* M6 */
	
	/* Analog axes */
	input_set_abs_params(input, ABS_X, -32768, 32767, 16, 128);   /* Left stick X */
	input_set_abs_params(input, ABS_Y, -32768, 32767, 16, 128);   /* Left stick Y */
	input_set_abs_params(input, ABS_RX, -32768, 32767, 16, 128);  /* Right stick X */
	input_set_abs_params(input, ABS_RY, -32768, 32767, 16, 128);  /* Right stick Y */
    input_set_abs_params(input, ABS_Z, 0, 255, 0, 0);   /* Left trigger */
    input_set_abs_params(input, ABS_RZ, 0, 255, 0, 0);  /* Right trigger */
	input_set_abs_params(input, ABS_Z, 0, 255, 0, 0);             /* Left trigger */
	return 0;
}

/**
 * USB probe - called when device is connected
 */
static int wolverine_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	struct usb_device *usbdev = interface_to_usbdev(intf);
	struct wolverine_device *wv;
	struct input_dev *input;
	struct usb_endpoint_descriptor *ep_in;
	int error;

	/* Only handle interface 0 (vendor-specific interface with gamepad data) */
	if (intf->cur_altsetting->desc.bInterfaceNumber != 0)
		return -ENODEV;

	dev_info(&intf->dev, "Razer Wolverine V3 Pro connecting via razerwolverine driver\n");

	/* Find the interrupt IN endpoint (0x81) */
	ep_in = NULL;
	for (int i = 0; i < intf->cur_altsetting->desc.bNumEndpoints; i++) {
		struct usb_endpoint_descriptor *ep = &intf->cur_altsetting->endpoint[i].desc;
		if (usb_endpoint_is_int_in(ep)) {
			ep_in = ep;
			break;
		}
	}
	
	if (!ep_in) {
		dev_err(&intf->dev, "Could not find interrupt IN endpoint\n");
		return -ENODEV;
	}

	/* Allocate device structure */
	wv = kzalloc(sizeof(struct wolverine_device), GFP_KERNEL);
	if (!wv)
		return -ENOMEM;

	/* Allocate input device */
	input = input_allocate_device();
	if (!input) {
		error = -ENOMEM;
		goto err_free_dev;
	}

	wv->usbdev = usbdev;
	wv->input = input;
	wv->intf = intf;
	
	/* Initialize connection tracking */
	mutex_init(&wv->reg_lock);
	atomic_set(&wv->controller_connected, 0);
	wv->input_registered = false;
	wv->shutting_down = false;
	wv->last_packet_time = 0;
	INIT_WORK(&wv->connect_work, wolverine_connect_work);
	INIT_DELAYED_WORK(&wv->disconnect_work, wolverine_disconnect_work);

	/* Setup input device - use Xbox 360 name for Steam compatibility */
	input->name = "Microsoft X-Box 360 pad";
	input->phys = wv->phys;
	usb_make_path(usbdev, wv->phys, sizeof(wv->phys));
	strlcat(wv->phys, "/input0", sizeof(wv->phys));

	input->dev.parent = &intf->dev;
	input->id.bustype = BUS_USB;
	/* Report as Xbox 360 controller for Steam/game compatibility.
	 * This makes Steam treat it as a native Xbox controller with
	 * automatic button mapping and game compatibility detection. */
	input->id.vendor = USB_VENDOR_ID_MICROSOFT;
	input->id.product = USB_DEVICE_ID_XBOX360_CONTROLLER;
	input->id.version = 0x0110;  /* Xbox 360 controller version */

	input_set_drvdata(input, wv);

	/* Configure input capabilities */
	error = wolverine_setup_input(wv);
	if (error)
		goto err_free_input;

	/* Allocate URB for interrupt transfers */
	wv->irq = usb_alloc_urb(0, GFP_KERNEL);
	if (!wv->irq) {
		error = -ENOMEM;
		goto err_free_input;
	}

	/* Allocate DMA buffer for input data */
	wv->data_size = 20;
	wv->data = usb_alloc_coherent(usbdev, wv->data_size, GFP_KERNEL, &wv->data_dma);
	if (!wv->data) {
		error = -ENOMEM;
		goto err_free_urb;
	}

	/* Initialize the URB */
	usb_fill_int_urb(wv->irq, usbdev,
			 usb_rcvintpipe(usbdev, ep_in->bEndpointAddress),
			 wv->data, wv->data_size,
			 wolverine_irq, wv, ep_in->bInterval);
	wv->irq->transfer_dma = wv->data_dma;
	wv->irq->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

	/* For wired mode (0x0A57), register input device immediately since
	 * the controller is always connected when the cable is plugged in.
	 * For wireless mode (0x0A59), defer registration until we receive data. */
	if (id->idProduct == USB_DEVICE_ID_RAZER_WOLVERINE_V3_PRO_WIRED) {
		error = input_register_device(input);
		if (error)
			goto err_free_dma;
		wv->input_registered = true;
		atomic_set(&wv->controller_connected, 1);
	}

	/* Submit URB to start receiving data */
	error = usb_submit_urb(wv->irq, GFP_KERNEL);
	if (error) {
		dev_err(&intf->dev, "Failed to submit URB: %d\n", error);
		goto err_unregister_input;
	}

	usb_set_intfdata(intf, wv);
	
	if (id->idProduct == USB_DEVICE_ID_RAZER_WOLVERINE_V3_PRO_WIRED) {
		dev_info(&intf->dev, "Razer Wolverine V3 Pro (wired) connected\n");
	} else {
		dev_info(&intf->dev, "Razer Wolverine V3 Pro dongle ready, waiting for controller\n");
	}
	
	return 0;

err_unregister_input:
	if (wv->input_registered) {
		input_unregister_device(input);
		input = NULL;
	}
err_free_dma:
	usb_free_coherent(usbdev, wv->data_size, wv->data, wv->data_dma);
err_free_urb:
	usb_free_urb(wv->irq);
err_free_input:
	input_free_device(input);
err_free_dev:
	kfree(wv);
	return error;
}

/**
 * USB disconnect - called when device is removed
 */
static void wolverine_disconnect(struct usb_interface *intf)
{
	struct wolverine_device *wv = usb_get_intfdata(intf);

	usb_set_intfdata(intf, NULL);
	if (wv) {
		/* Signal shutdown to prevent work items from doing anything */
		wv->shutting_down = true;
		
		/* Cancel pending work items */
		cancel_work_sync(&wv->connect_work);
		cancel_delayed_work_sync(&wv->disconnect_work);
		
		usb_kill_urb(wv->irq);
		
		/* Only unregister if it was registered */
		if (wv->input_registered)
			input_unregister_device(wv->input);
		else
			input_free_device(wv->input);
		
		usb_free_urb(wv->irq);
		usb_free_coherent(wv->usbdev, wv->data_size, wv->data, wv->data_dma);
		mutex_destroy(&wv->reg_lock);
		kfree(wv);
	}
	dev_info(&intf->dev, "Razer Wolverine V3 Pro dongle disconnected\n");
}

/**
 * USB driver structure
 */
static struct usb_driver wolverine_driver = {
	.name       = "razerwolverine",
	.probe      = wolverine_probe,
	.disconnect = wolverine_disconnect,
	.id_table   = wolverine_table,
	.supports_autosuspend = 0,
	.disable_hub_initiated_lpm = 1,
};

module_usb_driver(wolverine_driver);
