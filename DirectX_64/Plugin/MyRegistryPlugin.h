#ifndef _MYREGISTRY_PLUGIN_H
#define _MYREGISTRY_PLUGIN_H

//-----------------------------------------------------------

#define WINVER 0x0500
#define _WIN32_WINNT 0x0500
#define _WIN32_WINDOWS 0x0410
#define _WIN32_IE 0x0700
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#include <atlbase.h>
#include <atlstr.h>

#define MY_EXPORT extern "C" __declspec(dllexport)

//-----------------------------------------------------------

#endif //_MYREGISTRY_PLUGIN_H
