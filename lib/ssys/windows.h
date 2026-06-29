//
//
//
#pragma once

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <winsock2.h>
#undef RGB

typedef short int16_t;
typedef int int32_t;
typedef unsigned char u_char;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long u_long;

#endif
