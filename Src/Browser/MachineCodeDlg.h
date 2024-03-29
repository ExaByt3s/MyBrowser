#ifndef _CMACHINECODEDIALOG_H_
#define _CMACHINECODEDIALOG_H_

#pragma once

#include "UIlib.h"
#include "StdString.h"
#include "Globals.h"

#include <map>
using namespace DuiLib;

namespace Browser
{
	class CMachineCodeDlg : public CWindowWnd, public INotifyUI, public IMessageFilterUI
	{
	public:
		CMachineCodeDlg()
		{

		};
		virtual ~CMachineCodeDlg()
		{
			DestroyWindow(GetParent(this->GetHWND()));
		};
		LPCTSTR GetWindowClassName() const { return _T("UILoginFrame"); };
		UINT GetClassStyle() const { return UI_CLASSSTYLE_DIALOG; };
		void OnFinalMessage(HWND /*hWnd*/)
		{
			m_pm.RemovePreMessageFilter(this);
			delete this;
		};

		void Init() {
			CLabelUI* pLabelCompanyName = static_cast<CLabelUI*>(m_pm.FindControl(_T("lable_CompanyName_Content")));
			pLabelCompanyName->SetText(GLOBALS::strCompanyName);
		}

		void Notify(TNotifyUI& msg)
		{
			// 如为点击消息则通过控件名字判断是哪个控件
			if (msg.sType == DUI_MSGTYPE_CLICK) {
				if (msg.pSender == static_cast<CButtonUI*>(m_pm.FindControl(_T("confirmbtn")))) {
					CEditUI* pEditMachineCode = static_cast<CEditUI*>(m_pm.FindControl(_T("editMachineCode")));
					pEditMachineCode->SetText(GLOBALS::strMachineCodeEncoded);
				}
				else if (msg.pSender == static_cast<CButtonUI*>(m_pm.FindControl(_T("closebtn")))) {
					Close();
				}
			}
		}

		LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
			LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
			styleValue &= ~WS_CAPTION;
			::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

			m_pm.Init(m_hWnd);
			m_pm.AddPreMessageFilter(this);
			CDialogBuilder builder;
			CControlUI* pRoot = builder.Create(_T("Skin\\MachineCodeDlg.xml"), (UINT)0, NULL, &m_pm);
			ASSERT(pRoot && "Failed to parse XML");
			m_pm.AttachDialog(pRoot);
			m_pm.AddNotifier(this);

			Init();
			return 0;
		}

		LRESULT OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
			if (::IsIconic(*this)) { bHandled = FALSE; }
			return (wParam == 0) ? TRUE : FALSE;
		}

		LRESULT OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
			return 0;
		}

		LRESULT OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
			return 0;
		}

		LRESULT OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
			POINT pt; pt.x = GET_X_LPARAM(lParam); pt.y = GET_Y_LPARAM(lParam);
			::ScreenToClient(*this, &pt);

			RECT rcClient;
			::GetClientRect(*this, &rcClient);

			RECT rcCaption = m_pm.GetCaptionRect();
			if (pt.x >= rcClient.left + rcCaption.left &&
				pt.x < rcClient.right - rcCaption.right &&
				pt.y >= rcCaption.top &&
				pt.y < rcCaption.bottom) {
				CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(pt));
				if (pControl && _tcscmp(pControl->GetClass(), DUI_CTR_BUTTON) != 0) {
					return HTCAPTION;
				}
			}

			return HTCLIENT;
		}

		LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
			SIZE szRoundCorner = m_pm.GetRoundCorner();
			if (!::IsIconic(*this) && (szRoundCorner.cx != 0 || szRoundCorner.cy != 0)) {
				CDuiRect rcWnd;
				::GetWindowRect(*this, &rcWnd);
				rcWnd.Offset(-rcWnd.left, -rcWnd.top);
				rcWnd.right++; rcWnd.bottom++;
				HRGN hRgn = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom, szRoundCorner.cx, szRoundCorner.cy);
				::SetWindowRgn(*this, hRgn, TRUE);
				::DeleteObject(hRgn);
			}

			bHandled = FALSE;
			return 0;
		}

		LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			LRESULT lRes = 0;
			BOOL bHandled = TRUE;
			switch (uMsg) {
			case WM_CREATE:        lRes = OnCreate(uMsg, wParam, lParam, bHandled); break;
			case WM_NCACTIVATE:    lRes = OnNcActivate(uMsg, wParam, lParam, bHandled); break;
			case WM_NCCALCSIZE:    lRes = OnNcCalcSize(uMsg, wParam, lParam, bHandled); break;
			case WM_NCPAINT:       lRes = OnNcPaint(uMsg, wParam, lParam, bHandled); break;
			case WM_NCHITTEST:     lRes = OnNcHitTest(uMsg, wParam, lParam, bHandled); break;
			case WM_SIZE:          lRes = OnSize(uMsg, wParam, lParam, bHandled); break;
			default:
				bHandled = FALSE;
			}
			if (bHandled) { return lRes; }
			if (m_pm.MessageHandler(uMsg, wParam, lParam, lRes)) { return lRes; }
			return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
		}

		LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
		{
			if (uMsg == WM_KEYDOWN) {
				if (wParam == VK_RETURN) {
					CEditUI* pEditMachineCode = static_cast<CEditUI*>(m_pm.FindControl(_T("editMachineCode")));
					pEditMachineCode->SetText(GLOBALS::strMachineCodeEncoded);
					return true;
				}
				else if (wParam == VK_ESCAPE) {
					Close();
					return true;
				}

			}
			return false;
		}

	public:
		CPaintManagerUI m_pm;
	};
}
#endif // _CMACHINECODEDIALOG_H_