import types
import inspect


def get_attrs(obj):
    all_obj_attrs = [x for x in dir(obj) if not x.startswith('_')]
    props = []
    funcs = []
    fields = []

    for obj_attr in all_obj_attrs:
        objs_to_check = list(obj.__class__.__mro__)
        objs_to_check.insert(0, obj)

        for obj_class in objs_to_check:
            try:
                attr = getattr(obj_class, obj_attr)

                if isinstance(attr, property):
                    get_sig = str(inspect.signature(attr.fget))
                    if '->' in get_sig:
                        get_sig = ':' + get_sig.split('-> ')[1]
                    else:
                        get_sig = ''

                    if attr.fset is not None:
                        set_sig = inspect.signature(attr.fset)
                        if len(set_sig.parameters.keys()) > 0:
                            set_type = str(set_sig.parameters[list(set_sig.parameters.keys())[-1]])
                        else:
                            set_type = ''

                        if ':' in set_type:
                            set_type = ':' + set_type.split(':')[1]
                        else:
                            set_type = ''
                        props.append(obj_attr + ' - get{0}, set{1}'.format(get_sig, set_type))
                    else:
                        props.append(obj_attr + ' - get{0}'.format(get_sig))
                    break
                elif isinstance(attr, types.FunctionType):
                    funcs.append(obj_attr + str(inspect.signature(attr)))
                    break
            except Exception as err:
                pass
        else:
            fields.append(obj_attr)

    return sorted(props), sorted(funcs), sorted(fields)


def print_attrs(obj, recurse_to=None, indent=0):
    if recurse_to is None:
        recurse_to = set()
    else:
        recurse_to = set(recurse_to)

    props, funcs, fields = get_attrs(obj)

    def print2(x): return print((" " * indent) + x)

    print(obj.__class__.__name__)
    if len(props) > 0:
        if len(funcs) + len(fields) == 0:
            print2(" └ properties:")
            pipe = ' '
        else:
            print2(" ├ properties:")
            pipe = '│'

        for index, prop in enumerate(props, start=1):
            if prop in recurse_to:
                if len(props) != index:
                    print((" " * indent) + ' {0}  ├- {1} - '.format(pipe, prop), end='')
                    print_attrs(getattr(obj, prop), recurse_to=(recurse_to - set(prop)), indent=indent + 7)
                else:
                    print((" " * indent) + ' {0}  └- {1} - '.format(pipe, prop), end='')
                    print_attrs(getattr(obj, prop), recurse_to=(recurse_to - set(prop)), indent=indent + 7)
            else:
                if len(props) != index:
                    print2(' {0}  ├- {1}'.format(pipe, prop))
                else:
                    print2(' {0}  └- {1}'.format(pipe, prop))
    if len(funcs) > 0:
        if len(fields) == 0:
            print2(" └ methods:")
            pipe = ' '
        else:
            print2(" ├ methods:")
            pipe = '│'

        for index, func in enumerate(funcs, start=1):
            if len(funcs) != index:
                print2(' {0}  ├- {1}'.format(pipe, func))
            else:
                print2(' {0}  └- {1}'.format(pipe, func))
    if len(fields) > 0:
        print2(" └ fields:")

        for index, field in enumerate(fields, start=1):
            if field in recurse_to:
                if len(fields) != index:
                    print((" " * indent) + '   ├- {0} - '.format(field), end='')
                    print_attrs(getattr(obj, field), recurse_to=(recurse_to - set(field)), indent=indent + 6)
                else:
                    print((" " * indent) + '   └- {0} - '.format(field), end='')
                    print_attrs(getattr(obj, field), recurse_to=(recurse_to - set(field)), indent=indent + 6)
            else:
                if len(fields) != index:
                    print2('   ├- {0}'.format(field))
                else:
                    print2('   └- {0}'.format(field))
