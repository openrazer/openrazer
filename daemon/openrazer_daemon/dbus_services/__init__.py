"""
DBus module

Has an endpoint decorator to wrap a method for DBus
"""
from functools import wraps


def endpoint(interface_name, function_name, in_sig=None, out_sig=None, byte_arrays=False):
    """
    DBus Endpoint

    :param interface_name: DBus Interface name
    :type interface_name: str

    :param function_name: DBus Method name
    :type function_name: str

    :param in_sig: DBus parameter signature
    :type in_sig: str

    :param out_sig: DBus return signature
    :type out_sig: str

    :param byte_arrays: is Byte Array
    :type byte_arrays: bool

    :return: Function
    :rtype: callable
    """
    # pylint: disable=missing-docstring
    def inner_render(func):
        def wrapped(*args, **kwargs):
            return func(*args, **kwargs)
        wrapped.endpoint = True
        wrapped.interface = interface_name
        wrapped.name = function_name
        wrapped.in_sig = in_sig
        wrapped.out_sig = out_sig
        wrapped.byte_arrays = byte_arrays
        wrapped.code = func.__code__
        wrapped.globals = func.__globals__
        wrapped.defaults = func.__defaults__
        wrapped.closure = func.__closure__
        return wraps(func)(wrapped)
    return inner_render
