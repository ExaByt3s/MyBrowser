// PUID.h: interface for the CPUID class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PUID_H__27C18CFB_98A1_45A9_B7E2_F4839F47CA6B__INCLUDED_)
#define AFX_PUID_H__27C18CFB_98A1_45A9_B7E2_F4839F47CA6B__INCLUDED_
typedef unsigned long DWORD;
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "StdString.h"

struct SerialNumber
{
    DWORD nibble[6];
};

class CPUID
{
public:
    CPUID();
    virtual ~CPUID();
    CStdString GetCpuSerialNumber();
    CStdString GetCpuVID();

	// 获取CPU序列号
	CStdString GetCpuSn();
	// 获取硬盘序列号
	CStdString getHDSerialNum();
	// 获取硬盘型号
	CStdString getHDModelNum();
	// 获取路由Mac
	CStdString getRouteMac();
	// 获取本机MAC
	CStdString getMAC();
	// 获取网卡MAC(与小房子一致取法)
	CStdString GetMacInfo();

private:
	// 用来实现cpuid
    void Executecpuid(DWORD eax); 
	CStdString ParseData();

    DWORD m_eax;   // 存储返回的eax
    DWORD m_ebx;   // 存储返回的ebx
    DWORD m_ecx;   // 存储返回的ecx
    DWORD m_edx;   // 存储返回的edx
};

#endif // !defined(AFX_PUID_H__27C18CFB_98A1_45A9_B7E2_F4839F47CA6B__INCLUDED_)
