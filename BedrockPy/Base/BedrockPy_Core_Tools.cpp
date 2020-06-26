#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"
//
// Created by allan on 2020/6/24.
//
#include <functional>

#include "BedrockPy_Core_Tools.hpp"
#include <unordered_map>
#include <tuple>
#include <typeinfo>

namespace py = pybind11;
using namespace py::literals;

void call_py_spf(std::function<void(*)(std::string*)> &f){f(&sss);}
std::string ssss("ssss");void call_py_spff(std::function<void(*)(std::string*)> f){f(&ssss);}

using addr_t = uint64_t;

template<typename T>uint64_t get_type_hash(){return typeid(T).hash_code();}
template<typename T>uint64_t address(T tt){return uint64_t(tt);}

int ret_int()
{return 1;}


struct HookBase {
    HookBase() = default;
    HookBase(const HookBase &Impl) : _target_func(Impl._target_func), _hook(Impl._hook) { }
    HookBase(addr_t target_func, addr_t hook) : _target_func(target_func), _hook(hook) { }
    void setTargetFunc(addr_t target_func) { _target_func = target_func; }
    addr_t &getTargetFunc() const { return const_cast<addr_t &>(static_cast<const addr_t &>(_target_func)); }
    addr_t _hook = addr_t(nullptr);
    addr_t _target_func = addr_t(nullptr);
    void func_arg(py::function py_func){
        auto pyo_func = py_func.ptr();
        std::cout << typeid(pyo_func).name() << std::endl;
    }
};


struct PyCallback {
    py::function callback;
    const type_info &cpp_int = typeid(int);

};


PYBIND11_EMBEDDED_MODULE(_BedrockPy_Core_Tools, m) {
// `m` is a `py::module` which is used to bind functions and classes
    m.def("RegisterStaticHook", [](addr_t sym, addr_t hook, addr_t org){
              RegisterStaticHook(sym, reinterpret_cast<void*>(hook), reinterpret_cast<void**>(org));
          }
    );
    m.def("extract_symbols", extract_symbols);
    m.def("new_uint64_array", [](unsigned int size){
        return reinterpret_cast<addr_t>(new uint64_t [size]);
    });
    m.def("free_uint64_array", [](addr_t address){
        delete [] reinterpret_cast<uint64_t*>(address);
    });
    m.def("new_double_array", [](unsigned int size){
        return reinterpret_cast<addr_t>(new double [size]);
    });
    m.def("free_double_array", [](addr_t address){
        delete [] reinterpret_cast<double*>(address);
    });
    py::class_<HookBase>(m, "HookBase", py::dynamic_attr())
            .def(py::init<>())
            .def(py::init<addr_t, addr_t>())
            .def(py::init<const HookBase &>())
            .def("setTargetFunc", &HookBase::setTargetFunc)
            .def("func_arg", &HookBase::func_arg)
            .def("getTargetFunc", &HookBase::getTargetFunc)
            .def_readwrite("_hook", &HookBase::_hook)
            .def_readwrite("_target_func", &HookBase::_target_func);
    m.def("call_cppyy_test", [](addr_t cppyy_func_addr, const std::string& what){
        auto cppyy_func = reinterpret_cast<void (*) (std::string)>(cppyy_func_addr);
        cppyy_func(what);
    });
}


#pragma clang diagnostic pop