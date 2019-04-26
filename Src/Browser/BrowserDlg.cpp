#include "stdafx.h"
#include "BrowserDlg.h"
#include "MainContext.h"
#include "BrowserDlgManager.h"

#include "HttpClient.h"
#include "LoginDialog.h"
#include "MachineCodeDlg.h"
#include "QuitConfirmDialog.h"
#include "EncryptByMd5.h"
#include "Globals.h"

#define WM_SHOW_LOGIN_DLG WM_USER + 1

namespace Browser
{
	namespace {
		const int FrameLeft = 2;
		const int FrameTop = 1;
		const int FrameRight = 2;
		const int FrameBottom = 2;
		const int TitleHeight = 32;
		const int ToolbarHeight = 28;
	}

	namespace {
		bool bActiveNewWindow = true;
	}

	BrowserDlg::BrowserDlg()
		: m_Delegate(NULL),
		m_hParent(NULL),
		m_pBrowserUI(NULL),
		m_bWithControls(false),
		m_bToolbarVisible(false),
		m_bIsPopup(false),
		m_rcStart(),
		m_bInitialized(false),
		m_bWindowDestroyed(false),
		m_bBrowserDestroyed(false),
		m_nCurBrowserId(0)
	{
		uiTabs = NULL;
		pTitle = NULL;
		tabNew = NULL;
		uiToolbar = NULL;
		btnBackward = NULL;
		btnForward = NULL;
		editUrl = NULL;
		editKeyword = NULL;
	}

	BrowserDlg::~BrowserDlg()
	{
		DCHECK(m_bWindowDestroyed);
		DCHECK(m_bBrowserDestroyed);
		// PostQuitMessage(0);
	}

	LPCTSTR BrowserDlg::GetWindowClassName() const
	{
		return _T("BrowserDlg");
	}

	void BrowserDlg::InitWindow()
	{
		SetIcon(IDR_MAINFRAME);
		uiTabs = static_cast<DuiLib::CHorizontalLayoutUI*>(m_Manager.FindControl(_T("uiTabs")));
		tabNew = static_cast<DuiLib::CButtonUI*>(m_Manager.FindControl(_T("tabNew")));
		uiToolbar = static_cast<DuiLib::CControlUI*>(m_Manager.FindControl(_T("uiToolbar")));
		btnBackward = static_cast<DuiLib::CButtonUI*>(m_Manager.FindControl(_T("btnBackward")));
		btnForward = static_cast<DuiLib::CButtonUI*>(m_Manager.FindControl(_T("btnForward")));
		editUrl = static_cast<DuiLib::CEditUI*>(m_Manager.FindControl(_T("editUrl")));
		editKeyword = static_cast<DuiLib::CEditUI*>(m_Manager.FindControl(_T("editKeyword")));
		btnActiveNewWin = static_cast<DuiLib::COptionUI*>(m_Manager.FindControl(_T("btnActiveNewWin")));
		DuiLib::CLabelUI* labelTitle = static_cast<DuiLib::CLabelUI*>(m_Manager.FindControl(_T("titlelabel")));		
		DuiLib::CControlUI* layoutUrl = static_cast<DuiLib::CControlUI*>(m_Manager.FindControl(_T("layoutUrl")));
		DuiLib::CControlUI* ctrlSeperator1 = static_cast<DuiLib::CControlUI*>(m_Manager.FindControl(_T("seperator1")));
		DuiLib::CControlUI* ctrlSeperator2 = static_cast<DuiLib::CControlUI*>(m_Manager.FindControl(_T("seperator2")));
		DuiLib::CControlUI* ctrlSeperator3 = static_cast<DuiLib::CControlUI*>(m_Manager.FindControl(_T("seperator3")));
		DuiLib::CControlUI* ctrlSeperator4 = static_cast<DuiLib::CControlUI*>(m_Manager.FindControl(_T("seperator4")));

		if (uiTabs == NULL || 
			tabNew == NULL || 
			uiToolbar == NULL || 
			editUrl == NULL   
			// || editKeyword == NULL
			) {
			MessageBox(NULL, _T("加载资源文件失败"), _T("Browser"), MB_OK | MB_ICONERROR);
			return;
		}
		
		if (m_bWithControls) {
			uiToolbar->SetVisible(true);
			if(btnBackward) {
				btnBackward->SetEnabled(false);
			}
			if(btnForward) {
				btnForward->SetEnabled(false);
			}
		} else {
			uiToolbar->SetVisible(false);
		}

		if (m_bIsPopup) {
			tabNew->SetVisible(false);
			uiToolbar->SetVisible(false);
		} else {
			tabNew->SetVisible(true);
		}

		if (labelTitle != NULL) {
			DuiLib::CDuiString strTitleText;
			strTitleText.Format(_T("【TopInfo浏览器%s】- %s%s"), 
				GLOBALS::strMajorVersion.GetBuffer(), // _T("3.3.2"),
				GLOBALS::strCompanyName.GetBuffer(),  // _T("北京分公司"),
				GLOBALS::strUsageTips.GetBuffer()     // _T("店面专用版"
			);
			labelTitle->SetText(strTitleText);
		}
		
		// 是否显示地址栏
		if(GLOBALS::strShowAddressBar.CompareNoCase(_T("1")) == 0) {
		    btnBackward->SetVisible(true);
		    btnForward->SetVisible(true);
		    layoutUrl->SetVisible(true);
			ctrlSeperator1->SetVisible(true);
			ctrlSeperator2->SetVisible(true);
			ctrlSeperator3->SetVisible(true);
		} else {
			ctrlSeperator4->SetVisible(true);
			tabNew->SetVisible(false);
		}

		if (m_bWithControls) {
			if (GLOBALS::strShowToolBar.CompareNoCase(_T("1")) == 0) {
				m_bToolbarVisible = true;
			}
		}
		// else {
		//	m_bToolbarVisible = false;
		// }

		uiToolbar->SetVisible(m_bToolbarVisible);
		btnActiveNewWin->Selected(true);
		// 窗口最大化
		SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0);

		if (GLOBALS::strShowToolBar.CompareNoCase(_T("1")) != 0) {
			PostMessage(WM_SHOW_LOGIN_DLG, 0, 0);
		}
	}

	void BrowserDlg::OnFinalMessage(HWND hWnd)
	{
		WindowImplBase::OnFinalMessage(hWnd);
		// delete this;
		m_bWindowDestroyed = true;
		NotifyDestroyedIfDone();
	}

	DuiLib::CDuiString BrowserDlg::GetSkinFile()
	{
		return _T("Skin\\BrowserDlg.xml");
	}

	LRESULT BrowserDlg::ResponseDefaultKeyEvent(WPARAM wParam)
	{
		if (wParam == VK_RETURN) {
			return FALSE;
		} else if (wParam == VK_ESCAPE)	{
			return TRUE;
		}
		return FALSE;
	}

	DuiLib::CControlUI* BrowserDlg::CreateControl(LPCTSTR pstrClass)
	{
		DuiLib::CControlUI* pUI = NULL;
		if (_tcsicmp(pstrClass, _T("BrowserUI")) == 0) {
			if(m_pBrowserUI == NULL){
				m_pBrowserUI = new Browser::BrowserUI(this, m_hWnd);
			}
			if (m_pBrowserUI != NULL && m_BrowserCtrl.get() != NULL){
				m_pBrowserUI->SetCtrl(m_BrowserCtrl.get());
			}
			pUI = m_pBrowserUI;
		} else if (_tcsicmp(pstrClass, _T("Title")) == 0)
		{
			pUI = new Browser::TitleUI();
		}

		return pUI;
	}

	LRESULT BrowserDlg::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch( uMsg )
		{
		case WM_CLOSE:
			if(wParam == 0L) {
				return 0L;
			}
			break;
		case WM_SHOW_LOGIN_DLG:
			ShowLoginDlg();
			break;
		default:
			break;
		}
		return WindowImplBase::HandleMessage(uMsg, wParam, lParam);
	}

	void BrowserDlg::Notify(DuiLib::TNotifyUI& msg)
	{
		DuiLib::CDuiString sCtrlName = msg.pSender->GetName();

		if (_tcsicmp(msg.sType, DUI_MSGTYPE_CLICK) == 0)
		{
			if (_tcsicmp(sCtrlName, _T("btnActiveNewWin")) == 0) {
				bActiveNewWindow = !bActiveNewWindow;
			} else if (_tcsicmp(sCtrlName, _T("btnClose")) == 0) {
				if(m_bWithControls) {
					// 退出前确认
					CQuitConfirmDlg* quitConfirmDlg = new CQuitConfirmDlg();
					quitConfirmDlg->Create(m_hWnd, _T(""), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
					quitConfirmDlg->CenterWindow();
					quitConfirmDlg->ShowModal();

					CStdString strConfirmResult = quitConfirmDlg->m_strConfirmResult;
					delete quitConfirmDlg;

					if (!strConfirmResult.IsEmpty()) {
						Logout();
						Close();
					}
				} else {
					// 开发者工具
					Close();
				}
			} else if (_tcsicmp(sCtrlName, _T("btnLogo")) == 0) {
				if (m_bWithControls) {
					if (!m_bToolbarVisible) {
						m_bToolbarVisible = true;
						uiToolbar->SetVisible(m_bToolbarVisible);
					} else {
						m_bToolbarVisible = false;
						uiToolbar->SetVisible(m_bToolbarVisible);
					}
				} else {
					uiToolbar->SetVisible(false);
				}
			} else if (_tcsicmp(sCtrlName, _T("btnGoto")) == 0){
				// 跳转
				DuiLib::CDuiString sUrl = editUrl->GetText();
				editUrl->SetText(sUrl);
				LoadURL(sUrl.GetData());
			} else if (_tcsicmp(sCtrlName, _T("btnSearch")) == 0) {
				// 搜索
				DuiLib::CDuiString sUrl, sKeyword = editKeyword->GetText();
				sUrl.Format(_T("https://www.baidu.com/s?wd=%s"), sKeyword.GetData());
				editUrl->SetText(sUrl);
				LoadURL(sUrl.GetData());
			} else if (_tcsicmp(sCtrlName, _T("btnHome")) == 0) {
				// 主页
				editUrl->SetText(MainContext::Get()->GetMainURL().c_str());
				LoadURL(MainContext::Get()->GetMainURL().c_str());
			} else if (_tcsicmp(sCtrlName, _T("btnMail")) == 0) {
				// 邮箱
				editUrl->SetText(GLOBALS::strMailUrl.GetBuf());
				LoadURL(GLOBALS::strMailUrl.GetBuf());
			} else if (_tcsicmp(sCtrlName, _T("btnTool")) == 0) {
				// 实勘助手
				TCHAR szFilePath[MAX_PATH];
				GetModuleFileName(NULL, szFilePath, _MAX_PATH);
				CStdString strRunDir = szFilePath;
				int nPos = strRunDir.ReverseFind('\\');
				if (nPos != -1) { strRunDir = strRunDir.Left(nPos + 1); }

				CStdString strImageTunerPath = CStdString(strRunDir).append(_T("ImageTuner.exe"));
				ShellExecute(NULL,
					_T("open"),
					strImageTunerPath,
					NULL,
					NULL,
					SW_SHOWNORMAL);
			} else if (_tcsicmp(sCtrlName, _T("btnUpdate")) == 0) {
				// 检查更新
				TCHAR szFilePath[MAX_PATH];
				GetModuleFileName(NULL, szFilePath, _MAX_PATH);
				CStdString strRunDir = szFilePath;
				int nPos = strRunDir.ReverseFind('\\');
				if (nPos != -1) { strRunDir = strRunDir.Left(nPos + 1); }

				CStdString strImageTunerPath = CStdString(strRunDir).append(_T("AutoUpdate.exe"));
				ShellExecute(NULL,
					_T("open"),
					strImageTunerPath,
					NULL,
					NULL,
					SW_SHOWNORMAL);
			} else if (_tcsicmp(sCtrlName, _T("btnSettings")) == 0) {
				DuiLib::CDuiString sUrl = _T("about:settings");
				editUrl->SetText(sUrl);
				LoadURL(sUrl.GetData());
			} else if (_tcsicmp(sCtrlName, _T("btnLogin")) == 0) {
				ShowLoginDlg();
			} else if (_tcsicmp(sCtrlName, _T("btnMachineCode")) == 0) {
				CMachineCodeDlg* machineCodeDlg = new CMachineCodeDlg();
				machineCodeDlg->Create(m_hWnd, _T(""), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
				machineCodeDlg->CenterWindow();
				machineCodeDlg->ShowModal();
			} else if (_tcsicmp(sCtrlName, _T("btnBackward")) == 0) {
				CefRefPtr<CefBrowser> browser = GetBrowser();
				if (browser) {
					browser->GoBack();
				}
			} else if (_tcsicmp(sCtrlName, _T("btnForward")) == 0) {
				CefRefPtr<CefBrowser> browser = GetBrowser();
				if (browser) {
					browser->GoForward();
				}
			} else if (_tcsicmp(sCtrlName, _T("tabNew")) == 0) {
				NewTab(L"about:blank");
			} else if (_tcsnicmp(sCtrlName, _T("tabClose"), 8) == 0) {
				CDuiString sBuffer = sCtrlName;
				sBuffer.Replace(_T("tabClose"), _T(""));
				int nBrowserId = _ttoi(sBuffer.GetData());
				int nTabsCount = uiTabs->GetCount();
				if (nTabsCount <= 2) {
					LoadURL(L"about:blank");
				} else {
					for (int idx = 0; idx < nTabsCount; idx++) {
						TitleUI* pItem = (TitleUI*)uiTabs->GetItemAt(idx);
						if (pItem != NULL) {
							if (pItem->GetTag() == nBrowserId) {
								if (m_pBrowserUI) {
									m_pBrowserUI->CloseBrowser(nBrowserId);
								}
							}
						}
					}
				}
			}
		} else if (_tcsicmp(msg.sType, DUI_MSGTYPE_RETURN) == 0) {
			if (_tcsicmp(sCtrlName, _T("editUrl")) == 0) {
				DuiLib::CDuiString sUrl = editUrl->GetText();
				editUrl->SetText(sUrl);
				LoadURL(sUrl.GetData());
			} else if (_tcsicmp(sCtrlName, _T("editKeyword")) == 0) {
				DuiLib::CDuiString sUrl,sKeyword = editKeyword->GetText();
				sUrl.Format(_T("https://www.baidu.com/s?wd=%s"), sKeyword.GetData());
				editUrl->SetText(sUrl);
				LoadURL(sUrl.GetData());
			}
		} else if (_tcsicmp(msg.sType, DUI_MSGTYPE_SELECTCHANGED) == 0) {
			if (_tcsnicmp(sCtrlName, _T("tabTitle"), 8) == 0){
				CDuiString sBuffer = msg.pSender->GetText();
				SetWindowText(m_hWnd, sBuffer.GetData());
				sBuffer = msg.pSender->GetUserData();
				editUrl->SetText(sBuffer.GetData());
				if (m_pBrowserUI){
					m_nCurBrowserId = msg.pSender->GetTag();
					m_pBrowserUI->ShowBrowser(m_nCurBrowserId);
				}
			}
		} else if (_tcsicmp(msg.sType, DUI_MSGTYPE_DBCLICK) == 0) {
			if (_tcsnicmp(sCtrlName, _T("tabTitle"), 8) == 0) {
				// 双击关闭标签页
				CDuiString sBuffer = sCtrlName;
				sBuffer.Replace(_T("tabTitle"), _T(""));
				int nBrowserId = _ttoi(sBuffer.GetData());
				int nTabsCount = uiTabs->GetCount();
				if (nTabsCount <= 2) {
					LoadURL(L"about:blank");
				} else {
					for (int idx = 0; idx < nTabsCount; idx++) {
						TitleUI* pItem = (TitleUI*)uiTabs->GetItemAt(idx);
						if (pItem != NULL) {
							if (pItem->GetTag() == nBrowserId) {
								if (m_pBrowserUI) {
									m_pBrowserUI->CloseBrowser(nBrowserId);
								}
							}
						}
					}
				}
			}
		}
		
		return WindowImplBase::Notify(msg);
	}

	void BrowserDlg::OnDragLinkDrop(CefRefPtr<CefBrowser> browser, 
		CefString linkUrl) {
		if (!bActiveNewWindow) {
			NewTab(CStdString(linkUrl.c_str()));
		}
	}

	void BrowserDlg::OnBrowserCreated(CefRefPtr<CefBrowser> browser)
	{
		DCHECK(CefCurrentlyOn(TID_UI));
		
		int nBrowserId = browser->GetIdentifier();
		if(m_nCurBrowserId != nBrowserId){
			if (bActiveNewWindow) {
				// 如果打开新链接时需要激活新窗口
				m_nCurBrowserId = nBrowserId;
			} else {
				// 无需激活新窗口
				if(m_nCurBrowserId == 0) {
					m_nCurBrowserId = nBrowserId;
				}
			}
		}

		if (m_bIsPopup) {
			// For popup browsers create the root window once the browser has been created.
			CreateBrowserDlg(CefBrowserSettings());
		} else {
			if (m_pBrowserUI){
				RECT rcPos = m_pBrowserUI->GetPos();
				if(m_BrowserCtrl){
					m_BrowserCtrl->ShowBrowser(
						m_nCurBrowserId,rcPos.left, 
						rcPos.top, 
						rcPos.right - rcPos.left, 
						rcPos.bottom - rcPos.top);
				}
			}
		}
	}
	
	void BrowserDlg::OnBrowserClosing(CefRefPtr<CefBrowser> browser)
	{
		DCHECK(CefCurrentlyOn(TID_UI));
		int nBrowserId = browser->GetIdentifier();
		int nTabsCount = uiTabs->GetCount();
		for (int idx = 0; idx < nTabsCount; idx++) {
			TitleUI* pItem = (TitleUI*)uiTabs->GetItemAt(idx);
			if (pItem != NULL) {				
				if (pItem->GetTag() == nBrowserId) {
					TitleUI* pTitle;
					if (idx > 0) {
						pTitle = (TitleUI*)uiTabs->GetItemAt(idx - 1);
					} else {
						pTitle = (TitleUI*)uiTabs->GetItemAt(idx + 1);
					}
					uiTabs->Remove(pItem);
					pTitle->Selected(true);
				}
			}
		}
	}

	void BrowserDlg::OnBrowserClosed(CefRefPtr<CefBrowser> browser)
	{
		DCHECK(CefCurrentlyOn(TID_UI));
	}

	void BrowserDlg::OnBrowserAllClosed()
	{
		DCHECK(CefCurrentlyOn(TID_UI));

		m_BrowserCtrl.reset();

		if (!m_bWindowDestroyed) {
			// The browser was destroyed first. This could be due to the use of
			// off-screen rendering or execution of JavaScript window.close().
			// Close the RootWindow.
			Close(true);
		}

		m_bBrowserDestroyed = true;
		NotifyDestroyedIfDone();
	}

	void BrowserDlg::OnSetAddress(
		CefRefPtr<CefBrowser> browser, 
		const CefString& url)
	{
		DCHECK(CefCurrentlyOn(TID_UI));
		CDuiString sBuffer;
		CDuiString sUrl;
		bool bAddTab = true;
		int nTabsCount = uiTabs->GetCount();
		int nBrowserId = browser->GetIdentifier();
		if(url.length() > 0) {
			sUrl = url.c_str();
		}

		for (int idx = 0; idx < nTabsCount; idx++)
		{
			DuiLib::CControlUI* pTitle = (DuiLib::CControlUI*)uiTabs->GetItemAt(idx);
			if (pTitle == NULL || _tcsicmp(pTitle->GetClass(), _T("ButtonUI")) == 0) {
				continue;
			}
			if (pTitle->GetTag() == nBrowserId){
				bAddTab = false;
				pTitle->SetUserData(sUrl);
				CDuiString sBuffer = pTitle->GetText();
				if(sBuffer.GetLength() == 0) {
					pTitle->SetText(sUrl);
				}
				break;
			}
		}
		
		if (nBrowserId == m_nCurBrowserId) {
			editUrl->SetText(sUrl);
		}

		if(bAddTab){
			if(m_bIsPopup){
				if(pTitle == NULL){
					pTitle = new CLabelUI;
				}
				pTitle->SetText(sUrl);
				uiTabs->AddAt(pTitle, nTabsCount - 1);
				sBuffer.Format(_T("name=\"labTitle\" height=\"24\" floatalign=\"right\" textpadding=\"5,1,20,2\" textcolor=\"FFFFFFFF\""));
				pTitle->ApplyAttributeList(sBuffer);
			} else {
				TitleUI* pTitle = new TitleUI;
				pTitle->SetTag(nBrowserId);
				pTitle->SetUserData(sUrl);
				uiTabs->AddAt(pTitle, nTabsCount - 1);
				int iOriginalBrowserId = m_nCurBrowserId;

				m_nCurBrowserId = nBrowserId;
				sBuffer.Format(_T("name=\"tabTitle%d\" height=\"24\" minwidth=\"50\" maxwidth=\"156\" floatalign=\"right\" borderround=\"2,2\" textpadding=\"5,1,20,2\" bkcolor=\"FF3D85C6\" selectedbkcolor=\"FF1C4587\" textcolor=\"FFFFFFFF\" selectedtextcolor=\"FFFFFFFF\" group=\"Titles\""), nBrowserId);
				pTitle->ApplyAttributeList(sBuffer);
				CControlUI* pControl = new CControlUI;
				CButtonUI* pClose = new CButtonUI;
				pTitle->Add(pControl);
				pTitle->Add(pClose);
				sBuffer.Format(_T("name=\"tabClose%d\" width=\"24\" height=\"24\" floatalign=\"right\" normalimage=\"file='SysBtn\\btnTabClose.png' source='0,0,11,11' dest='8,8,19,19'\" hotimage=\"file='btnTabClose.png' source='11,0,22,11' dest='8,8,19,19'\" pushedimage=\"file='btnTabClose.png' source='22,0,33,11' dest='8,8,19,19'\""), nBrowserId);
				pClose->ApplyAttributeList(sBuffer);
				if(bActiveNewWindow) {
					// 打开新链接时激活新窗口
					pTitle->Selected(true);
				} else {
					// 不激活新窗口
					m_nCurBrowserId = iOriginalBrowserId;
					m_pBrowserUI->ShowBrowser(m_nCurBrowserId);
				}
				editUrl->SetText(sUrl);
			}
		}
	}

	void BrowserDlg::OnSetTitle(CefRefPtr<CefBrowser> browser, const CefString& title)
	{
		DCHECK(CefCurrentlyOn(TID_UI));
		CDuiString strTitle;
		bool bAddTab = true;
		int nTabsCount = uiTabs->GetCount();
		int nBrowserId = browser->GetIdentifier();
		if(title.length() > 0) {
			strTitle = title.c_str();
		}

		if(m_bIsPopup){
			if(pTitle == NULL){
				pTitle = new CLabelUI;
			}
			pTitle->SetText(strTitle);
			SetWindowText(m_hWnd, strTitle);
		} else {
			int nTabsCount = uiTabs->GetCount();
			int nBrowserId = browser->GetIdentifier();
			for (int idx = 0; idx < nTabsCount; idx++) {
				DuiLib::CControlUI* pTitle = (DuiLib::CControlUI*)uiTabs->GetItemAt(idx);
				if (pTitle == NULL || _tcsicmp(pTitle->GetClass(), _T("ButtonUI")) == 0) {
					continue;
				}
				if (pTitle->GetTag() == nBrowserId){
					pTitle->SetText(strTitle);
				}
			}
			if (nBrowserId == m_nCurBrowserId) {
				SetWindowText(m_hWnd, strTitle);
			}
		}

	}

	void BrowserDlg::OnSetFullscreen(
		CefRefPtr<CefBrowser> browser, 
		bool fullscreen)
	{
		DCHECK(CefCurrentlyOn(TID_UI));
	}

	void BrowserDlg::OnSetLoadingState(
		CefRefPtr<CefBrowser> browser, 
		bool isLoading,
		bool canGoBack,
		bool canGoForward)
	{
		DCHECK(CefCurrentlyOn(TID_UI));
		if(btnBackward) {
			btnBackward->SetEnabled(canGoBack);
		}
		if(btnForward) {
			btnForward->SetEnabled(canGoForward);
		}
	}

	void BrowserDlg::OnSetDraggableRegions(
		CefRefPtr<CefBrowser> browser, 
		const std::vector<CefDraggableRegion>& regions)
	{
		DCHECK(CefCurrentlyOn(TID_UI));
	}

	void BrowserDlg::OnNewTab(
		CefRefPtr<CefBrowser> browser, 
		const CefString& url) {
		DCHECK(CefCurrentlyOn(TID_UI));
		NewTab(url);
	}

	void BrowserDlg::NotifyDestroyedIfDone() {
		// Notify once both the window and the browser have been destroyed.
		if (m_bWindowDestroyed && m_bBrowserDestroyed) {
			m_Delegate->OnBrowserDlgDestroyed(this);
		}
	}

	void BrowserDlg::Init(
		BrowserDlg::Delegate* delegate,
		HWND hParent,
		bool with_controls,
		const CefRect& bounds,
		const CefBrowserSettings& settings,
		const CefString& url)
	{
		DCHECK(delegate);
		DCHECK(!m_bInitialized);

		m_Delegate = delegate;
		m_bWithControls = true;

		m_rcStart.left = bounds.x;
		m_rcStart.top = bounds.y;
		m_rcStart.right = bounds.x + bounds.width;
		m_rcStart.bottom = bounds.y + bounds.height;

		CreateBrowserWindow(url);

		m_bInitialized = true;

		// Create the native root window on the main thread.
		if (CefCurrentlyOn(TID_UI)) {
			CreateBrowserDlg(settings);
		} else {
			CefPostTask(TID_UI,
				CefCreateClosureTask(
					base::Bind(
						&BrowserDlg::CreateBrowserDlg, 
						this, 
						settings)));
		}
	}

	void BrowserDlg::InitAsPopup(
		BrowserDlg::Delegate* delegate,
		bool with_controls,
		const CefPopupFeatures& popupFeatures,
		CefWindowInfo& windowInfo,
		CefRefPtr<CefClient>& client,
		CefBrowserSettings& settings)
	{
		DCHECK(delegate);
		DCHECK(!m_bInitialized);

		m_Delegate = delegate;
		m_bWithControls = with_controls;
		m_bIsPopup = true;

		if (popupFeatures.xSet) { m_rcStart.left = popupFeatures.x; }
		if (popupFeatures.ySet) { m_rcStart.top = popupFeatures.y; }
		if (popupFeatures.widthSet) { m_rcStart.right = m_rcStart.left + popupFeatures.width; }
		if (popupFeatures.heightSet) { m_rcStart.bottom = m_rcStart.top + popupFeatures.height; }
		
		CreateBrowserWindow(std::string());

		m_bInitialized = true;

		// The new popup is initially parented to a temporary window. The native root
		// window will be created after the browser is created and the popup window
		// will be re-parented to it at that time.
		m_BrowserCtrl->GetPopupConfig(TempWindow::GetWindowHandle(), windowInfo, client, settings);
	}

	void BrowserDlg::NewTab(const CefString& url)
	{
		if(m_BrowserCtrl && m_pBrowserUI && url.length() > 0){
			RECT rcPos = m_pBrowserUI->GetPos();
			CefRect cef_rect(rcPos.left, rcPos.top, rcPos.right - rcPos.left, rcPos.bottom - rcPos.top);
			CefBrowserSettings settings;
			MainContext::Get()->PopulateBrowserSettings(&settings);
			m_BrowserCtrl->CreateBrowser(
				m_hWnd, 
				url, 
				cef_rect, 
				settings, 
				m_Delegate->GetRequestContext());
		}
	}

	CefRefPtr<CefBrowser> BrowserDlg::GetBrowser()
	{
		DCHECK(CefCurrentlyOn(TID_UI));

		if (m_BrowserCtrl) {
			return m_BrowserCtrl->GetBrowser(m_nCurBrowserId);
		}
		return NULL;
	}

	CefWindowHandle BrowserDlg::GetWindowHandle()
	{
		return m_hWnd;
	}

	void BrowserDlg::LoadURL(const CefString& url)
	{
		if (m_BrowserCtrl.get() != NULL){
			CefRefPtr<CefBrowser> pBrowser = m_BrowserCtrl->GetBrowser(m_nCurBrowserId);
			if(pBrowser){
				CefRefPtr<CefFrame> pFrame = pBrowser->GetMainFrame();
				if (pFrame){
					pFrame->LoadURL(url);
				}
			}
		}
	}

	void BrowserDlg::CreateBrowserWindow(const CefString& startup_url)
	{
		m_BrowserCtrl.reset(new BrowserWindow(this, startup_url));
	}

	void BrowserDlg::CreateBrowserDlg(const CefBrowserSettings& settings)
	{
		DCHECK(CefCurrentlyOn(TID_UI));

		int x, y, width, height;
	
		if (::IsRectEmpty(&m_rcStart)) {
			// Use the default window position/size.
			x = y = width = height = CW_USEDEFAULT;
		} else {
			RECT rcWindow = m_rcStart;
			if (m_bWithControls){
				rcWindow.bottom += FrameTop + TitleHeight + FrameBottom;
				rcWindow.right += FrameLeft + FrameRight;
				if (!m_bIsPopup){
					rcWindow.bottom += ToolbarHeight;
				}
			}
			x = rcWindow.left;
			y = rcWindow.top;
			width = rcWindow.right - rcWindow.left;
			height = rcWindow.bottom - rcWindow.top;
		}

		// Create the main window.
		Create(m_hParent, 
			_T("Browser"),
			UI_WNDSTYLE_FRAME | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
			WS_EX_APPWINDOW,
			x, 
			y, 
			width, 
			height,
			NULL);

		::SetMenu(m_hWnd, NULL);

		if(m_pBrowserUI){
			RECT rect = m_pBrowserUI->GetPos();

			if(m_bIsPopup){
				// With popups we already have a browser window. Parent the browser window
				// to the root window and show it in the correct location.
				m_BrowserCtrl->ShowPopup(
					m_nCurBrowserId, 
					m_hWnd, 
					rect.left, 
					rect.top, 
					rect.right - rect.left, 
					rect.bottom - rect.top);
			} else {
				// Create the browser window.
				CefRect cef_rect(rect.left, rect.top,
					rect.right - rect.left,
					rect.bottom - rect.top);
				m_BrowserCtrl->CreateBrowser(
					m_hWnd, 
					std::wstring(),
					cef_rect, 
					settings, 
					m_Delegate->GetRequestContext());
			}
		}
	}

	void BrowserDlg::ShowLoginDlg() {
		CLoginDlg* loginDlg = new CLoginDlg();
		loginDlg->Create(m_hWnd, _T(""), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
		loginDlg->CenterWindow();
		loginDlg->ShowModal();

		CStdString strUserName = loginDlg->m_strUserName;
		CStdString strPassword = loginDlg->m_strPassword;
		delete loginDlg;

		if (!strUserName.IsEmpty() &&
			!strPassword.IsEmpty()) {
			Login(strUserName, strPassword);
		}
	}

	// 登录
	void BrowserDlg::Login(CStdString strUserName, CStdString strPassword) {
		char strUserNameUtf8[1024]{0};
		CStdString strUserNameUtf8UrlEncoded = CEncryptByMd5::UrlEncode(strUserName);

		CStdString strUrl(_T(""));
		strUrl.Format(
			_T("%s?username=%s&username1=%s&password=%s&key=%s&cpusn=%s&hdsn=%s&hdmodnum=%s&mac=%s&routemac=%s&bios=%s&version=%s&5I5JCV=%s&type=0"),
			GLOBALS::strLogInUrl.GetBuffer(),
			strUserNameUtf8UrlEncoded.GetBuffer(),
			strUserNameUtf8UrlEncoded.GetBuffer(),
			strPassword.GetBuffer(),
			GLOBALS::strMachineCode.GetBuffer(),
			GLOBALS::strCpuSn.GetBuffer(),
			GLOBALS::strHdSn.GetBuffer(),
			GLOBALS::strHdMod.GetBuffer(),
			GLOBALS::strLocalMac.GetBuffer(),
			GLOBALS::strRouteMac.GetBuffer(),
			GLOBALS::strBiosID.GetBuffer(),
			GLOBALS::strVersion.CompareNoCase(_T("500")) == 0 ? _T("3.3") : _T("3.2"),
			_T("336"));

		LoadURL(strUrl.GetBuffer());
	}

	// 登出
	void BrowserDlg::Logout() {
		CStdString strData(_T(""));
		strData.Format(
			_T("key=%s&cpusn=%s&hdsn=%s&hdmodnum=%s&mac=%s&routemac=%s&bios=%s&version=%s&type=1"),
			GLOBALS::strMachineCode.GetBuffer(),
			GLOBALS::strCpuSn.GetBuffer(),
			GLOBALS::strHdSn.GetBuffer(),
			GLOBALS::strHdMod.GetBuffer(),
			GLOBALS::strLocalMac.GetBuffer(),
			GLOBALS::strRouteMac.GetBuffer(),
			GLOBALS::strBiosID.GetBuffer(),
			GLOBALS::strVersion.CompareNoCase(_T("500")) == 0 ? _T("3.3") : _T("3.2"));

#if defined(_WIN32)
		WORD    version = MAKEWORD(2, 2);
		WSADATA data;
		// failed init network
		if (WSAStartup(version, &data) != 0) { return; }
#endif // _WIN32

		HTTPClient httpclient;
		HTTPRequest request;
		HTTPResponse response;
		std::string result;
		char* strLogOutUrlAnsi = CEncryptByMd5::Unicode2ANSI(GLOBALS::strLogOutUrl);
		char* strDataAnsi = CEncryptByMd5::Unicode2ANSI(strData);
		request.url = strLogOutUrlAnsi;
		request.setHead(HTTP_HEAD_REFERER, strLogOutUrlAnsi);
		bool httpresult = httpclient.post(
			request,
			response, 
			strDataAnsi,
			result);
		// if (httpresult) { }
		delete strLogOutUrlAnsi;
		delete strDataAnsi;
#if defined(_WIN32)
		WSACleanup();
#endif // _WIN32
	}
}
