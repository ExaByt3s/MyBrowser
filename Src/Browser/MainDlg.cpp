#include "stdafx.h"
#include "include/base/cef_scoped_ptr.h"
#include "include/cef_command_line.h"
#include "include/cef_sandbox_win.h"
#include "ClientAppBrowser.h"
#include "ClientAppRenderer.h"
#include "MessageLoop.h"
#include "MainContext.h"
#include "BrowserDlgManager.h"
#include "ClientRunner.h"
#include <windows.h>
#include <objbase.h>

#include "SystemInfo.h"
#include "PUID.h"
#include "Globals.h"
#include "EncryptByMd5.h"
#include "EnDeCode.h"

namespace Browser
{
	// 计算机器码
	CStdString CalcMachineCode(
		CStdString version,
		CStdString machineCodeMode) {
		if (version.CompareNoCase(_T("500")) == 0) {
			// 3.3
			CStdString strHdInfo(_T(""));
			strHdInfo.append(GLOBALS::strHdSn);
			if (GLOBALS::strHdMod.GetLength() > 4) {
				strHdInfo.append(GLOBALS::strHdMod.Left(3));
			}

			CStdString strCpuSnHdInfo(_T(""));
			strCpuSnHdInfo.Format(_T("@%s@%s"),
				GLOBALS::strCpuSn.GetBuffer(),
				strHdInfo.GetBuffer());

			CStdString strMd5Part1 = CEncryptByMd5::Digest(strCpuSnHdInfo);
			CStdString strMd5Part2 = CEncryptByMd5::Digest(GLOBALS::strRouteMac);
			return strMd5Part1.append(strMd5Part2);
		}
		else if (version.CompareNoCase(_T("400")) == 0) {
			// 3.2
			if (machineCodeMode.CompareNoCase(_T("1")) == 0) {
				// 长机器码模式
				CStdString strHdInfo(_T(""));
				strHdInfo.append(GLOBALS::strHdSn);
				if (GLOBALS::strHdMod.GetLength() > 4) {
					strHdInfo.append(GLOBALS::strHdMod.Left(3));
				}

				CStdString strMachinePlainCode(_T(""));
				strMachinePlainCode.Format(_T("@%s%s"),
					strHdInfo.GetBuffer(),
					GLOBALS::strRouteMac.GetBuffer());

				return CEncryptByMd5::Digest(strMachinePlainCode);
			}
			else {
				// 短机器码模式
				CStdString strHdInfo(_T(""));
				strHdInfo.append(GLOBALS::strHdSn);
				if (GLOBALS::strHdMod.GetLength() > 4) {
					strHdInfo.append(GLOBALS::strHdMod.Left(3));
				}

				CStdString strMachinePlainCode(_T(""));
				strMachinePlainCode.Format(_T("@%s"),
					strHdInfo.GetBuffer());

				return CEncryptByMd5::Digest(strMachinePlainCode);
			}
		}
	}

	// Encode机器码
	CStdString EncodeMachineCode(
		CStdString version,
		CStdString machineCodeMode) {
		if (version.CompareNoCase(_T("500")) == 0) {
			// 3.3
			CStdString strHdInfo(_T(""));
			strHdInfo.append(GLOBALS::strHdSn);
			if (GLOBALS::strHdMod.GetLength() > 4) {
				strHdInfo.append(GLOBALS::strHdMod.Left(3));
			}

			CStdString strCpuSnHdInfo(_T(""));
			strCpuSnHdInfo.Format(_T("@%s@%s"),
				GLOBALS::strCpuSn.GetBuffer(),
				strHdInfo.GetBuffer());

			CStdString strMd5Part1 = CEncryptByMd5::Digest(strCpuSnHdInfo);
			CStdString strMd5Part2 = CEncryptByMd5::Digest(GLOBALS::strRouteMac);
			return strMd5Part1.append(strMd5Part2);
		}
		else if (version.CompareNoCase(_T("400")) == 0) {
			// 3.2
			if (machineCodeMode.CompareNoCase(_T("1")) == 0) {
				// 长机器码模式
				CStdString strHdInfo(_T(""));
				strHdInfo.append(GLOBALS::strHdSn);
				if (GLOBALS::strHdMod.GetLength() > 4) {
					strHdInfo.append(GLOBALS::strHdMod.Left(3));
				}

				CStdString strMachinePlainCode(_T(""));
				strMachinePlainCode.Format(_T("@%s%s"),
					strHdInfo.GetBuffer(),
					GLOBALS::strRouteMac.GetBuffer());

				char *cTemp = CEncryptByMd5::Unicode2ANSI(strMachinePlainCode);

				CStdString strMachineCodeEncoded = CEnDeCode::Encode(cTemp);
				delete cTemp;

				return strMachineCodeEncoded;
			}
			else {
				// 短机器码模式
				CStdString strHdInfo(_T(""));
				strHdInfo.append(GLOBALS::strHdSn);
				if (GLOBALS::strHdMod.GetLength() > 4) {
					strHdInfo.append(GLOBALS::strHdMod.Left(3));
				}

				CStdString strMachinePlainCode(_T(""));
				strMachinePlainCode.Format(_T("@%s"),
					strHdInfo.GetBuffer());

				char *cTemp = CEncryptByMd5::Unicode2ANSI(strMachinePlainCode);

				CStdString strMachineCodeEncoded = CEnDeCode::Encode(cTemp);
				delete cTemp;

				return strMachineCodeEncoded;
			}
		}
	}

	int RunMain(HINSTANCE hInstance, int nCmdShow, const CefString& url, bool bSingleInstance)
	{
		int result = 0;
		HRESULT hr = ::CoInitialize(NULL);
		if (FAILED(hr)) { return 0; }

		DuiLib::CPaintManagerUI::SetInstance(hInstance);
		// DuiLib::CPaintManagerUI::SetResourceType(DuiLib::UILIB_ZIPRESOURCE);
		DuiLib::CPaintManagerUI::SetResourceType(DuiLib::UILIB_FILE);
		DuiLib::CPaintManagerUI::SetResourcePath(DuiLib::CPaintManagerUI::GetInstancePath());
		// DuiLib::CPaintManagerUI::SetResourceZip(MAKEINTRESOURCE(IDR_ZIPRES), _T("ZIPRES"));

		CefMainArgs main_args(hInstance);
		CefSettings settings;
		void* sandbox_info = NULL;
#ifdef CEF_USE_SANDBOX
		CefScopedSandboxInfo scoped_sandbox;
		sandbox_info = scoped_sandbox.sandbox_info();
#else
		settings.no_sandbox = true;
#endif
		// Parse command-line arguments.
		CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
		command_line->InitFromString(::GetCommandLineW());

		CefRefPtr<CefApp> app;
		ClientApp::ProcessType process_type = ClientApp::GetProcessType(command_line);
		if (process_type == ClientApp::BrowserProcess) {
			if(bSingleInstance) {
				::CreateMutex(NULL, TRUE, _T("MyCefBrowser"));
				if (GetLastError() == ERROR_ALREADY_EXISTS)
				{
					// AfxMessageBox("您已经运行了本软件！");
					return FALSE;
				}
			}
			app = new ClientAppBrowser();
		}
		else if (process_type == ClientApp::RendererProcess) {
			app = new ClientAppRenderer();
		}
		else if (process_type == ClientApp::OtherProcess) {
			app = new ClientAppOther();
		}

		// Execute the secondary process, if any.
		int exit_code = CefExecuteProcess(main_args, app, sandbox_info);
		if (exit_code >= 0) {
			return exit_code;
		}

		// Create the main context object.
		scoped_ptr<MainContext> context(new MainContext(command_line, true));

		// Populate the settings based on command line arguments.
		context->PopulateSettings(&settings);

		WCHAR szBuffer[MAX_PATH];
		::ZeroMemory(&szBuffer, sizeof(szBuffer));
		GetTempPathW(MAX_PATH,szBuffer);
		std::wstring sBuffer = szBuffer;
		sBuffer += L"Browser";
		CefString(&settings.cache_path).FromWString(sBuffer);

		settings.ignore_certificate_errors = true;

		// settings.command_line_args_disabled = true;
		// settings.locale = "zh-CN";
		CefString(&settings.locale).FromASCII("zh-CN");
		// const char* loc = "zh-CN";
		// cef_string_from_ascii(loc, strlen(loc), &settings.locale);

#ifndef _DEBUG
		settings.log_severity = LOGSEVERITY_DISABLE;
#endif
		// settings.multi_threaded_message_loop = true;
		// settings.single_process = true;
		

		// Initialize CEF.
		context->Initialize(main_args, settings, app, sandbox_info);

		// Register scheme handlers.
		// ClientRunner::RegisterSchemeHandlers();

		// Create the first window.
		BrowserDlg* pDlg = context->GetBrowserDlgManager()->CreateBrowserDlg(
			NULL,
			true,		// Show controls.
			CefRect(),	// Use default system size.
			url);		// Use default URL.

		if(pDlg) {
			pDlg->CenterWindow();
		}

		// DuiLib::CPaintManagerUI::MessageLoop();
		CefRunMessageLoop();

		DuiLib::CPaintManagerUI::Term();

		// Shut down CEF.
		context->Shutdown();

		::CoUninitialize();
		return result;
	}

	int GetConfigValue() 
	{
		// 获得BIOS序列号(主机出厂编号)
		SystemInfo info;
		GetSystemInfo(info);
		GLOBALS::strBiosID = info.BIOSID;
		CPUID hdInfoTool;
		// 获取CPU序列号
		GLOBALS::strCpuSn = hdInfoTool.GetCpuSn();
		// 获取硬盘序列号
		GLOBALS::strHdSn = hdInfoTool.getHDSerialNum();
		// 获取硬盘型号
		GLOBALS::strHdMod = hdInfoTool.getHDModelNum();
		// 获取路由Mac
		GLOBALS::strRouteMac = hdInfoTool.getRouteMac();
		// 获取本机Mac
		GLOBALS::strLocalMac = hdInfoTool.GetMacInfo();

		TCHAR szFilePath[MAX_PATH];
		GetModuleFileName(NULL, szFilePath, _MAX_PATH);
		CStdString strRunDir = szFilePath;
		int nPos = strRunDir.ReverseFind('\\');
		if (nPos != -1) { strRunDir = strRunDir.Left(nPos + 1); }

		CStdString strIniFilePath = CStdString(strRunDir).append(_T("MyBrowser.ini"));
		CStdString strAutoUpdateCfgIniFilePath = CStdString(strRunDir).append(_T("UpdateConfig.ini"));

		// 从INI文件中读取配置：登录url
		CStdString strLogInUrl;
		::GetPrivateProfileString(
			_T("Settings"),
			_T("LoginUrl"),
			_T(""),
			strLogInUrl.GetBuffer(MAX_PATH),
			MAX_PATH,
			strIniFilePath);
		GLOBALS::strLogInUrl = strLogInUrl;

		// 从INI文件中读取配置：登出url
		CStdString strLogOutUrl;
		::GetPrivateProfileString(
			_T("Settings"),
			_T("LogoutUrl"),
			_T(""),
			strLogOutUrl.GetBuffer(MAX_PATH),
			MAX_PATH,
			strIniFilePath);
		GLOBALS::strLogOutUrl = strLogOutUrl;

		// 从INI文件中读取配置：邮箱url
		CStdString strMailUrl;
		::GetPrivateProfileString(
			_T("Settings"),
			_T("MailUrl"),
			_T(""),
			strMailUrl.GetBuffer(MAX_PATH),
			MAX_PATH,
			strIniFilePath);
		GLOBALS::strMailUrl = strMailUrl;

		// 从INI文件中读取配置：公司名称
		CStdString strCompanyName;
		::GetPrivateProfileString(
			_T("Settings"),
			_T("CompanyName"),
			_T(""),
			strCompanyName.GetBuffer(MAX_PATH),
			MAX_PATH,
			strIniFilePath);
		GLOBALS::strCompanyName = strCompanyName;

		// 从INI文件中读取配置：用途说明
		CStdString strUsageTips;
		::GetPrivateProfileString(
			_T("Settings"),
			_T("ReleaseVersion"),
			_T(""),
			strUsageTips.GetBuffer(MAX_PATH),
			MAX_PATH,
			strIniFilePath);
		GLOBALS::strUsageTips = strUsageTips;

		// 从在线更新INI文件中读取配置：主版本号
		CStdString strMajorVersion;
		::GetPrivateProfileString(
			_T("UPDATE"),
			_T("Version"),
			_T("3.3.0"),
			strMajorVersion.GetBuffer(MAX_PATH),
			MAX_PATH,
			strAutoUpdateCfgIniFilePath);
		GLOBALS::strMajorVersion = strMajorVersion;

		// 从INI文件中读取配置：版本号
		CStdString strVersion;
		::GetPrivateProfileString(
			_T("Settings"),
			_T("Version"),
			_T("500"),
			strVersion.GetBuffer(MAX_PATH),
			MAX_PATH,
			strIniFilePath);
		GLOBALS::strVersion = strVersion;

		// 从INI文件中读取配置：是否显示地址栏
		CStdString strShowAddressBar;
		::GetPrivateProfileString(
			_T("Settings"),
			_T("ShowAddressBar"),
			_T("0"),
			strShowAddressBar.GetBuffer(MAX_PATH),
			MAX_PATH,
			strIniFilePath);
		GLOBALS::strShowAddressBar = strShowAddressBar;

		// 从INI文件中读取配置：是否显示工具栏
		CStdString strShowToolBar;
		::GetPrivateProfileString(
			_T("Settings"),
			_T("ShowToolBar"),
			_T("0"),
			strShowToolBar.GetBuffer(MAX_PATH),
			MAX_PATH,
			strIniFilePath);
		GLOBALS::strShowToolBar = strShowToolBar;

		// 从INI文件中读取配置：是否显示开发者工具
		CStdString strShowDevTools;
		::GetPrivateProfileString(
			_T("Settings"),
			_T("ShowDevTools"),
			_T("0"),
			strShowDevTools.GetBuffer(MAX_PATH),
			MAX_PATH,
			strIniFilePath);
		GLOBALS::strShowDevTools = strShowDevTools;

		// 从INI文件中读取配置：官网URL
		CStdString strHomeUrl;
		::GetPrivateProfileString(
			_T("Settings"),
			_T("HomeUrl"),
			_T("0"),
			strHomeUrl.GetBuffer(MAX_PATH),
			MAX_PATH,
			strIniFilePath);
		GLOBALS::strHomeUrl = strHomeUrl;

		// 从INI文件中读取配置：是否只允许一个实例运行
		CStdString strSingleInstance;
		::GetPrivateProfileString(
			_T("Settings"),
			_T("SingleInstance"),
			_T("1"),
			strSingleInstance.GetBuffer(MAX_PATH),
			MAX_PATH,
			strIniFilePath);
		GLOBALS::strSingleInstance = strSingleInstance;

		// 从INI文件中读取配置：机器码长短模式
		CStdString strMachineCodeMode;
		::GetPrivateProfileString(
			_T("Settings"),
			_T("MachineCodeMode"),
			_T("1"),
			strMachineCodeMode.GetBuffer(MAX_PATH),
			MAX_PATH,
			strIniFilePath);

		CStdString strMachineCodeEncoded = Browser::EncodeMachineCode(strVersion, strMachineCodeMode);
		GLOBALS::strMachineCodeEncoded = strMachineCodeEncoded;

		CStdString strMachineCode = Browser::CalcMachineCode(strVersion, strMachineCodeMode);
		GLOBALS::strMachineCode = strMachineCode;
	}
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
	Browser::GetConfigValue();

	bool bSingleInstance = false;
	if (Browser::GLOBALS::strSingleInstance.CompareNoCase(_T("1")) == 0) {
		bSingleInstance = true;
		// ======== cef主进程会接下来创建子进程，这段代码会进入多遍 ======== 
		// ======== 故在此处控制单例不合适 ========
	}

	return Browser::RunMain(hInstance, nCmdShow, Browser::GLOBALS::strHomeUrl, bSingleInstance);
}