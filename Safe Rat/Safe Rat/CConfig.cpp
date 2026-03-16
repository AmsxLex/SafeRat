// CConfig.cpp: 实现文件
//

#include "pch.h"
#include "Safe Rat.h"
#include "CConfig.h"
#include "afxdialogex.h"
#include "Safe RatDlg.h"

extern CSafeRatDlg* g_pFrame;	//全局主窗口指针
// CConfig 对话框

IMPLEMENT_DYNAMIC(CConfig, CDialogEx)

CConfig::CConfig(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_INI_DLG, pParent)
	, port(_T(""))
	, m_max(_T(""))
{
	
	m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_SET));
}

CConfig::~CConfig()
{
}

void CConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Text(pDX, IDC_EDIT4, port);
	DDX_Text(pDX, IDC_EDIT5, m_max);
	DDX_Control(pDX, IDC_BUTTON2, m_ok);

}


BEGIN_MESSAGE_MAP(CConfig, CDialogEx)
	ON_WM_CLOSE()
ON_BN_CLICKED(IDC_BUTTON2, &CConfig::OnBnClickedButton2)
END_MESSAGE_MAP()


// CConfig 消息处理程序


BOOL CConfig::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	port = g_pFrame->m_IniFile.GetString("safe", "port", "6667");
	m_max = g_pFrame->m_IniFile.GetString("safe", "max", "1000");


	UpdateData(false);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CConfig::OnClose()
{

	CDialogEx::OnClose();
}



void CConfig::OnBnClickedButton2()
{
	UpdateData(TRUE);
	CDialog::OnOK();
}





BOOL CConfig::PreTranslateMessage(MSG* pMsg)
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
