from functools import wraps

def endpoint(interface_name, in_sig=None, out_sig=None, byte_arrays=False):
    """
    Used to define an endpoints interface and signature

    To be used on functions
    :param interface_name: Name of interface
    :type interface_name: str

    :param in_sig: Argument signature
    :type in_sig: str or None

    :param out_sig: Output signature
    :type out_sig: str or None

    :param byte_arrays: Byte array
    :type byte_arrays: bool

    :return: Returns a function
    :rtype: func
    """

    # Called with args
    def decorator(func):
        """
        Decorator Function
        """
        func.endpoint = True
        func.interface = interface_name
        func.in_sig = in_sig
        func.out_sig = out_sig
        func.byte_arrays = byte_arrays
        return func
    return decorator

def endpoint2(interface_name, in_sig=None, out_sig=None, byte_arrays=False):
    def inner_render(fn):
        def wrapped(*args, **kwargs):
            return fn(*args, **kwargs)
        wrapped.endpoint = True
        wrapped.interface = interface_name
        wrapped.in_sig = in_sig
        wrapped.out_sig = out_sig
        wrapped.byte_arrays = byte_arrays
        wrapped.code = fn.__code__
        wrapped.globals = fn.__globals__
        wrapped.defaults = fn.__defaults__
        wrapped.closure = fn.__closure__
        return wraps(fn)(wrapped)
    return inner_render