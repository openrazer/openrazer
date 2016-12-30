.. role:: pycode(code)
   :language: py


RazerDevice Class
=================

I tried to make this class sorta generic so that if there wasnt a specific class for a device, it would still have some functionality.

All the device class does is just combine DBus endpoints into a *simple* python object.

You shouln't need to create a RazerDevice (or any subclasses of it) manually, really you should just use the device manager. If you really want to create it manually you
can just specify a serial and it will gather the needed data about the device via the daemon. You wont benifit from any subclasses that might be more specific for that device so
if you call :func:`razer.client.device.RazerDeviceFactory.get_device` that will select the right class for the job. For example the Razer Tartarus would be covered by the RazerKeyboard device class
which is more tailored to keyboard devices. A Razer Mamba would use a RazerMouse class which has DPI settings.

The Device class and its derivatives all operate on the notion of capabilities. When the RazerDevice class is initialised, it creates an internal dictionary called ``RazerDevice._capabilities`` which 
literally contains a massive set of keys with a boolean value. You could look into this dictionary to see if a specific feature is supported but I added a 
convenience method :func:`razer.client.devices.RazerDevice.has` which literally takes a string and returns True/False. 

**For developers** All of the features that have the possibility of not being preset are
stored in this dict, if you look at the source of the method/property that you are calling there will probably be a ``self.has`` line in there checking if the feature is available, I would advise 
checking before you call stuff as otherwise you could get ``None``'s returned or worse exceptions being thrown.

.. autoclass:: razer.client.devices.RazerDevice
   :members:

Example:

.. code-block:: python3

   >>> from razer.client import DeviceManager
   >>> dev_manager = DeviceManager()
   >>> device = dev_manager.devices[0]
   >>> device
   <RazerMouse PM9999Z99999999>

   >>> device.name
   'Razer DeathAdder Elite'

   >>> device.has('brightness')
   False
   >>> device.has('lighting_logo_brightness')
   True
   >>> device.has('lighting_scroll_brightness')
   True

   >>> # Device doesnt have 1 global brightness property but seperate brightness's for both the logo and the scroll wheel (some devices can control LEDs individually)
   >>> device.fx.misc.logo.brightness
   71.76
   >>> device.fx.misc.logo.brightness = 25.0
   >>> device.fx.misc.logo.brightness
   25.1
   >>> # Brightness takes a percentage 0.0->100.0, its stored into a single byte (0->255) so the returned value wont always be what was sent.

   >>> # Getting and setting DPI
   >>> device.has('dpi')
   True
   >>> device.dpi
   (800, 800)
   >>> device.dpi = (16000, 16000)
   >>> device.dpi
   (16000, 16000)

   >>> # Getting versions
   >>> device.firmware_version
   'v1.1'
   >>> device.driver_version
   '1.1.3'


RazerDeviceFactory Class
========================

I thought I'd document the device factory here. All it does is when given a serial, it will get some info about the device and select a more appropiate class for the device if one exists. There
is a dictionary which maps device types to classes, so mice and keyboards use their appropriate subclasses but if the device type doesn't exist it will fall back to RazerDevice.

.. autoclass:: razer.client.device.RazerDeviceFactory
   :members:
