#include "stdafx.h"
#include "ClientAppRenderer.h"
#include "PerfTest.h"
#include <algorithm>
#include <string>
#include "include/wrapper/cef_stream_resource_handler.h"

#include "StdString.h"
#include "EncryptByMd5.h"
#include "Globals.h"

namespace Browser
{
	namespace PerfTest
	{
		const char arrGetMachineCode[] = "getMachineCode";
		const char arrGetMachineInfo[] = "getMachineInfo";
		const char arrShowLoginDlg[] = "showLoginDlg";

		class V8Handler_ShowLoginDlg : public CefV8Handler
		{
		public:
			V8Handler_ShowLoginDlg() {}

			virtual bool Execute(const CefString& name,
				CefRefPtr<CefV8Value> object,
				const CefV8ValueList& arguments,
				CefRefPtr<CefV8Value>& retval,
				CefString& exception) OVERRIDE
			{
				if (name == arrShowLoginDlg) {
					CefRefPtr<CefBrowser> browser =	CefV8Context::GetCurrentContext()->GetBrowser();
					CefRefPtr<CefProcessMessage> message = CefProcessMessage::Create("showLoginDlg");
					browser->SendProcessMessage(PID_BROWSER, message);
				}

				return true;
			}
		private:
			IMPLEMENT_REFCOUNTING(V8Handler_ShowLoginDlg);
		};

		class V8Handler_GetMachineCode : public CefV8Handler
		{
		public:
			V8Handler_GetMachineCode() {}

			virtual bool Execute(const CefString& name,
				CefRefPtr<CefV8Value> object,
				const CefV8ValueList& arguments,
				CefRefPtr<CefV8Value>& retval,
				CefString& exception) OVERRIDE
			{
				if (name == arrGetMachineCode) {
					char* strMachineCodeAnsi = CEncryptByMd5::Unicode2ANSI(GLOBALS::strMachineCode);
					retval = CefV8Value::CreateString(strMachineCodeAnsi);
					delete strMachineCodeAnsi;
				}

				return true;
			}
		private:
			IMPLEMENT_REFCOUNTING(V8Handler_GetMachineCode);
		};

		class V8Handler_GetMachineInfo : public CefV8Handler
		{
		public:
			V8Handler_GetMachineInfo() {}

			virtual bool Execute(const CefString& name,
				CefRefPtr<CefV8Value> object,
				const CefV8ValueList& arguments,
				CefRefPtr<CefV8Value>& retval,
				CefString& exception) OVERRIDE
			{
				if (name == arrGetMachineInfo) {
					CStdString seperator = _T("@");
					CStdString strMachineCode = GLOBALS::strMachineCode;
					CStdString strMachineInfo = strMachineCode.append(seperator)
						.append(GLOBALS::strBiosID).append(seperator)
						.append(GLOBALS::strCpuSn).append(seperator)
						.append(GLOBALS::strHdSn).append(seperator)
						.append(GLOBALS::strHdMod).append(seperator)
						.append(GLOBALS::strLocalMac).append(seperator)
						.append(GLOBALS::strRouteMac);
					char* strMachineInfoAnsi = CEncryptByMd5::Unicode2ANSI(strMachineInfo);
					retval = CefV8Value::CreateString(strMachineInfoAnsi);
					delete strMachineInfoAnsi;
				}

				return true;
			}
		private:
			IMPLEMENT_REFCOUNTING(V8Handler_GetMachineInfo);
		};

		// Handle bindings in the render process.
		class RenderDelegate : public ClientAppRenderer::Delegate
		{
		public:
			RenderDelegate() {}

			virtual void OnContextCreated(CefRefPtr<ClientAppRenderer> app,
				CefRefPtr<CefBrowser> browser,
				CefRefPtr<CefFrame> frame,
				CefRefPtr<CefV8Context> context) OVERRIDE {
					CefRefPtr<CefV8Value> object = context->GetGlobal();

					CefRefPtr<CefV8Handler> handlerGetMachineCode = new V8Handler_GetMachineCode();
					CefRefPtr<CefV8Handler> handlerGetMachineInfo = new V8Handler_GetMachineInfo();
					CefRefPtr<CefV8Handler> handlerShowLoginDlg = new V8Handler_ShowLoginDlg();

					// Bind functions.
					object->SetValue(arrGetMachineCode,
						CefV8Value::CreateFunction(arrGetMachineCode, handlerGetMachineCode),
						V8_PROPERTY_ATTRIBUTE_READONLY);

					// Bind functions.
					object->SetValue(arrGetMachineInfo,
						CefV8Value::CreateFunction(arrGetMachineInfo, handlerGetMachineInfo),
						V8_PROPERTY_ATTRIBUTE_READONLY);

					// Bind functions.
					object->SetValue(arrShowLoginDlg,
						CefV8Value::CreateFunction(arrShowLoginDlg, handlerShowLoginDlg),
						V8_PROPERTY_ATTRIBUTE_READONLY);
			}

		private:
			IMPLEMENT_REFCOUNTING(RenderDelegate);
		};

		void CreateDelegates(ClientAppRenderer::DelegateSet& delegates) {
			delegates.insert(new RenderDelegate);
		}
	}
}
