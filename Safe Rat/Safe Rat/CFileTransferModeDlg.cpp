// CFileTransferModeDlg.cpp: 实现文件
//

#include "pch.h"
#include "Safe Rat.h"
#include "CFileTransferModeDlg.h"
#include "afxdialogex.h"


// CFileTransferModeDlg 对话框

IMPLEMENT_DYNAMIC(CFileTransferModeDlg, CDialogEx)

CFileTransferModeDlg::CFileTransferModeDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TRANSFERMODE_DLG, pParent)
{

}

CFileTransferModeDlg::~CFileTransferModeDlg()
{
}

void CFileTransferModeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_OVERWRITE, m_o);
	DDX_Control(pDX, IDC_OVERWRITE_ALL, m_p);
	DDX_Control(pDX, IDC_ADDITION, m_q);
	DDX_Control(pDX, IDC_ADDITION_ALL, m_r);
	DDX_Control(pDX, IDC_JUMP, m_s);
	DDX_Control(pDX, IDC_JUMP_ALL, m_t);
	DDX_Control(pDX, IDC_CANCEL, m_z);
}


BEGIN_MESSAGE_MAP(CFileTransferModeDlg, CDialogEx)
	ON_BN_CLICKED(IDC_OVERWRITE, &CFileTransferModeDlg::OnBnClickedOverwrite)
	ON_BN_CLICKED(IDC_OVERWRITE_ALL, &CFileTransferModeDlg::OnBnClickedOverwriteAll)
	ON_BN_CLICKED(IDC_ADDITION, &CFileTransferModeDlg::OnBnClickedAddition)
	ON_BN_CLICKED(IDC_ADDITION_ALL, &CFileTransferModeDlg::OnBnClickedAdditionAll)
	ON_BN_CLICKED(IDC_JUMP, &CFileTransferModeDlg::OnBnClickedJump)
	ON_BN_CLICKED(IDC_JUMP_ALL, &CFileTransferModeDlg::OnBnClickedJumpAll)
	ON_BN_CLICKED(IDC_CANCEL, &CFileTransferModeDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CFileTransferModeDlg 消息处理程序

void CFileTransferModeDlg::OnEndDialog(UINT id)
{
	// TODO: Add your control notification handler code here
	EndDialog(id);
}

BOOL CFileTransferModeDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CString	str;
	str.Format(_T("此文件夹已包含一个名为“%s”的文件"), m_strFileName);

	for (int i = 0; i < str.GetLength(); i += 120)
	{
		str.Insert(i, _T("\n"));
		i += 1;
	}

	SetDlgItemText(IDC_TIPS, str);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CFileTransferModeDlg::OnBnClickedOverwrite()
{
	EndDialog(IDC_OVERWRITE);
}


void CFileTransferModeDlg::OnBnClickedOverwriteAll()
{
	EndDialog(IDC_OVERWRITE_ALL);
}


void CFileTransferModeDlg::OnBnClickedAddition()
{
	EndDialog(IDC_ADDITION);
}


void CFileTransferModeDlg::OnBnClickedAdditionAll()
{
	EndDialog(IDC_ADDITION_ALL);
}


void CFileTransferModeDlg::OnBnClickedJump()
{
	EndDialog(IDC_JUMP);
}


void CFileTransferModeDlg::OnBnClickedJumpAll()
{
	EndDialog(IDC_JUMP_ALL);
}


void CFileTransferModeDlg::OnBnClickedCancel()
{
	EndDialog(IDC_CANCEL);
}


BOOL CFileTransferModeDlg::PreTranslateMessage(MSG* pMsg)
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
