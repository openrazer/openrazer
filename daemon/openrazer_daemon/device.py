"""
Class to hold a device and collections of them
"""


class Device(object):
    """
    Razer Device (High level not dbus)
    """

    def __init__(self, device_id, device_serial, device_dbus_object):
        self._parent = None

        self._id = device_id
        self._serial = device_serial
        self._dbus = device_dbus_object
        # Register as parent
        self._dbus.register_parent(self)

    @property
    def device_id(self):
        """
        Device's USB ID String

        :return: Device ID
        :rtype: str
        """
        return self._id

    @property
    def serial(self):
        """
        Device's Serial String

        :return: Serial
        :rtype: str
        """
        return self._serial

    @property
    def dbus(self):
        """
        Device's DBus object

        :return: DBus Object
        :rtype: openrazer_daemon.hardware.device_base.__RazerDevice
        """
        return self._dbus

    def register_parent(self, parent):
        """
        Register the parent as an observer to be optionally notified (sends to other devices)

        :param parent: Observer
        :type parent: object
        """
        self._parent = parent

    def notify_parent(self, msg):
        """
        Notify observers with msg

        :param msg: Tuple with first element a string
        :type msg: tuple
        """
        self._parent.notify(self, msg)

    def notify_child(self, msg):
        """
        Receive observer messages

        :param msg: Tuple with first element a string
        :type msg: tuple
        """
        # Message from DBus object
        self._dbus.notify(msg)


class DeviceCollection(object):
    """
    Multimap of devices

    Can be referenced by either ID or serial
    """

    def __init__(self):
        self._id_map = {}
        self._serial_map = {}

    def add(self, device_id, device_serial, device_dbus):
        """
        Add device to collection

        :param device_id: Device's USB ID
        :type device_id: str

        :param device_serial: Device's Serial String
        :type device_serial: str

        :param device_dbus: Device's DBus object
        :type device_dbus: openrazer_daemon.hardware.device_base.__RazerDevice
        """
        device_object = Device(device_id, device_serial, device_dbus)
        device_object.register_parent(self)

        self._id_map[device_id] = device_object
        self._serial_map[device_serial] = device_object

    def remove(self, key):
        """
        Remove object being referenced to by ident from collection

        :param key: ID or serial
        :type key: str
        """
        self.__delitem__(key)

    def get(self, item):
        """
        Get device object by ID or serial

        :param item: ID or serial
        :type item: str

        :return: Device object
        :rtype: Device

        :raises IndexError: If item not found
        """
        return self.__getitem__(item)

    def id_items(self):
        """
        Get (id, Device) iterator

        :return: Items method from the id map
        :rtype: list of tuple of str, Device
        """
        return self._id_map.items()

    def serial_items(self):
        """
        Get (serial, Device) iterator

        :return: Items method from the serial map
        :rtype: list of tuple of str, Device
        """
        return self._serial_map.items()

    def serials(self):
        """
        Get list of serials

        :return: Serial list
        :rtype: list of str
        """
        return list(self._serial_map.keys())

    def __len__(self):
        """
        Get length of collection

        :return: Length
        :rtype: int
        """
        return len(self._id_map)

    def __getitem__(self, item):
        """
        Get device object by ID or serial

        :param item: ID or serial
        :type item: str

        :return: Device object
        :rtype: Device

        :raises IndexError: If item not found
        """
        if item in self._id_map:
            return self._id_map[item]
        elif item in self._serial_map:
            return self._serial_map[item]
        else:
            raise IndexError()

    def __delitem__(self, key):
        """
        Remove object being referenced to by ident from collection

        :param key: ID or serial
        :type key: str
        """
        if key in self._id_map:
            serial = self._id_map[key].serial
            self._id_map.pop(key, None)
            self._serial_map.pop(serial, None)
        elif key in self._serial_map:
            device_id = self._serial_map[key].device_id
            self._id_map.pop(device_id, None)
            self._serial_map.pop(key, None)

    def __contains__(self, item):
        """
        If ID or serial exists in datastructure

        :param item: ID or serial
        :type item: str

        :return: True if ID or serial exists
        :rtype: bool
        """
        return item in self._id_map or item in self._serial_map

    def __iter__(self):
        """
        Get devices

        :return: Devices
        :rtype: list of Device
        """
        return iter(self._id_map.values())

    @property
    def devices(self):
        """
        Get device list

        :return: List of devices
        :rtype: list of Device
        """
        return list(self._id_map.values())

    def notify(self, active_child, msg):
        """
        Send messages between children

        :param active_child: Child sending the message
        :type active_child: Device

        :param msg: Messgae
        :type msg: tuple
        """
        for child in self._id_map.values():
            if child is not active_child:
                child.notify_child(msg)
