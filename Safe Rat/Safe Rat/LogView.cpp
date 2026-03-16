// LogView.cpp : implementation file
//

#include"pch.h"
#include "safe rat.h"
#include "safe ratdlg.h"
#include "LogView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLogView

IMPLEMENT_DYNCREATE(CLogView, CEditView)

CLogView::CLogView()
{
}

CLogView::~CLogView()
{
 
}


BEGIN_MESSAGE_MAP(CLogView, CEditView)
	//{{AFX_MSG_MAP(CLogView)
	ON_CONTROL_REFLECT(EN_CHANGE, OnChange)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
	ON_WM_SIZE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLogView drawing

void CLogView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CLogView diagnostics

#ifdef _DEBUG
void CLogView::AssertValid() const
{
	CEditView::AssertValid();
}

void CLogView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CLogView message handlers

void CLogView::OnInitialUpdate() 
{
	CEditView::OnInitialUpdate();
	
	// TODO: Add your specialized code here and/or call the base class

}

void CLogView::OnChange() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CEditView::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	// TODO: Add your control notification handler code here
}
extern CSafeRatDlg* g_pFrame;
void CLogView::AddToLog(CString str)
{	

	if (!::IsWindow(m_hWnd)  )
	{
		return;
	}

	int	len = GetEditCtrl().GetWindowTextLength();

	GetEditCtrl().SetSel(len, len,FALSE);

	CTime time = CTime::GetCurrentTime(); ///构造CTime对象 
	CString strTime;
	str += "\r\n";
	strTime.Format("[%s]	%s", time.Format("%Y-%m-%d %H:%M:%S"), str);

	if (strTime.GetLength()+len >=  0xFFFFF)
	{
		len = 0;
		GetEditCtrl().SetWindowText("");
	}
 
	GetEditCtrl().ReplaceSel(strTime); 
}


BOOL CLogView::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if((pMsg->message == WM_KEYDOWN ))
	{
		return TRUE;
	}
	return CEditView::PreTranslateMessage(pMsg);
}


BOOL CLogView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此添加专用代码和/或调用基类

	return CEditView::PreCreateWindow(cs);
}


void CLogView::OnSize(UINT nType, int cx, int cy)
{
	CEditView::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	SCROLLINFO info;
	ZeroMemory(&info, sizeof(SCROLLINFO));
	GetScrollInfo(SB_VERT, &info, SIF_ALL);//获取原来的属性值
	info.nMin = 0;
	info.nMax = cy;

	SetScrollInfo(SB_VERT, &info, TRUE);//设置新属性
}
