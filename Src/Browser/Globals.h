#ifndef __GLOBALS_H__
#define __GLOBALS_H__
#pragma once

#include "StdString.h"

namespace Browser
{
	class GLOBALS
	{
	public:
		static CStdString strBiosID;
		static CStdString strCpuSn;
		static CStdString strHdSn;
		static CStdString strHdMod;
		static CStdString strRouteMac;
		static CStdString strLocalMac;
		static CStdString strHomeUrl;
		static CStdString strSingleInstance;
		static CStdString strLogInUrl;
		static CStdString strLogOutUrl;
		static CStdString strMailUrl;
		static CStdString strMachineCode;
		static CStdString strMachineCodeEncoded;		
		static CStdString strVersion;
		static CStdString strCompanyName;
		static CStdString strUsageTips;
		static CStdString strMajorVersion;
		static CStdString strShowAddressBar;
		static CStdString strShowToolBar;
		static CStdString strShowDevTools;
	};
}

#endif