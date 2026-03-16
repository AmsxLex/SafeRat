// CInputDlg.cpp: 实现文件
//

#include "pch.h"
#include "Safe Rat.h"
#include "CInputDlg.h"
#include "afxdialogex.h"


// CInputDlg 对话框

IMPLEMENT_DYNAMIC(CInputDlg, CDialog)

CInputDlg::CInputDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_INPUT_DLG, pParent)
	, m_putdata(_T(""))
	, m_Prompt(_T(""))
{

}

CInputDlg::~CInputDlg()
{
}

void CInputDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_INPUT, m_putdata);
	DDX_Text(pDX, IDC_STATIC_MSG, m_Prompt);
	DDX_Control(pDX, IDC_BUTTON_OK, m_ok);
}


BEGIN_MESSAGE_MAP(CInputDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_OK, &CInputDlg::OnBnClickedButtonOk)
END_MESSAGE_MAP()


// CInputDlg 消息处理程序


BOOL CInputDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWindowText(m_caption);
	UpdateData(FALSE);


	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

void CInputDlg::Init(CString caption, CString prompt, CString show, bool bIsEmpty)
{
	m_bIsEmpty = bIsEmpty;
	m_caption = caption;
	m_Prompt = prompt;
	m_putdata = show;
}

void CInputDlg::OnBnClickedButtonOk()
{
	UpdateData(TRUE);

	if (!m_bIsEmpty  && m_putdata.IsEmpty())
	{
		MessageBeep(0);
		return; // don't quit dialog!
	}

	CDialog::OnOK();
}


BOOL CInputDlg::PreTranslateMessage(MSG* pMsg)
{
	if (
		pMsg->message == WM_KEYDOWN		
		&&(	pMsg->wParam == VK_ESCAPE	 
		||	pMsg->wParam == VK_CANCEL	
		||	pMsg->wParam == VK_RETURN	
		))
		return TRUE;

	return CDialog::PreTranslateMessage(pMsg);
}
