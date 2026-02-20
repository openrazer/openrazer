from openrazer.client import DeviceManager

# Create a DeviceManager. This is used to get specific devices
device_manager = DeviceManager()


print("Found {} Razer devices".format(len(device_manager.devices)))
print()


# Iterate over each device, find the keyboards and apply the macro
for device in device_manager.devices:

    if device.type == "keyboard":

        # Hypershift macros (Fn + key)
        if device.macro:
            try:
                # Example spawning a shell if GENOME available
                macro = device.macro.create_script_macro_item("gnome-terminal", "-- python3")
                device.macro.add_macro("P", macro, hypershift=True)
                print(f"Bound FN+P to open Python terminal")
            except Exception as e:
                print(f"Error binding FN+P: {e}")
