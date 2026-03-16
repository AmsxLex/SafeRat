// CRegText.cpp: 实现文件
//

#include "pch.h"
#include "Safe Rat.h"
#include "CRegText.h"
#include "afxdialogex.h"

// CRegText 对话框
IMPLEMENT_DYNAMIC(CRegText, CDialogEx)

CRegText::CRegText(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_REG_EDIT_DLG, pParent)
	, m_key(_T(""))
	, m_value(_T(""))
{
	EPath = false;
	EKey = false;
	isOK = false;
	isDWORD = false;
}

CRegText::~CRegText()
{
}

void CRegText::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_KEY, m_key);
	DDX_Text(pDX, IDC_EDIT_VALUE, m_value);
}


BEGIN_MESSAGE_MAP(CRegText, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT_VALUE, &CRegText::OnEnChangeEditValue)
END_MESSAGE_MAP()


// CRegText 消息处理程序


void CRegText::OnOK()
{
	// TODO: Add extra validation here
	UpdateData(true);

	CString restr = _T("数据不能为空！");
	if (m_key == _T("") && !EPath) {
		MessageBox(restr);
		isOK = false;
		return;
	}
	if (m_value == _T("") && !EKey)
	{
		MessageBox(restr);
		isOK = false;
		return;
	}
	isOK = true;
	CDialog::OnOK();
}


BOOL CRegText::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO: Add extra initialization here
	if (EPath == true)
	{
		EnablePath();
	}
	if (EKey == true)
	{
		EnableKey();
	}
	UpdateData(false);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
void CRegText::EnablePath()
{
	GetDlgItem(IDC_EDIT_KEY)->EnableWindow(false);
	UpdateData(false);
}
void CRegText::EnableKey()
{
	GetDlgItem(IDC_EDIT_VALUE)->EnableWindow(false);
	UpdateData(false);
}

void CRegText::OnEnChangeEditValue()
{
	// TODO: Add your control notification handler code here
	if (!isDWORD)
		return;
	UpdateData(true);
	int length = m_value.GetLength();

	for (int i = 0; i < length; i++)
	{
		TCHAR ch = m_value.GetAt(i);
		if (ch > 57 || ch < 48)
		{
			m_value.Delete(i);
			UpdateData(false);
		}
	}
	length = m_value.GetLength();
	if (length > 9)
	{                               //数据不能超范围
		m_value.Delete(length - 1);
		UpdateData(false);
	}
}


BOOL CRegText::PreTranslateMessage(MSG* pMsg)
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
