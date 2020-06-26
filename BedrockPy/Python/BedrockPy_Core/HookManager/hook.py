import cppyy
import cppyy.ll
from ctypes import CFUNCTYPE
from string import Template
from parse import parse
import re
import uuid
from typing import Iterable, Optional, Callable



cppyy.cppdef("""
#include <Python.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>
namespace py = pybind11;
using namespace py::literals;
using namespace std;
using hash_func_t = uint64_t (*)();
""")


class CPPCallback(object):
    class CPPTemplate(Template):
        delimiter = '%'

    _cpp_arg_prefix = "arg_"

    _cpp_wrapper_headers = """
        #include <Python.h>
        #include <pybind11/pybind11.h>
        #include <pybind11/embed.h>
        #include <pybind11/operators.h>
        #include <pybind11/stl.h>
        #include <pybind11/functional.h>
        #include <unordered_map>
        #include <vector>
        #include <string>
        namespace py = pybind11;
        using namespace py::literals;
        using namespace std;
    """

    _cpp_wrapper_signature = """
    %{ret_type} %{name}(%{arg_type_with_name})
    """

    _cpp_wrapper_preamble = """
    {
        py
    """

    _type_cast_table = {
        "float": ""
    }

    _valid_c_type_hashes = {
        "int",
        "string"
    }

    def __init__(self, py_callback: Callable, ret_type: str, *arg_type: Optional[str]):
        self.ret_type = ret_type
        self.arg_type = arg_type
        self.arg_name = (self._cpp_arg_prefix + str(i) for i in range(len(arg_type)))
        self.c_wrapper_name = self._generate_random_identifier()
        self.callback = py_callback

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

    def _generate_full_arg_sigature(self):
        return ", ".join(t + " " + n for t, n in zip(self.arg_type, self.arg_name))

    @staticmethod
    def _get_type_hash(type_str):
        cppyy.cppdef("auto string_id2 = [](){return typeid(std::string).hash_code();};")

    def __str__(self):
        return f"CPPCallback(\"{self.ret_type} {self.c_wrapper_name}({self._generate_full_arg_sigature()})\")"

    __repr__ = __str__


class CPPGlobalDict:



warp = CPPCallback.decorator


if __name__ == '__main__':
    cppyy.add_include_path(r"E:\BedrockPy-Ng\Dist\BedrockPy\include")
    @warp("void(int, char*, bool)")
    def func(a, b, c):
        pass

    print(func)
