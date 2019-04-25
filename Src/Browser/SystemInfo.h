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
	char BaseBoardID[256];         // ����ID
	char BIOSID[64];               // BIOS���к�
} SystemInfo;

int GetSystemInfo(SystemInfo &info);
#endif