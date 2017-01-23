.. role:: pycode(code)
   :language: py


RazerFX Class
=============

The FX class contains all of the common hardware effects on devices. ## TODO logo and scroll effects are here SingleLEDCLASS ##.

This class has a method :func:`razer.client.fx.RazerFX.has` which is similar to the method in :class:`razer.client.devices.RazerDevice`, it still looks
through the capability dictionary but it prefex's ``"lighting_"`` onto the key so you can do ``device_obj.fx.has("static")`` which will look up ``"lighting_static"`` in the capability
dictionary. People should utilise has, as if the function is not available, it will return False always. Soon I am thinking to replace this with raising :class:`NotImplementedError`.

The ripple methods will soon be replaced and better custom effects will be supported.

.. autoclass:: razer.client.fx.RazerFX
   :show-inheritance:
   :inherited-members:
   :special-members: __init__
   :members:


RazerAdvancedFX Class
=====================

.. autoclass:: razer.client.fx.RazerAdvancedFX
   :show-inheritance:
   :inherited-members:
   :special-members: __init__
   :members: