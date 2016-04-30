from functools import wraps

def endpoint(interface_name, function_name, in_sig=None, out_sig=None, byte_arrays=False):
    def inner_render(fn):
        def wrapped(*args, **kwargs):
            return fn(*args, **kwargs)
        wrapped.endpoint = True
        wrapped.interface = interface_name
        wrapped.name = function_name
        wrapped.in_sig = in_sig
        wrapped.out_sig = out_sig
        wrapped.byte_arrays = byte_arrays
        wrapped.code = fn.__code__
        wrapped.globals = fn.__globals__
        wrapped.defaults = fn.__defaults__
        wrapped.closure = fn.__closure__
        return wraps(fn)(wrapped)
    return inner_render