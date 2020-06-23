// BedrockPy.h : Header file for your target.

#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include<iostream>
#include<filesystem>
#include <string>

#include <Python.h>
#include <frameobject.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/operators.h>

#include "../version.hpp"
