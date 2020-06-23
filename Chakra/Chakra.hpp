#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>

#ifdef CHAKRA_EXPORTS
#define CHAKRA_API __declspec(dllexport)
#else
#define CHAKRA_API __declspec(dllimport)
#endif

