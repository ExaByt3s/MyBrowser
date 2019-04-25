#include "LoginDialog.h"
#include "PUID.h"
#include "EncryptByMd5.h"

#include <memory>

// ��Ϣ֪ͨ
void CLoginDialog::Notify(TNotifyUI& msg)
{     
     // ��Ϊ�����Ϣ��ͨ���ؼ������ж����ĸ��ؼ�
    if ( msg.sType == DUI_MSGTYPE_CLICK) {
        if( msg.pSender == static_cast<CButtonUI*>(m_paintMgr.FindControl(_T("minbtn"))) ) {
            SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
		}
  
		else if( msg.pSender == static_cast<CButtonUI*>(m_paintMgr.FindControl(_T("closebtn"))) ) {
            // PostQuitMessage(0);
		}
  
		else if( msg.pSender == static_cast<CButtonUI*>(m_paintMgr.FindControl(_T("maxbtn"))) ) {
            ::IsZoomed(*this) ?
				SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0) : 
				SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0);
		}

		else if (msg.pSender == static_cast<CButtonUI*>(m_paintMgr.FindControl(_T("loginbtn")))) {
			// Login();
			// PostQuitMessage(0);
        }
	}
	else if (msg.sType == DUI_MSGTYPE_RETURN) {
		// // �س�
		// Login();
		// PostQuitMessage(0);
	}
	// else if (msg.sType == DUI_MSGTYPE_ESCAPE) {
	//	// ESC
	//	PostQuitMessage(0);
	// }
}
  
// ����������Ϣѭ���������麯��������Ϣ����
LRESULT CLoginDialog::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_KEYDOWN)
	{
		switch (wParam)
		{
		case VK_RETURN:                        // �س��� 
			// Login();
			// PostQuitMessage(0);
			break;
		case VK_ESCAPE:                        // ESC�� 
			// PostQuitMessage(0);
			break;
		default:
			break;
		}
	}

    // ��ʼ����Ϣӳ��MAP,����auto_ptrά��ָ��,static��ֻ֤����һ��
    static std::auto_ptr<MessageMap> customMessageMap(InitMessageMap());
  
    BOOL bHandled = TRUE;
    LRESULT lRes = 0;
  
    // ����Ϣ����Ϣӳ��map�н��в��� �ҵ���Ӧ����Ϣ������
    if ( customMessageMap->find(uMsg) != customMessageMap->end() ) {
        // typedef HRESULT (CLoginDialog::*CustomMsgHandler)(WPARAM, LPARAM, BOOL&);
        // ����ҵ�,������Ӧ����Ϣ��Ӧ����
        CustomMsgHandler handler = (*customMessageMap)[uMsg];
        // ͨ��this->(*handler)������Ϣ��Ӧ�����ĵ���
        lRes = (this->*handler)(wParam, lParam, bHandled);
        // ���bHandled����Trueû�б��޸���ô˵����Ϣ�Ѿ�������,����
		if (bHandled) { return lRes; }
    }
    // CPaintManagerUI����PaintManagerUI���д���
	// ��������˷���True,���򷵻�false������
	if (m_paintMgr.MessageHandler(uMsg, wParam, lParam, lRes)) { return lRes; }
    // ��󶪸�Ĭ�ϵ�windows��Ϣ������
    return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
}
  
// ��ʼ����Ϣѭ����Ӧ����Ϣ��Ӧ����
CLoginDialog::MessageMap* CLoginDialog::InitMessageMap()
{
    MessageMap* map = new MessageMap;
    (*map)[WM_CREATE] = &CLoginDialog::OnCreate;
    (*map)[WM_DESTROY] = &CLoginDialog::OnDestory;
    (*map)[WM_ERASEBKGND] = &CLoginDialog::OnErasebkgnd;
    (*map)[WM_NCPAINT] = &CLoginDialog::OnNcPaint;
    // ����������Ϣ��������ϵͳ������
    (*map)[WM_NCACTIVATE] = &CLoginDialog::OnNcActive;
    (*map)[WM_NCCALCSIZE] = &CLoginDialog::OnNcCalcSize;
    (*map)[WM_NCHITTEST] = &CLoginDialog::OnNcHit;
    (*map)[WM_SYSCOMMAND] = &CLoginDialog::OnSysCommand;
    return map;
}
  
// ���ڴ���ʱ��
HRESULT CLoginDialog::OnCreate( WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    // ��ȡ��ǰ���ڷ��
    LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
    styleValue &= ~WS_CAPTION;
    // ����STYLE
    ::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
    // ��ʼ��������Ⱦ��
	m_paintMgr.Init(m_hWnd);

	CDialogBuilder builder;
    // ͨ��xml�Լ���Ⱦ����Ⱦ����UI
    CControlUI* pRoot = builder.Create(_T("Skin\\logindlg.xml"), (UINT)0, NULL, &m_paintMgr);
    // ���ӽ���UI���Ի�������
	m_paintMgr.AttachDialog(pRoot);
    // ������Ϣ����,��Ϊʵ����INotifyUI�ӿ�
	m_paintMgr.AddNotifier(this);

	// ҵ����س�ʼ��
	// Init();

    return 0;
}
  
HRESULT CLoginDialog::OnDestory( WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    // �����˳���Ϣ
    // ::PostQuitMessage(0L);
    return 0;
}
  
// ��������
HRESULT CLoginDialog::OnErasebkgnd( WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    return 1;
}
  
HRESULT CLoginDialog::OnNcPaint( WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    return 0;
}
  
HRESULT CLoginDialog::OnNcActive( WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	if (::IsIconic(*this)) { bHandled = FALSE; }
    return (wParam == 0) ? TRUE : FALSE;
}
  
// �����������ô�ͻᵼ��DUILIB��ͣ����ϵͳ��Ϣ���д���
// ����ϵͳ������ �ƺ�������һ���������
HRESULT CLoginDialog::OnNcCalcSize( WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    return 0;
}
  
HRESULT CLoginDialog::OnNcHit( WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    return HTCLIENT;
}
  
// ϵͳ�����
LRESULT CLoginDialog::OnSysCommand(WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if( wParam == SC_CLOSE ) {
        // ::PostQuitMessage(0L);
        bHandled = TRUE;
        return 0;
    }
  
    BOOL bZoomed = ::IsZoomed(*this);
    LRESULT lRes = CWindowWnd::HandleMessage(WM_SYSCOMMAND, wParam, lParam);
  
    return 1L;
}  

// ���ڳ�ʼ������
void CLoginDialog::Init() {
	CPUID* hdInfoTool = new CPUID();

	// ��ȡCPU���к�
	// CStdString cpuSn = hdInfoTool->GetCpuSerialNumber();
	CStdString cpuSn = hdInfoTool->GetCpuSn();
	// ��ȡӲ�����к�
	CStdString hdSn = hdInfoTool->getHDSerialNum();
	// ��ȡӲ���ͺ�
	CStdString hdMod = hdInfoTool->getHDModelNum();
	// ��ȡ·��Mac
	CStdString routeMac = hdInfoTool->getRouteMac();
	// ��ȡ����Mac
	CStdString localMac = hdInfoTool->GetMacInfo();

	// Ӳ�����к�
	CTextUI* pTextHdSn = (CTextUI*)m_paintMgr.FindControl(_T("editHdSn"));
	if(NULL != pTextHdSn) {
	    pTextHdSn->SetText(hdSn);
	}

	// Ӳ���ͺ�
	CTextUI* pTextHdMod = (CTextUI*)m_paintMgr.FindControl(_T("editHdMod"));
	if (NULL != pTextHdMod) {
		pTextHdMod->SetText(hdMod);
	}

	// CPU���к�
	CTextUI* pTextCpuId = (CTextUI*)m_paintMgr.FindControl(_T("editCPUID"));
	if (NULL != pTextCpuId) {
		pTextCpuId->SetText(cpuSn);
	}

	// ·����MAC
	CTextUI* pTextRouteMac = (CTextUI*)m_paintMgr.FindControl(_T("editRouteMac"));
	if (NULL != pTextRouteMac) {
		pTextRouteMac->SetText(routeMac);
	}

	// ����MAC
	CTextUI* pTextLocalMac = (CTextUI*)m_paintMgr.FindControl(_T("editLocalMac"));
	if (NULL != pTextLocalMac) {
		pTextLocalMac->SetText(localMac);
	}
}

// ��¼
void CLoginDialog::Login() {
	TCHAR szFilePath[MAX_PATH];

	GetModuleFileName(NULL, szFilePath, _MAX_PATH);
	CStdString strRunDir = szFilePath;
	int nPos = strRunDir.ReverseFind('\\');
	if (nPos != -1) { strRunDir = strRunDir.Left(nPos + 1); }

	CStdString strIniFilePath = CStdString(strRunDir).append(_T("myie.ini"));

	// ��INI�ļ��ж�ȡ���ã���¼url
	CStdString strLogInUrl;
	::GetPrivateProfileString(
		_T("Settings"),
		_T("url"),
		_T(""),
		strLogInUrl.GetBuffer(MAX_PATH),
		MAX_PATH,
		strIniFilePath);

	// ��INI�ļ��ж�ȡ���ã��Ƿ���ʾ��ַ��
	CStdString strAddrInNewWin;
	::GetPrivateProfileString(
		_T("Bands"),
		_T("band3"),
		_T("0"),
		strAddrInNewWin.GetBuffer(MAX_PATH),
		MAX_PATH,
		strIniFilePath);

	// ��INI�ļ��ж�ȡ���ã��汾��
	CStdString strVersion;
	::GetPrivateProfileString(
		_T("Settings"),
		_T("Version"),
		_T("500"),
		strVersion.GetBuffer(MAX_PATH),
		MAX_PATH,
		strIniFilePath);

	// ��INI�ļ��ж�ȡ���ã������볤��ģʽ
	CStdString strMachineCodeMode;
	::GetPrivateProfileString(
		_T("Settings"),
		_T("MachineCodeMode"),
		_T("1"),
		strMachineCodeMode.GetBuffer(MAX_PATH),
		MAX_PATH,
		strIniFilePath);

	CTextUI* pTextUserName = (CTextUI*)m_paintMgr.FindControl(_T("editUserName"));
	CStdString strUserName = pTextUserName->GetText();

	CTextUI* pTextPassword = (CTextUI*)m_paintMgr.FindControl(_T("editPassword"));
	CStdString strPassword = pTextPassword->GetText();

	CStdString strChromePath = CStdString(strRunDir).append(_T("ChromePortable\\ChromePortable.exe"));
	CStdString strMachineCode = CalcMachineCode(strVersion, strMachineCodeMode);

	CStdString strUrl(_T(""));
	strUrl.Format(_T("%s%s?username=%s&password=%s&key=%s&version=%s&type=1"),
		strAddrInNewWin.CompareNoCase(_T("1")) == 0 ? _T("") : _T("--app="),
		strLogInUrl.GetBuffer(),
		strUserName.GetBuffer(),
		strPassword.GetBuffer(),
		strMachineCode.GetBuffer(),
		strVersion.CompareNoCase(_T("500")) == 0 ? _T("3.3") : _T("3.2"));

	ShellExecute(NULL,
		_T("open"),
		strChromePath,
		strUrl,
		NULL,
		SW_SHOWMAXIMIZED);
}

// ���������
CStdString CLoginDialog::CalcMachineCode(
	    CStdString version, 
	    CStdString machineCodeMode) {
	CPUID* hdInfoTool = new CPUID();

	// ��ȡCPU���к�
	CStdString cpuSn = hdInfoTool->GetCpuSn();
	// ��ȡӲ�����к�
	CStdString hdSn = hdInfoTool->getHDSerialNum();
	// ��ȡӲ���ͺ�
	CStdString hdMod = hdInfoTool->getHDModelNum();
	// ��ȡ·��Mac
	CStdString routeMac = hdInfoTool->getRouteMac();

	if (version.CompareNoCase(_T("500")) == 0) {
		// 3.3
		CStdString strHdInfo(_T(""));
		strHdInfo.append(hdSn);
		if (hdMod.GetLength() > 4) {
			strHdInfo.append(hdMod.Left(3));
		}

		CStdString strCpuSnHdInfo(_T(""));
		strCpuSnHdInfo.Format(_T("@%s@%s"),
			cpuSn.GetBuffer(),
			strHdInfo.GetBuffer());

		CStdString strMd5Part1 = CEncryptByMd5::Digest(strCpuSnHdInfo);
		CStdString strMd5Part2 = CEncryptByMd5::Digest(routeMac);
		return strMd5Part1.append(strMd5Part2);
	}
	else if (version.CompareNoCase(_T("400")) == 0) {
		// 3.2
		if (machineCodeMode.CompareNoCase(_T("1")) == 0) {
			// ��������ģʽ
			CStdString strHdInfo(_T(""));
			strHdInfo.append(hdSn);
			if (hdMod.GetLength() > 4) {
				strHdInfo.append(hdMod.Left(3));
			}

			CStdString strMachinePlainCode(_T(""));
			strMachinePlainCode.Format(_T("@%s%s"),
				strHdInfo.GetBuffer(),
				routeMac.GetBuffer());

			return CEncryptByMd5::Digest(strMachinePlainCode);
		}
		else {
			// �̻�����ģʽ
			CStdString strHdInfo(_T(""));
			strHdInfo.append(hdSn);
			if (hdMod.GetLength() > 4) {
				strHdInfo.append(hdMod.Left(3));
			}

			CStdString strMachinePlainCode(_T(""));
			strMachinePlainCode.Format(_T("@%s"),
				strHdInfo.GetBuffer());

			return CEncryptByMd5::Digest(strMachinePlainCode);
		}
	}
}