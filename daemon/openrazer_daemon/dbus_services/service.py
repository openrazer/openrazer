"""
Service Object for DBus
"""

# Disable some pylint stuff
# pylint: disable=no-member

import types
import dbus
import dbus.service


def copy_func(function_reference, name=None):
    """
    Copy function

    :param function_reference: Function
    :type function_reference: func

    :param name: Name of function
    :type name: str

    :return: Copy of function
    :rtype: func
    """
    if hasattr(function_reference, 'code'):
        return types.FunctionType(function_reference.code, function_reference.globals, name or function_reference.func_name, function_reference.defaults, function_reference.closure)
    else:
        return types.FunctionType(function_reference.__code__, function_reference.__globals__, name or function_reference.func_name, function_reference.__defaults__, function_reference.__closure__)


class DBusServiceFactory(object):
    """
    Factory object to create differnet service objects.

    Different service objects are useful, as the DBus service table is stored in the class which would be shared
    between all class instances.
    """
    service_number = 0

    @staticmethod
    def get_service(bus_name, object_path):
        """
        Get an instance of the service history

        :param bus_name: DBus bus name
        :type bus_name: str

        :param object_path: DBus Object name
        :type object_path: str

        :return: New object
        :rtype: DBusService
        """
        new_service = type("DBUSService{0:04}".format(DBusServiceFactory.service_number), (DBusService,), {})
        DBusServiceFactory.service_number += 1

        return new_service(bus_name, object_path)


class DBusService(dbus.service.Object):
    """
    DBus Service object

    Allows for dynamic method adding
    """
    BUS_TYPE = 'session'

    def __init__(self, bus_name, object_path):
        """
        Init the object

        :param bus_name: DBus bus name
        :type bus_name: str

        :param object_path: DBus Object name
        :type object_path: str
        """
        self.bus_name = bus_name
        self.object_path = object_path

        if DBusService.BUS_TYPE == 'session':
            bus_object = dbus.service.BusName(bus_name, bus=dbus.SessionBus())
        else:
            bus_object = dbus.service.BusName(bus_name, bus=dbus.SystemBus())

        super(DBusService, self).__init__(bus_object, object_path)

    def add_dbus_method(self, interface_name, function_name, function, in_signature=None, out_signature=None, byte_arrays=False):
        """
        Add method to DBus Object

        :param interface_name: DBus interface name
        :type interface_name: str

        :param function_name: DBus function name
        :type function_name: str

        :param function: Function reference
        :type function: object

        :param in_signature: DBus function signature
        :type in_signature: str

        :param out_signature: DBus function signature
        :type out_signature: str

        :param byte_arrays: Is byte array
        :type byte_arrays: bool
        """

        # Get class key for use in the DBus introspection table
        class_key = [key for key in self._dbus_class_table.keys() if key.endswith(self.__class__.__name__)][0]

        # Create a copy of the function so that if its used multiple times it wont affect other instances if the names changed
        function_deepcopy = copy_func(function, function_name)
        func = dbus.service.method(interface_name, in_signature=in_signature, out_signature=out_signature, byte_arrays=byte_arrays)(function_deepcopy)

        # Add method to DBus tables
        try:
            self._dbus_class_table[class_key][interface_name][function_name] = func
        except KeyError:
            self._dbus_class_table[class_key][interface_name] = {function_name: func}

        # Add method to class as DBus expects it to be there.
        setattr(self.__class__, function_name, func)

    def del_dbus_method(self, interface_name, function_name):
        """
        Remove method from DBus Object

        :param interface_name: DBus interface name
        :type interface_name: str

        :param function_name: DBus function name
        :type function_name: str
        """

        # Get class key for use in the DBus introspection table
        class_key = [key for key in self._dbus_class_table.keys() if key.endswith(self.__class__.__name__)][0]

        # Remove method from DBus tables
        # Remove method from class
        try:
            del self._dbus_class_table[class_key][interface_name][function_name]
            delattr(DBusService, function_name)

        except (KeyError, AttributeError):
            pass
