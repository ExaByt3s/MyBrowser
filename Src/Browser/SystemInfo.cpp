
#include "SystemInfo.h"
#include <windows.h>

// 去掉字符串中的空格
void Trims(char* data)
{
	int i = -1, j = 0;
	int ch = ' ';
	while (data[++i] != '\0') {
		if (data[i] != ch) {
			data[j++] = data[i];
		}
	}
	data[j] = '\0';
}

int GetSystemInfo(SystemInfo &info)
{
	HRESULT hres;
	memset(&info, 0x00, sizeof(SystemInfo));

	CoUninitialize();

	// 第二个参数设置当前线程的并发模式为多线程
	hres = CoInitializeEx(0, COINIT_MULTITHREADED);

	if (FAILED(hres)) { return -1; }
	hres = CoInitializeSecurity(
		NULL,
		-1,
		NULL,
		NULL,
		RPC_C_AUTHN_LEVEL_DEFAULT,
		RPC_C_IMP_LEVEL_IMPERSONATE,
		NULL,
		EOAC_NONE,
		NULL);

	if (FAILED(hres)) {
		CoUninitialize();
		return -2;
	}
	IWbemLocator *pLoc = NULL;
	hres = CoCreateInstance(
		CLSID_WbemLocator,
		0,
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, (LPVOID *)&pLoc);
	if (FAILED(hres)) {
		CoUninitialize();
		return -3;
	}
	IWbemServices *pSvc = NULL;
	hres = pLoc->ConnectServer(
		_bstr_t(L"ROOT\\CIMV2"),
		NULL,
		NULL,
		0,
		NULL,
		0,
		0,
		&pSvc);
	if (FAILED(hres)) {
		pLoc->Release();
		CoUninitialize();
		return -4;
	}
	hres = CoSetProxyBlanket(
		pSvc,
		RPC_C_AUTHN_WINNT,
		RPC_C_AUTHZ_NONE,
		NULL,
		RPC_C_AUTHN_LEVEL_CALL,
		RPC_C_IMP_LEVEL_IMPERSONATE,
		NULL,
		EOAC_NONE
	);
	if (FAILED(hres)) {
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return -5;
	}

	IEnumWbemClassObject* pEnumerator = NULL;
	IWbemClassObject *pclsObj;
	ULONG uReturn = 0;

	// 获取主板ID
	hres = pSvc->ExecQuery(
		bstr_t("WQL"),
		bstr_t("SELECT * FROM Win32_BaseBoard"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL,
		&pEnumerator);
	if (FAILED(hres)) {
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return -7;
	}
	while (pEnumerator) {
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
		if (0 == uReturn) { break; }
		VARIANT vtProp;
		hr = pclsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0);
		wcstombs(info.BaseBoardID, vtProp.bstrVal, 20);
		VariantClear(&vtProp);
		pclsObj->Release();
	}


	// 获取BIOS序列号
	pEnumerator->Release();
	pEnumerator = NULL;
	hres = pSvc->ExecQuery(
		bstr_t("WQL"),
		bstr_t("SELECT * FROM Win32_BIOS"),
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL,
		&pEnumerator);
	if (FAILED(hres)) {
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return -9;
	}
	while (pEnumerator) {
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
		if (0 == uReturn) { break; }
		VARIANT vtProp;
		hr = pclsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0);
		wcstombs(info.BIOSID, vtProp.bstrVal, 64);
		VariantClear(&vtProp);
		pclsObj->Release();
	}

	pSvc->Release();
	pLoc->Release();
	pEnumerator->Release();
	CoUninitialize();

	return 0;
}