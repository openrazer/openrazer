/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef __HID_RAZER_KBD_TURRET_H
#define __HID_RAZER_KBD_TURRET_H

#include <linux/hid.h>

/* Razer Turret receiver, called by razerkbd for every receiver interface */
int razer_turret_receiver_probe(struct hid_device *hdev);
void razer_turret_receiver_remove(struct hid_device *hdev);

#endif
