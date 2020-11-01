#!/usr/bin/python3
import openrazer.client

devmgr = openrazer.client.DeviceManager()

for d in devmgr.devices:
    # Sanity check matrix capabilities
    if d.has("lighting_led_matrix"):
        d.fx.advanced.matrix[0, 0] = [0, 255, 0]
        try:
            d.fx.advanced.draw()
        except Exception as e:
            print('\n~~~ ' + d.name + ' ~~~\n')
            print(e)
