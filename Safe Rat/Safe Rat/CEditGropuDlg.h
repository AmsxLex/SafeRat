#pragma once


// CEditGropuDlg 对话框

class CEditGropuDlg : public CDialog
{
	DECLARE_DYNAMIC(CEditGropuDlg)

	CString m_NewGroup;

public:
	CEditGropuDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CEditGropuDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EDIT_GROUP };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CXTPComboBox m_combo_group;
	afx_msg void OnBnClickedOk();
	CButton m_ok;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
