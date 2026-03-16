#pragma once


// CInputDlg 对话框

class CInputDlg : public CDialog
{
	DECLARE_DYNAMIC(CInputDlg)

public:
	CInputDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CInputDlg();
	void Init(CString caption, CString prompt, CString show = "", bool bIsEmpty = false);
	CString m_caption;
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_INPUT_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString m_putdata;
	bool    m_bIsEmpty;
	virtual BOOL OnInitDialog();
	CString m_Prompt;
	afx_msg void OnBnClickedButtonOk();
	CButton m_ok;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
