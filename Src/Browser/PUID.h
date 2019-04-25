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

	// ��ȡCPU���к�
	CStdString GetCpuSn();
	// ��ȡӲ�����к�
	CStdString getHDSerialNum();
	// ��ȡӲ���ͺ�
	CStdString getHDModelNum();
	// ��ȡ·��Mac
	CStdString getRouteMac();
	// ��ȡ����MAC
	CStdString getMAC();
	// ��ȡ����MAC(��С����һ��ȡ��)
	CStdString GetMacInfo();

private:
	// ����ʵ��cpuid
    void Executecpuid(DWORD eax); 
	CStdString ParseData();

    DWORD m_eax;   // �洢���ص�eax
    DWORD m_ebx;   // �洢���ص�ebx
    DWORD m_ecx;   // �洢���ص�ecx
    DWORD m_edx;   // �洢���ص�edx
};

#endif // !defined(AFX_PUID_H__27C18CFB_98A1_45A9_B7E2_F4839F47CA6B__INCLUDED_)
