//
// Created by allan on 2020/6/24.
//
#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <unordered_map>
#include <tuple>
#include <algorithm>

#include <Python.h>

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

using FunctionRAVList   = std::vector<DWORD>;
using FunctionSymList   = std::vector<std::string>;
using FunctionDecList   = std::vector<std::string>;
using FunctionMeta      = std::tuple<FunctionRAVList, FunctionSymList, FunctionDecList>;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

__declspec(dllimport) void RegisterStaticHook(unsigned int sym, void *hook, void **org);
__declspec(dllimport) FunctionMeta &extract_symbols(const std::string &pdb_path);

#ifdef __cplusplus
}
#endif // __cplusplus