#ifndef __SYSTEM_INFO_H__
#define __SYSTEM_INFO_H__
#pragma once 

#include "stdafx.h"
#define _WIN32_DCOM
#include <comdef.h>
#include <Wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")
// using namespace std;

typedef struct SystemInfo_t
{
	char BaseBoardID[256];         // Ö÷°åID
	char BIOSID[64];               // BIOSÐòÁÐºÅ
} SystemInfo;

int GetSystemInfo(SystemInfo &info);
#endif