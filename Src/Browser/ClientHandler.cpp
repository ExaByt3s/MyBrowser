#include "stdafx.h"
#include "ClientHandler.h"
#include <stdio.h>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <string>
#include "include/base/cef_bind.h"
#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "include/cef_parser.h"
#include "include/cef_command_line.h"
#include "include/wrapper/cef_closure_task.h"
#include "MainContext.h"
#include "ResourceUtil.h"
#include "BrowserDlgManager.h"
#include "ClientRunner.h"
#include "ClientSwitches.h"

#include "LoginDialog.h"
#include "Globals.h"

namespace Browser
{
	namespace
	{
		// Custom menu command Ids.
		enum client_menu_ids {
			CLIENT_ID_REFRESH = MENU_ID_USER_FIRST,
			CLIENT_ID_SHOW_DEVTOOLS,
			CLIENT_ID_CLOSE_DEVTOOLS,
			CLIENT_ID_LOGO,
			CLIENT_ID_BINDING,
		};

		std::string GetTimeString(const CefTime& value)
		{
			if (value.GetTimeT() == 0) {
				return "Unspecified";
			}

			static const char* arrMonths[] = {
				"January", "February", "March", "April", "May", "June", "July", "August",
				"September", "October", "November", "December"
			};
			std::string month;
			if (value.month >= 1 && value.month <= 12) {
				month = arrMonths[value.month - 1];
			}
			else {
				month = "Invalid";
			}

			std::stringstream ss;
			ss << month << " " << value.day_of_month << ", " << value.year << " " <<
				std::setfill('0') << std::setw(2) << value.hour << ":" <<
				std::setfill('0') << std::setw(2) << value.minute << ":" <<
				std::setfill('0') << std::setw(2) << value.second;
			return ss.str();
		}

		std::string GetBinaryString(CefRefPtr<CefBinaryValue> value)
		{
			if (!value.get()) {
				return "&nbsp;";
			}

			// Retrieve the value.
			const size_t size = value->GetSize();
			std::string src;
			src.resize(size);
			value->GetData(const_cast<char*>(src.data()), size, 0);

			// Encode the value.
			return CefBase64Encode(src.data(), src.size());
		}

		std::string GetCertStatusString(cef_cert_status_t status)
		{
            #define FLAG(flag) if (status & flag) result += std::string(#flag) + "<br/>" 
			std::string result;

			FLAG(CERT_STATUS_COMMON_NAME_INVALID);
			FLAG(CERT_STATUS_DATE_INVALID);
			FLAG(CERT_STATUS_AUTHORITY_INVALID);
			FLAG(CERT_STATUS_NO_REVOCATION_MECHANISM);
			FLAG(CERT_STATUS_UNABLE_TO_CHECK_REVOCATION);
			FLAG(CERT_STATUS_REVOKED);
			FLAG(CERT_STATUS_INVALID);
			FLAG(CERT_STATUS_WEAK_SIGNATURE_ALGORITHM);
			FLAG(CERT_STATUS_NON_UNIQUE_NAME);
			FLAG(CERT_STATUS_WEAK_KEY);
			FLAG(CERT_STATUS_PINNED_KEY_MISSING);
			FLAG(CERT_STATUS_NAME_CONSTRAINT_VIOLATION);
			FLAG(CERT_STATUS_VALIDITY_TOO_LONG);
			FLAG(CERT_STATUS_IS_EV);
			FLAG(CERT_STATUS_REV_CHECKING_ENABLED);
			FLAG(CERT_STATUS_SHA1_SIGNATURE_PRESENT);
			FLAG(CERT_STATUS_CT_COMPLIANCE_FAILED);

			if (result.empty()) {
				return "&nbsp;";
			}
			return result;
		}

		// Load a data: URI containing the error message.
		void LoadErrorPage(CefRefPtr<CefFrame> frame,
			const std::string& failed_url,
			cef_errorcode_t error_code,
			const std::string& other_info)
		{
			std::stringstream ss;
			ss << "<html><head><title>Page failed to load</title></head>"
				"<body bgcolor=\"white\">"
				"<h3>Page failed to load.</h3>"
				"URL: <a href=\"" << failed_url << "\">"<< failed_url << "</a>"
				"<br/>Error: " << ClientRunner::GetErrorString(error_code) <<
				" (" << error_code << ")";

			if (!other_info.empty()) {
				ss << "<br/>" << other_info;
			}

			ss << "</body></html>";
			frame->LoadURL(ClientRunner::GetDataURI(ss.str(), "text/html"));
		}
	}

	ClientHandler::ClientHandler(Delegate* delegate, const CefString& startup_url)
		: m_Delegate(delegate),
		m_nDefaultBrowserId(-1),
		m_sStartupUrl(startup_url),
		m_sConsoleLogFile(MainContext::Get()->GetConsoleLogPath()),
		m_bFirstConsoleMessage(true),
		m_bFocusOnEditableField(false)
	{
		DCHECK(!m_sConsoleLogFile.empty());

		m_ResourceManager = new CefResourceManager();
		ClientRunner::SetupResourceManager(m_ResourceManager);

		// Read command line settings.
		CefRefPtr<CefCommandLine> command_line = CefCommandLine::GetGlobalCommandLine();
		m_bMouseCursorChangeDisabled = command_line->HasSwitch(Switches::kMouseCursorChangeDisabled);
	}

	void ClientHandler::DetachDelegate()
	{
		if (!CefCurrentlyOn(TID_UI)) {
			// Execute this method on the main thread.
			CefPostTask(TID_UI, CefCreateClosureTask(base::Bind(&ClientHandler::DetachDelegate, this)));
			return;
		}

		DCHECK(m_Delegate);
		m_Delegate = NULL;
	}

	bool ClientHandler::OnProcessMessageReceived(
		CefRefPtr<CefBrowser> browser,
		CefProcessId source_process,
		CefRefPtr<CefProcessMessage> message)
	{
		CEF_REQUIRE_UI_THREAD();

		CefString strMsgName = message->GetName();
		if (strMsgName.compare("showLoginDlg") == 0) {
			CLoginDlg* loginDlg = new CLoginDlg();
			loginDlg->Create(NULL, _T(""), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
			loginDlg->CenterWindow();
			loginDlg->ShowModal();

			CStdString strUserName = loginDlg->m_strUserName;
			CStdString strPassword = loginDlg->m_strPassword;
			delete loginDlg;

			if (!strUserName.IsEmpty() && !strPassword.IsEmpty()) {
				CStdString strUrl(_T(""));
				strUrl.Format(
					_T("%s?username=%s&password=%s&key=%s&cpusn=%s&hdsn=%s&hdmodnum=%s&mac=%s&routemac=%s&bios=%s&version=%s&type=1"),
					GLOBALS::strLogInUrl.GetBuffer(),
					strUserName.GetBuffer(),
					strPassword.GetBuffer(),
					GLOBALS::strMachineCode.GetBuffer(),
					GLOBALS::strCpuSn.GetBuffer(),
					GLOBALS::strHdSn.GetBuffer(),
					GLOBALS::strHdMod.GetBuffer(),
					GLOBALS::strLocalMac.GetBuffer(),
					GLOBALS::strRouteMac.GetBuffer(),
					GLOBALS::strBiosID.GetBuffer(),
					GLOBALS::strVersion.CompareNoCase(_T("500")) == 0 ? _T("3.3") : _T("3.2"));

				browser->GetMainFrame()->LoadURL(strUrl.GetBuffer());
			}
		}

		if (m_MessageRouter->OnProcessMessageReceived(browser, source_process, message)) {
			return true;
		}

		return false;
	}

	void ClientHandler::OnBeforeContextMenu(
		CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefRefPtr<CefContextMenuParams> params,
		CefRefPtr<CefMenuModel> model)
	{
		CEF_REQUIRE_UI_THREAD();
		// ��������Ĳ˵���
		// model->Clear();

		// ɾ����ӡ�˵�
		model->Remove(MENU_ID_PRINT);						
		// ɾ���鿴Դ��˵�
		model->Remove(MENU_ID_VIEW_SOURCE);

		model->AddItem(CLIENT_ID_REFRESH, CefString(L"ˢ�µ�ǰҳ��"));
		if(GLOBALS::strShowDevTools.CompareNoCase(_T("1")) == 0) {
			model->AddItem(CLIENT_ID_SHOW_DEVTOOLS, CefString(L"��ʾ�����߹���"));
		    model->AddItem(CLIENT_ID_CLOSE_DEVTOOLS, CefString(L"�رտ����߹���"));
		}
		// model->AddItem(CLIENT_ID_LOGO, CefString(L"��ʾLOGO"));
		// model->AddItem(CLIENT_ID_BINDING, CefString(L"Binding����"));
	}

	bool ClientHandler::OnContextMenuCommand(
		CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefRefPtr<CefContextMenuParams> params,
		int command_id,
		EventFlags event_flags)
	{
		CEF_REQUIRE_UI_THREAD();
		switch (command_id)
		{
		case CLIENT_ID_REFRESH:
			browser->Reload();
			return true;
		case CLIENT_ID_SHOW_DEVTOOLS:
			ShowDevTools(browser, CefPoint());
			return true;
		case CLIENT_ID_CLOSE_DEVTOOLS:
			CloseDevTools(browser);
			return true;
		case CLIENT_ID_LOGO:
			frame->LoadURL("http://tests/logo.png");
			return true;
		case CLIENT_ID_BINDING:
			frame->LoadURL("http://tests/binding.html");
			return true;
		default:
			return false;
		}
	}

	void ClientHandler::OnAddressChange(
		CefRefPtr<CefBrowser> browser, 
		CefRefPtr<CefFrame> frame, 
		const CefString& url)
	{
		CEF_REQUIRE_UI_THREAD();
		if (frame->IsMain()){
			NotifyAddress(browser, url);
		}
	}

	void ClientHandler::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title)
	{
		CEF_REQUIRE_UI_THREAD();
		NotifyTitle(browser, title);
	}

	void ClientHandler::OnFaviconURLChange(
		CefRefPtr<CefBrowser> browser,
		const std::vector<CefString>& icon_urls)
	{
		CEF_REQUIRE_UI_THREAD();
	}

	void ClientHandler::OnFullscreenModeChange(CefRefPtr<CefBrowser> browser, bool fullscreen)
	{
		CEF_REQUIRE_UI_THREAD();
		NotifyFullscreen(browser, fullscreen);
	}

	bool ClientHandler::OnConsoleMessage(
		CefRefPtr<CefBrowser> browser, 
		cef_log_severity_t level,
		const CefString& message, 
		const CefString& source, 
		int line)
	{
		CEF_REQUIRE_UI_THREAD();

		// FILE* file = fopen(m_sConsoleLogFile.c_str(), "a");
		// if (file) {
		//	std::stringstream ss;
		//	ss << "Message: " << message.ToString() << "\r\n" <<
		//		"Source: " << source.ToString() << "\r\n" <<
		//		"Line: " << line << "\r\n" <<
		//		"-----------------------" << "\r\n";
		//	fputs(ss.str().c_str(), file);
		//	fclose(file);

		//	// if (m_bFirstConsoleMessage) {
		//	//	ClientRunner::Alert(browser,
		//	//		"Console messages written to \"" + m_sConsoleLogFile + "\"");
		//	//	m_bFirstConsoleMessage = false;
		//	// }
		// }

		return false;
	}

	bool ClientHandler::OnTooltip(CefRefPtr<CefBrowser> browser, CefString& text)
	{
		CEF_REQUIRE_UI_THREAD();

		// if (m_pEvent != NULL) {
		//	m_pEvent->OnTooltip(text);
		// }

		return false;
	}

	void ClientHandler::OnStatusMessage(CefRefPtr<CefBrowser> browser,
		const CefString& value)
	{
		CEF_REQUIRE_UI_THREAD();
	}

	// ��дjs alert ��
	bool ClientHandler::OnJSDialog(CefRefPtr<CefBrowser> browser,
		const CefString& origin_url,
		JSDialogType dialog_type,
		const CefString& message_text,
		const CefString& default_prompt_text,
		CefRefPtr<CefJSDialogCallback> callback,
		bool& suppress_message) {

		if (dialog_type == JSDIALOGTYPE_ALERT) {
			// ��ͨ��ʾ��
			MessageBox(NULL, CStdString(message_text.c_str()), _T("��ʾ"), MB_OK);
			suppress_message = true;
			return false;
		} else if (dialog_type == JSDIALOGTYPE_CONFIRM) {
			// ѯ�ʿ�
			int result = ::MessageBox(NULL, CStdString(message_text.c_str()), _T("��ȷ��"), MB_OKCANCEL);
			if(result == IDOK) {
				callback->Continue(true, "");
			} else {
				callback->Continue(false, "");
			}
			suppress_message = false;
			return true;
		} else if (dialog_type == JSDIALOGTYPE_PROMPT) {
			// ����򣬲�֧��
			MessageBox(NULL, _T("���������JS������ʱ��֧�֣�"), _T("��ʾ"), MB_OK);
			suppress_message = true;
			return false;
		}

		return false;
	}

	// ��д�뿪ҳ����ʾ��(��ǰҳ�������ѱ��޸ģ��뿪ǰ����ȷ�Ͽ�)
	bool ClientHandler::OnBeforeUnloadDialog(
		CefRefPtr<CefBrowser> browser,
		const CefString& message_text,
		bool is_reload,
		CefRefPtr<CefJSDialogCallback> callback) {
		
		CStdString msg = _T("�뿪����վ��\r\n\r\nϵͳ���ܲ��ᱣ���������ĸ��ġ�");
		int result = ::MessageBox(NULL, msg, _T("��ȷ��"), MB_OKCANCEL);
		if (result == IDOK) {
			callback->Continue(true, "");
		} else {
			callback->Continue(false, "");
		}

		return true;
	}

	// void ClientHandler::OnDialogClosed(CefRefPtr<CefBrowser> browser) {

	// }

	void ClientHandler::OnBeforeDownload(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefDownloadItem> download_item,
		const CefString& suggested_name,
		CefRefPtr<CefBeforeDownloadCallback> callback)
	{
		CEF_REQUIRE_UI_THREAD();
		// Continue the download and show the "Save As" dialog.
		callback->Continue(MainContext::Get()->GetDownloadPath(suggested_name), true);
	}

	void ClientHandler::OnDownloadUpdated(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefDownloadItem> download_item,
		CefRefPtr<CefDownloadItemCallback> callback)
	{
		CEF_REQUIRE_UI_THREAD();
		if (download_item->IsComplete())
		{
			ClientRunner::Alert(
				browser,
				CefString(_T("�ļ���")).ToString()
				    + download_item->GetFullPath().ToString() 
				    + CefString(_T("�� ������ɣ�")).ToString()
			);
		}
	}

	bool ClientHandler::OnDragEnter(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefDragData> dragData,
		CefDragHandler::DragOperationsMask mask)
	{
		CEF_REQUIRE_UI_THREAD();

		// Forbid dragging of link URLs.
		if (mask & DRAG_OPERATION_LINK) {
			if(dragData->IsLink()) {
				CefString linkUrl = dragData->GetLinkURL();
				if(!linkUrl.empty()) {
					if (m_Delegate) {
						m_Delegate->OnDragLinkDrop(browser, linkUrl);
					}
				}
				return true;
			}
		}

		// bool isFile = dragData->IsFile();
		// if (isFile) {
		//	std::vector<CefString> fileNames;
		//	dragData->GetFileNames(fileNames);
		//	// MessageBox(NULL, CStdString(fileNames.at(0).c_str()), _T("��Ϣ"), MB_OK);
		// }

		return false;
	}

	void ClientHandler::OnDraggableRegionsChanged(
		CefRefPtr<CefBrowser> browser,
		const std::vector<CefDraggableRegion>& regions)
	{
		CEF_REQUIRE_UI_THREAD();

		// NotifyDraggableRegions(regions);
	}

	// bool ClientHandler::OnRequestGeolocationPermission(CefRefPtr<CefBrowser> browser,
	//	const CefString& requesting_url,
	//	int request_id,
	//	CefRefPtr<CefGeolocationCallback> callback)
	// {
	//	CEF_REQUIRE_UI_THREAD();
	//	// ��������������վ�ĵ���λ�÷���.
	//	callback->Continue(true);
	//	return true;
	// }

	bool ClientHandler::OnPreKeyEvent(CefRefPtr<CefBrowser> browser,
		const CefKeyEvent& event,
		CefEventHandle os_event,
		bool* is_keyboard_shortcut)
	{
		if (!event.focus_on_editable_field && 
			event.windows_key_code == 0x20){
			if (event.type == KEYEVENT_RAWKEYDOWN){
				// browser->GetMainFrame()->ExecuteJavaScript("alert('You pressed the space bar!');", "", 0);
			}
			return true;
		} else if (!event.focus_on_editable_field && 
			event.windows_key_code == 0x74) {
			// F5ˢ��
			if (event.type == KEYEVENT_RAWKEYDOWN) {
				browser->Reload();
			}
			return true;
		} else if (!event.focus_on_editable_field && 
			event.windows_key_code == 0x77) {
			// F8(CTRL+F5��ʱ�޷�ʵ��)�޻���ˢ��
			if (event.type == KEYEVENT_RAWKEYDOWN) {
				browser->ReloadIgnoreCache();
			}
			return true;
		} else if (!event.focus_on_editable_field && 
			event.windows_key_code == 0x7b) {
			// ���������߹���
			if (event.type == KEYEVENT_RAWKEYDOWN) {
				if (GLOBALS::strShowDevTools.CompareNoCase(_T("1")) == 0) {
					ShowDevTools(browser, CefPoint());
				}
			}
			return true;
		}

		return false;
	}

	bool ClientHandler::OnBeforePopup(
		CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		const CefString& target_url,
		const CefString& target_frame_name,
		cef_window_open_disposition_t target_disposition,
		bool user_gesture,
		const CefPopupFeatures& popupFeatures,
		CefWindowInfo& windowInfo,
		CefRefPtr<CefClient>& client,
		CefBrowserSettings& settings,
		bool* no_javascript_access)
	{
		CEF_REQUIRE_IO_THREAD();
		int nBrowserId = browser->GetIdentifier();

		// Return true to cancel the popup window.
		if(target_disposition == WOD_NEW_POPUP){
			return !CreatePopupWindow(browser, false, popupFeatures, windowInfo, client,settings);
		} else {
			NotifyNewTab(browser,target_url);
		}
		return true;
	}

	void ClientHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser)
	{
		CEF_REQUIRE_UI_THREAD();
		int nBrowserId = browser->GetIdentifier();

		if (!m_MessageRouter) {
			// Create the browser-side router for query handling.
			CefMessageRouterConfig config;
			m_MessageRouter = CefMessageRouterBrowserSide::Create(config);

			// Register handlers with the router.
			ClientRunner::CreateMessageHandlers(m_MessageHandlerSet);
			MessageHandlerSet::const_iterator it = m_MessageHandlerSet.begin();
			for (; it != m_MessageHandlerSet.end(); ++it) {
				m_MessageRouter->AddHandler(*(it), false);
			}
		}

		if(m_nDefaultBrowserId == -1) {
			m_nDefaultBrowserId = browser->GetIdentifier();
		}
		m_BrowserList.push_back(browser);

		// Disable mouse cursor change if requested via the command-line flag.
		if (m_bMouseCursorChangeDisabled) {
			browser->GetHost()->SetMouseCursorChangeDisabled(true);
		}

		NotifyBrowserCreated(browser);
	}

	bool ClientHandler::DoClose(CefRefPtr<CefBrowser> browser)
	{
		CEF_REQUIRE_UI_THREAD();

		NotifyBrowserClosing(browser);

		HWND hWnd = browser->GetHost()->GetWindowHandle();
		browser = NULL;
		if(hWnd) {
			PostMessage(hWnd, WM_CLOSE, 0, 0);
		}

		// Allow the close.
		// For windowed browsers this will result in the OS close event being sent.
		return false;
	}

	void ClientHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser)
	{
		CEF_REQUIRE_UI_THREAD();

		//AutoLock lock_scope(this);

		m_MessageRouter->OnBeforeClose(browser);

		std::vector<CefRefPtr<CefBrowser>>::iterator item = m_BrowserList.begin();
		for (; item != m_BrowserList.end(); item++)
		{
			if ((*item)->IsSame(browser)){
				m_BrowserList.erase(item);
				browser = NULL;
				break;
			}
		}

		NotifyBrowserClosed(browser);

		if (m_BrowserList.empty()) {
			m_nDefaultBrowserId = -1;
			// Remove and delete message router handlers.
			MessageHandlerSet::const_iterator it = m_MessageHandlerSet.begin();
			for (; it != m_MessageHandlerSet.end(); ++it) {
				m_MessageRouter->RemoveHandler(*(it));
				delete *(it);
			}
			m_MessageHandlerSet.clear();
			m_MessageRouter = NULL;
			NotifyBrowserAllClosed();
		}
	}

	void ClientHandler::OnLoadingStateChange(CefRefPtr<CefBrowser> browser,
		bool isLoading, 
		bool canGoBack, 
		bool canGoForward)
	{
		CEF_REQUIRE_UI_THREAD();

		NotifyLoadingState(browser, isLoading, canGoBack, canGoForward);
	}

	void ClientHandler::OnLoadStart(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		TransitionType transition_type)
	{
		CEF_REQUIRE_UI_THREAD();

		// if (m_BrowserId == browser->GetIdentifier() && 
		//     frame->IsMain())
		// {
		//	// We've just started loading a page
		//	SetLoading(true);
		//	Invoke_LoadStart(browser, frame);
		// }
	}

	void ClientHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser, 
		CefRefPtr<CefFrame> frame, 
		int httpStatusCode)
	{
		CEF_REQUIRE_UI_THREAD();

		// if (m_BrowserId == browser->GetIdentifier() && frame->IsMain())
		// {
		//	frame->ExecuteJavaScript("alert('ExecuteJavaScript works!');", frame->GetURL(), 0);
		//	CefRefPtr<CefV8Context> v8 = frame->GetV8Context();
		// }
	}

	void ClientHandler::OnLoadError(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		ErrorCode errorCode,
		const CefString& errorText,
		const CefString& failedUrl)
	{
		CEF_REQUIRE_UI_THREAD();

		// Don't display an error for downloaded files.
		if (errorCode == ERR_ABORTED) { return; }

		// Don't display an error for external protocols that we allow the OS to
		// handle. See OnProtocolExecution().
		if (errorCode == ERR_UNKNOWN_URL_SCHEME) {
			std::string urlStr = frame->GetURL();
			if (urlStr.find("spotify:") == 0) { return; }
		}

		// Load the error page.
		LoadErrorPage(frame, failedUrl, errorCode, errorText);
	}


	bool ClientHandler::OnBeforeBrowse(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefRefPtr<CefRequest> request,
		bool user_gesture,
		bool is_redirect)
	{
		CEF_REQUIRE_UI_THREAD();

		m_MessageRouter->OnBeforeBrowse(browser, frame);
		return false;
	}

	cef_return_value_t ClientHandler::OnBeforeResourceLoad(
		CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefRefPtr<CefRequest> request,
		CefRefPtr<CefRequestCallback> callback)
	{
		CEF_REQUIRE_IO_THREAD();
		return m_ResourceManager->OnBeforeResourceLoad(browser, frame, request, callback);
	}

	CefRefPtr<CefResourceHandler> ClientHandler::GetResourceHandler(
		CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefRefPtr<CefRequest> request)
	{
		CEF_REQUIRE_IO_THREAD();
		return m_ResourceManager->GetResourceHandler(browser, frame, request);
	}

	CefRefPtr<CefResponseFilter> ClientHandler::GetResourceResponseFilter(
		CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefRefPtr<CefRequest> request,
		CefRefPtr<CefResponse> response)
	{
		CEF_REQUIRE_IO_THREAD();
		return NULL;
	}

	bool ClientHandler::OnOpenURLFromTab(
		CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		const CefString& target_url,
		cef_window_open_disposition_t target_disposition,
		bool user_gesture)
	{

		NotifyNewTab(browser,target_url);
		return true;

		// Open the URL in the current browser window.
		// return false;
	}

	bool ClientHandler::OnQuotaRequest(CefRefPtr<CefBrowser> browser,
		const CefString& origin_url,
		int64 new_size,
		CefRefPtr<CefRequestCallback> callback) {
			CEF_REQUIRE_IO_THREAD();

			// 20mb.
			static const int64 max_size = 1024 * 1024 * 20;

			// Grant the quota request if the size is reasonable.
			callback->Continue(new_size <= max_size);
			return true;
	}

	void ClientHandler::OnProtocolExecution(CefRefPtr<CefBrowser> browser,
		const CefString& url,
		bool& allow_os_execution) {
			CEF_REQUIRE_UI_THREAD();

			std::string urlStr = url;

			// Allow OS execution of Spotify URIs.
			if (urlStr.find("spotify:") == 0) {
				allow_os_execution = true;
			}
	}


	bool ClientHandler::OnCertificateError(
		CefRefPtr<CefBrowser> browser,
		ErrorCode cert_error,
		const CefString& request_url,
		CefRefPtr<CefSSLInfo> ssl_info,
		CefRefPtr<CefRequestCallback> callback) {
			CEF_REQUIRE_UI_THREAD();

			// CefRefPtr<CefSSLCertPrincipal> subject = ssl_info->GetSubject();
			// CefRefPtr<CefSSLCertPrincipal> issuer = ssl_info->GetIssuer();

			// // Build a table showing certificate information. Various types of invalid
			// // certificates can be tested using https://badssl.com/.
			// std::stringstream ss;
			// ss << "X.509 Certificate Information:"
			//	"<table border=1><tr><th>Field</th><th>Value</th></tr>" <<
			//	"<tr><td>Subject</td><td>" <<
			//	(subject.get() ? subject->GetDisplayName().ToString() : "&nbsp;") <<
			//	"</td></tr>"
			//	"<tr><td>Issuer</td><td>" <<
			//	(issuer.get() ? issuer->GetDisplayName().ToString() : "&nbsp;") <<
			//	"</td></tr>"
			//	"<tr><td>Serial #*</td><td>" <<
			//	GetBinaryString(ssl_info->GetSerialNumber()) << "</td></tr>"
			//	"<tr><td>Status</td><td>" <<
			//	GetCertStatusString(ssl_info->GetCertStatus()) << "</td></tr>"
			//	"<tr><td>Valid Start</td><td>" <<
			//	GetTimeString(ssl_info->GetValidStart()) << "</td></tr>"
			//	"<tr><td>Valid Expiry</td><td>" <<
			//	GetTimeString(ssl_info->GetValidExpiry()) << "</td></tr>";

			// CefSSLInfo::IssuerChainBinaryList der_chain_list;
			// CefSSLInfo::IssuerChainBinaryList pem_chain_list;
			// ssl_info->GetDEREncodedIssuerChain(der_chain_list);
			// ssl_info->GetPEMEncodedIssuerChain(pem_chain_list);
			// DCHECK_EQ(der_chain_list.size(), pem_chain_list.size());

			// der_chain_list.insert(der_chain_list.begin(), ssl_info->GetDEREncoded());
			// pem_chain_list.insert(pem_chain_list.begin(), ssl_info->GetPEMEncoded());

			// for (size_t i = 0U; i < der_chain_list.size(); ++i) {
			//	ss << "<tr><td>DER Encoded*</td>"
			//		"<td style=\"max-width:800px;overflow:scroll;\">" <<
			//		GetBinaryString(der_chain_list[i]) << "</td></tr>"
			//		"<tr><td>PEM Encoded*</td>"
			//		"<td style=\"max-width:800px;overflow:scroll;\">" <<
			//		GetBinaryString(pem_chain_list[i]) << "</td></tr>";
			//}

			// ss << "</table> * Displayed value is base64 encoded.";

			// Load the error page.
			std::stringstream ss;
			ss << "Certificate Error!";
			LoadErrorPage(browser->GetMainFrame(), request_url, cert_error, ss.str());
			
			// Cancel the request.
			return false;
	}

	void ClientHandler::OnRenderProcessTerminated(CefRefPtr<CefBrowser> browser, TerminationStatus status)
	{
		CEF_REQUIRE_UI_THREAD();
		m_MessageRouter->OnRenderProcessTerminated(browser);
	}

	CefRefPtr<CefBrowser> ClientHandler::GetBrowser(int nBrowserId)
	{
		int nFindId = nBrowserId;
		if(nFindId <= 0){
			nFindId = m_nDefaultBrowserId;
		}
		std::vector<CefRefPtr<CefBrowser>>::iterator item = m_BrowserList.begin();
		for (; item != m_BrowserList.end(); item++)
		{
			if ((*item)->GetIdentifier() == nFindId){
				return (*item);
			}
		}
		return NULL;
	}

	void ClientHandler::ShowDevTools(CefRefPtr<CefBrowser> browser, const CefPoint& inspect_element_at)
	{
		CefWindowInfo windowInfo;
		CefRefPtr<CefClient> client;
		CefBrowserSettings settings;
		int nBrowserId = browser->GetIdentifier();

		if (CreatePopupWindow(browser, true, CefPopupFeatures(), windowInfo, client, settings)) {
			browser->GetHost()->ShowDevTools(windowInfo, client, settings, inspect_element_at);
		}
	}

	void ClientHandler::CloseDevTools(CefRefPtr<CefBrowser> browser)
	{
		browser->GetHost()->CloseDevTools();
	}

	bool ClientHandler::CreatePopupWindow(
		CefRefPtr<CefBrowser> browser,
		bool is_devtools,
		const CefPopupFeatures& popupFeatures,
		CefWindowInfo& windowInfo,
		CefRefPtr<CefClient>& client,
		CefBrowserSettings& settings)
	{
		int nBrowserId = browser->GetIdentifier();
		// Note: This method will be called on multiple threads.

		// The popup browser will be parented to a new native window.
		// Don't show URL bar and navigation buttons on DevTools windows.
		BrowserDlgManager* pManager = MainContext::Get()->GetBrowserDlgManager();
		if(pManager){
			pManager->CreateBrowserDlgAsPopup(!is_devtools, popupFeatures, windowInfo, client, settings);
		}

		return true;
	}

	void ClientHandler::NotifyBrowserCreated(CefRefPtr<CefBrowser> browser)
	{
		if (!CefCurrentlyOn(TID_UI)) {
			// Execute this method on the main thread.
			CefPostTask(TID_UI, CefCreateClosureTask(base::Bind(&ClientHandler::NotifyBrowserCreated, this, browser)));
			return;
		}

		if (m_Delegate) {
			m_Delegate->OnBrowserCreated(browser);
		}
	}

	void ClientHandler::NotifyBrowserClosing(CefRefPtr<CefBrowser> browser)
	{
		if (!CefCurrentlyOn(TID_UI)) {
			// Execute this method on the main thread.
			CefPostTask(TID_UI, CefCreateClosureTask(base::Bind(&ClientHandler::NotifyBrowserClosing, this, browser)));
			return;
		}

		if (m_Delegate) {
			m_Delegate->OnBrowserClosing(browser);
		}
	}

	void ClientHandler::NotifyBrowserClosed(CefRefPtr<CefBrowser> browser) {
		if (!CefCurrentlyOn(TID_UI)) {
			// Execute this method on the main thread.
			CefPostTask(TID_UI, CefCreateClosureTask(base::Bind(&ClientHandler::NotifyBrowserClosed, this, browser)));
			return;
		}

		if (m_Delegate) {
			m_Delegate->OnBrowserClosed(browser);
		}
	}

	void ClientHandler::NotifyBrowserAllClosed() {
		if (!CefCurrentlyOn(TID_UI)) {
			// Execute this method on the main thread.
			CefPostTask(TID_UI, CefCreateClosureTask(base::Bind(&ClientHandler::NotifyBrowserAllClosed, this)));
			return;
		}

		if (m_Delegate) {
			m_Delegate->OnBrowserAllClosed();
		}
	}

	void ClientHandler::NotifyAddress(CefRefPtr<CefBrowser> browser, const CefString& url) {
		if (!CefCurrentlyOn(TID_UI)) {
			// Execute this method on the main thread.
			CefPostTask(TID_UI, CefCreateClosureTask(base::Bind(&ClientHandler::NotifyAddress, this, browser, url)));
			return;
		}

		if (m_Delegate) {
			m_Delegate->OnSetAddress(browser, url);
		}
	}

	void ClientHandler::NotifyTitle(CefRefPtr<CefBrowser> browser, const CefString& title) {
		if (!CefCurrentlyOn(TID_UI)) {
			// Execute this method on the main thread.
			CefPostTask(TID_UI, CefCreateClosureTask(base::Bind(&ClientHandler::NotifyTitle, this, browser, title)));
			return;
		}

		if (m_Delegate) {
			m_Delegate->OnSetTitle(browser, title);
		}
	}

	void ClientHandler::NotifyFullscreen(CefRefPtr<CefBrowser> browser, bool fullscreen) {
		if (!CefCurrentlyOn(TID_UI)) {
			// Execute this method on the main thread.
			CefPostTask(TID_UI, CefCreateClosureTask(base::Bind(&ClientHandler::NotifyFullscreen, this, browser, fullscreen)));
			return;
		}

		if (m_Delegate) {
			m_Delegate->OnSetFullscreen(browser, fullscreen);
		}
	}

	void ClientHandler::NotifyLoadingState(CefRefPtr<CefBrowser> browser, bool isLoading,bool canGoBack,bool canGoForward) {
		if (!CefCurrentlyOn(TID_UI)) {
			// Execute this method on the main thread.
			CefPostTask(TID_UI, CefCreateClosureTask(base::Bind(&ClientHandler::NotifyLoadingState, this, browser, isLoading, canGoBack, canGoForward)));
			return;
		}

		if (m_Delegate) {
			m_Delegate->OnSetLoadingState(browser, isLoading, canGoBack, canGoForward);
		}
	}

	void ClientHandler::NotifyDraggableRegions(CefRefPtr<CefBrowser> browser, const std::vector<CefDraggableRegion>& regions) {
		if (!CefCurrentlyOn(TID_UI)) {
			// Execute this method on the main thread.
			CefPostTask(TID_UI, CefCreateClosureTask(base::Bind(&ClientHandler::NotifyDraggableRegions, this, browser, regions)));
			return;
		}

		if (m_Delegate) {
			m_Delegate->OnSetDraggableRegions(browser, regions);
		}
	}

	void ClientHandler::NotifyNewTab(
		CefRefPtr<CefBrowser> browser, 
		const CefString& url)
	{
		if (url.empty()) { return; }
		if (!CefCurrentlyOn(TID_UI)) {
			// Execute this method on the main thread.
			CefPostTask(TID_UI, CefCreateClosureTask(base::Bind(&ClientHandler::NotifyNewTab, this, browser, url)));
			return;
		}

		if (m_Delegate) {
			m_Delegate->OnNewTab(browser, url);
		}
	}
}
