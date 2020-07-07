import cppyy
import cppyy.ll
import uuid
import re
from string import Template
from typing import Iterable, Union, Any
from copy import copy


class CPPTemplate(Template):
    delimiter = '%'


type_list = list()

cppyy.cppdef("""
#include <typeinfo>
template <typename T>
uint64_t type_hash()
{
    return typeid(T).hash_code();
}
""")


class namespace(object):
    current_namespace = 'gbl'

    def __init__(self, name="gbl"):
        assert re.fullmatch(r"^[a-zA-Z_$][a-zA-Z_$0-9]*$", name)
        self.prev_namespace = self.current_namespace
        self.__class__.current_namespace = name

    def __enter__(self):
        pass

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.__class__.current_namespace = self.prev_namespace

    @classmethod
    def current(cls):
        if cls.current_namespace == "gbl":
            return ""
        else:
            return cls.current_namespace

    @classmethod
    def __generate_random_identifier__(cls):
        return re.sub(r'\W|^(?=\d)', '_', "_" + str(uuid.uuid4()))

    @classmethod
    def cdef(cls, *ctypes):
        if cls.current() == 'gbl':
            ns = ""
        else:
            ns = cls.current()
        for ctype in ctypes:
            cppyy.cppdef(f"namespace {ns} {{ {ctype.cdef} }}")


class ctype_meta(type):
    __cpp_type_name__ = None
    __cpp_type_namespace__ = ""

    def __new__(mcs, name, bases, attrs):
        return type.__new__(mcs, name, bases, attrs)

    def __init__(cls, name, bases, attrs):
        type_list.append(cls)
        type.__init__(cls, name, bases, attrs)
        if not cls.__name__.endswith('base') and getattr(cls, "__cpp_type_name__") is None:
            raise NotImplementedError("__cpp_type_name__")

    def __getitem__(cls, size: Union[int, tuple]):

        return ccarray.initializer(cls)[size]

    def __eq__(cls, other):
        return isinstance(other, ctype) and cls.__cpp_type_name__ == other.__cpp_type_name__ and \
               cls.__cpp_type_namespace__ == other.__cpp_type_namespace__

    def __repr__(cls):
        return f"<ctype: '{cls.__cpp_type_name__}'>"

    __str__ = __repr__


class simple_ctype_meta(ctype_meta):
    def __new__(mcs, name, bases, attrs):
        if not mcs.__name__.endswith("base"):
            attrs["__cpp_type_name__"] = name[1:]
            attrs["__cpp_type_namespace__"] = ""
        return type.__new__(mcs, name, bases, attrs)


class ctype(object):
    __cpp_type_name__ = None

    def __init__(self, *args, **kwargs):
        self.__cpp_identifier__ = self.__generate_random_identifier__()

    @property
    def __cpp_type_hash__(self):
        return cppyy.gbl.type_hash[self.__cpp_type_name__]()

    def __str__(self):
        return f"<'{self.__cpp_type_name__}' object>"

    __repr__ = __str__

    @staticmethod
    def __generate_random_identifier__():
        return re.sub(r'\W|^(?=\d)', '_', "_" + str(uuid.uuid4()))

    @property
    def cdef(self):
        raise NotImplementedError()

    # @classmethod
    # def __getitem__(self, sz):
    #     pass


class ccarray(ctype, metaclass=ctype_meta):
    __cpp_type_name__ = "carray"

    class initializer:
        def __init__(self, ctype):
            self._ctype = ctype
            self._size = tuple()

        def __getitem__(self, item):
            if not isinstance(item, tuple):
                item = (item,)
            assert all(e > 0 for e in item)
            self._size += item

            return self

        @property
        def nelem(self):
            if len(self._size) == 0:
                return 0
            else:
                sz = 1
                for s in self._size:
                    sz *= s
                return sz

        def __call__(self, *args, **kwargs):
            return ccarray(self._ctype, size=self._size, args_init=[args] * self.nelem,
                           kwargs_init=[kwargs] * self.nelem)

    def __init__(self, array_ctype, size: Union[int, tuple],
                 args_init: Iterable = None, kwargs_init: Union[Iterable, dict] = None):
        super(ccarray, self).__init__()
        if size is not None:
            size = size if isinstance(size, tuple) else (size, )
            assert all(e > 0 for e in size)
        if issubclass(array_ctype, ctype):
            assert size is not None
            self._cont_type = array_ctype
            self._size = size
            if isinstance(args_init, Iterable):
                args_init = list(args_init)
                assert self.nelem == len(args_init)
                assert all(isinstance(e, tuple) for e in args_init)
            elif args_init is None:
                args_init = [tuple()] * self.nelem
            else:
                raise ValueError()

            if isinstance(kwargs_init, Iterable):
                kwargs_init = list(kwargs_init)
                assert self.nelem == len(kwargs_init)
                assert all(isinstance(e, dict) for e in kwargs_init)
            elif kwargs_init is None:
                kwargs_init = [dict()] * self.nelem
            else:
                raise ValueError()

            self._args_init = args_init
            self._kwargs_init = kwargs_init
        else:
            raise ValueError()

    def __getitem__(self, item):  # TODO: verify array of array
        if not isinstance(item, tuple):
            item = (item, )
        if len(item) != len(self._size):
            raise IndexError("dimension doesn't match")
        if item >= self._size:
            raise IndexError("out of range")

        return  # TODO: return somthing

    def __str__(self):
        return f"<ctype: 'carray{self._size} of type {str(self._cont_type)}'>"

    @property
    def nelem(self):
        return self._nelem(self._size)

    @staticmethod
    def _nelem(size):
        if len(size) == 0:
            return 0
        else:
            sz = 1
            for s in size:
                sz *= s
            return sz

    def resize(self, new_size: Union[int, tuple]):
        if not isinstance(new_size, tuple):
            new_size = (new_size, )

        assert self._nelem(new_size) == self.nelem
        new_self = copy(self)
        new_self._size = new_size
        return new_self


class ctype_base(ctype, metaclass=simple_ctype_meta):
    pass


class simple_ctype_base(ctype, metaclass=simple_ctype_meta):
    @property
    def cdef(self):
        return f"{self.__cpp_type_name__}"  # TODO


class cuint64_t(simple_ctype_base):
    def __init__(self, value=None):
        super(cuint64_t, self).__init__()
        self.value = value if value is not None else 0


class cuint32_t(simple_ctype_base):
    pass


class cuint16_t(simple_ctype_base):
    pass


class cuint8_t(simple_ctype_base):
    pass


class cint64_t(simple_ctype_base):
    pass


class cint32_t(simple_ctype_base):
    pass


class cint16_t(simple_ctype_base):
    pass


class cint8_t(simple_ctype_base):
    pass


class cdouble(simple_ctype_base):
    pass


class cfloat(simple_ctype_base):
    pass


if __name__ == '__main__':
    u64 = cuint64_t()
    print(cuint64_t)
    print(cuint64_t[1][2]())
    print(u64)
    print(type_list)
