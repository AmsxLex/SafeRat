#pragma once



// CConfig 对话框

class CConfig : public CDialogEx
{
	DECLARE_DYNAMIC(CConfig)

public:
	CConfig(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CConfig();

	HICON m_hIcon;
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_INI_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();

	CString port;
	CString m_max;
	CButton m_ok;
	afx_msg void OnBnClickedButton2();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
