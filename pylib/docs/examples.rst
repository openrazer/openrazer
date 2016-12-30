.. role:: pycode(code)
   :language: py


Pylib Examples
==============

Ok first things first. The `DeviceManager` is the start of any interaction with the `razer-daemon` or Razer device.

The DeviceManager class can be imported like :pycode:`from razer.client import DeviceManager`. During initialisation, the class connects to the daemon service over DBus and grabs a list of devices
serials. From the list of serials it calls further DBus methods to gather enough data about the devices the serials refer to and then creates objects that represent said devices.

Apart from allowing you to access Razer devices, the DeviceManager also has the current pylib version and daemon version exposed via properties incase
those values are useful (driver versions are available from within the device objects themselves).

The DeviceManager class can also stop the daemon, enable/disable the logic to "turn off" the devices whilst the screensaver is active, and enable/disable syncing effects across multiple Razer devices.

.. autoclass:: razer.client.DeviceManager
   :members:

Now for a few examples:

.. code-block:: python3

   >>> # Import the DeviceManager
   >>> from razer.client import DeviceManager

   >>> # Create an instance of DeviceManager
   >>> dev_manager = DeviceManager()

   >>> # Get pylib version
   >>> dev_manager.version
   '1.1.3'

   >>> # Get daemon version
   >>> dev_manager.daemon_version
   '1.1.3'

   >>> # List devices
   >>> dev_manager.devices
   [<RazerMouse PM1640H04410109>, <RazerDevice HN1644D04700103>]




l

l

l

l

l

l

l

l

l

l



.. autoclass:: razer.client.devices.RazerDevice
   :members:
