import cppyy
import cppyy.ll
from ctypes import CFUNCTYPE
from string import Template
from parse import parse
import re
import uuid
from typing import Iterable, Optional, Callable

cppyy.ll.set_signals_as_exception(True)

cppyy.cppdef("""
class CPPCallback{
public:
CPPCallback() {}
virtual ~CPPCallback() {}
    void *m_py_func_pointer = nullptr;
    void *ori_func_pointer = nullptr;
};

""")

cppyy.cppdef("""
template<typename func_t>
inline uint64_t get_func_addr(const func_t *f)
{
    return reinterpret_cast<uint64_t>(f);
}


""")


class CPPCallback(cppyy.gbl.CPPCallback):
    callback_table = []

    def __init__(self, py_callback: Callable, ret_type: str, *arg_type: Optional[str]):
        super(CPPCallback, self).__init__()
        self.ret_type = ret_type
        self.arg_type = arg_type
        self.c_wrapper_name = self._generate_random_identifier()
        self.callback = py_callback
        self.callback_table.append(self)
        self.m_py_func_pointer = cppyy.ll.reinterpret_cast["void*"](self.cppaddr)

    @staticmethod
    def callback(*arg, **kwargs):
        raise NotImplementedError()

    @classmethod
    def decorator(cls, func_type: str):
        parsed = cls._parse_function_type(func_type)
        ret_type = parsed["ret_type"]
        arg_type = parsed["arg_type"]

        def _decorator(func):
            return CPPCallback(func, ret_type, *arg_type)

        return _decorator

    @staticmethod
    def _generate_random_identifier():
        return re.sub(r'\W|^(?=\d)', '_', "_" + str(uuid.uuid4()))

    @staticmethod
    def _parse_function_type(type_string):
        def arg_parser(text):
            return tuple(t.strip() for t in text.split(','))

        arg_parser.pattern = r".*"
        parsed = parse("{ret_type}({arg_type:arg})", type_string, dict(arg=arg_parser))
        return parsed.named if parsed is not None else None

    @property
    def cpptype(self):
        return f"{self.ret_type}({', '.join(self.arg_type)})"

    @property
    def cpppointertype(self):
        return f"{self.ret_type}(*)({', '.join(self.arg_type)})"

    @property
    def cppaddr(self):
        return cppyy.gbl.get_func_addr[self.cpptype](self.callback)

    def __str__(self):
        return f"CPPCallback(\"{self.ret_type} {self.c_wrapper_name}({', '.join(self.arg_type)})\")"

    __repr__ = __str__


warp = CPPCallback.decorator


class CPPTemplate(Template):
    delimiter = '%'


cppyy.ll.set_signals_as_exception(True)

"int(int, inti)"
hook_wrapper_templete = CPPTemplate("""
%{PY_CALLBACK_SIGNATURE}
{
    

}
""")

if __name__ == '__main__':
    cppyy.add_include_path(".")
    @warp("void(int, char*, bool)")
    def func(a, b, c):
        print((a,b,c))

    print(func.cppaddr)
    print(func.cppaddr)
    print(cppyy.ll.reinterpret_cast["uint64_t"](func.m_py_func_pointer))
