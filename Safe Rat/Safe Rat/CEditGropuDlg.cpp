// CEditGropuDlg.cpp: 实现文件
//

#include "pch.h"
#include "Safe Rat.h"
#include "CEditGropuDlg.h"
#include "afxdialogex.h"
#include "Safe RatDlg.h"

extern CSafeRatDlg* g_pFrame;

// CEditGropuDlg 对话框

IMPLEMENT_DYNAMIC(CEditGropuDlg, CDialog)

CEditGropuDlg::CEditGropuDlg(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_EDIT_GROUP, pParent)
{

}

CEditGropuDlg::~CEditGropuDlg()
{
}

void CEditGropuDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_combo_group);
	DDX_Control(pDX, IDOK, m_ok);
}


BEGIN_MESSAGE_MAP(CEditGropuDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CEditGropuDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CEditGropuDlg 消息处理程序


BOOL CEditGropuDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString strTemp = "";
	CString strGroup = "";
	// TODO:  在此添加额外的初始化
	for (HTREEITEM hItem = g_pFrame->m_OnlineList.GetTreeCtrl().GetRootItem(); hItem != NULL; hItem = g_pFrame->m_OnlineList.GetTreeCtrl().GetNextSiblingItem(hItem))
	{
		strTemp = g_pFrame->m_OnlineList.GetTreeCtrl().GetItemText(hItem);
		int n = strTemp.Find(')');
		if (n > 0)strGroup = strTemp.Mid(n + 1);
		else strGroup = strTemp;
		m_combo_group.AddString(strGroup);
	}

	m_combo_group.SetCurSel(0);
	m_combo_group.SetTheme(xtpControlThemeUltraFlat);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CEditGropuDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	((CEdit*)GetDlgItem(IDC_COMBO1))->GetWindowText(m_NewGroup);
	if (m_NewGroup == "")
	{
		AfxMessageBox("输入不能为空!");
		return;
	}
	g_pFrame->m_OnlineList.AddFindGroup(m_NewGroup.GetBuffer(0));
	CDialog::OnOK();
}


BOOL CEditGropuDlg::PreTranslateMessage(MSG* pMsg)
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
