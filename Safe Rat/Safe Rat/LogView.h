#if !defined(AFX_LOGVIEW_H__F5D71DAB_FD80_4820_BCAC_AB744D761143__INCLUDED_)
#define AFX_LOGVIEW_H__F5D71DAB_FD80_4820_BCAC_AB744D761143__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LogView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLogView view

class CLogView : public CEditView
{
public:
	CLogView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CLogView)
		virtual ~CLogView();
// Attributes
public:
 
	void AddToLog(CString str);
 
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLogView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CLogView)
	afx_msg void OnChange();
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOGVIEW_H__F5D71DAB_FD80_4820_BCAC_AB744D761143__INCLUDED_)
