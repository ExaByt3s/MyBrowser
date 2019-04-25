#ifndef __SHARE_MEMORY_H__
#define __SHARE_MEMORY_H__
#pragma once

#include "stdafx.h"
#include <Windows.h>
#include <iostream>

namespace Browser
{
	class CShareMemory
	{
	public:
		static int WriteShareMemory(char* strInBuff, int len) {
			char* lpBuffer = NULL;

			HANDLE lhShareMemory = CreateFileMappingA(
				HANDLE(0xFFFFFFFF),
				NULL,
				PAGE_READWRITE,
				0,
				10,
				"mySharedMemory");

			if (NULL == lhShareMemory) { return -1; }

			lpBuffer = (char*)MapViewOfFile(lhShareMemory, FILE_MAP_WRITE, 0, 0, 10);
			if (NULL == lpBuffer) { return -1; }
			strncpy(lpBuffer, strInBuff, len);
			UnmapViewOfFile(lpBuffer);
			return 0;
		}

		static int ReadShareMemory(char* strOutBuff, int len) {
			HANDLE lhShareMemory;
			char* lpcBuffer;

			lhShareMemory = OpenFileMappingA(FILE_MAP_READ, false, "mySharedMemory");
			if (NULL == lhShareMemory) { return -1; }

			lpcBuffer = (char*)MapViewOfFile(lhShareMemory, FILE_MAP_READ, 0, 0, 100);
			if (NULL == lpcBuffer) { return -1; }
			strncpy(strOutBuff, lpcBuffer, len);
			UnmapViewOfFile(lpcBuffer);

			return 0;
		}
	};
}
#endif