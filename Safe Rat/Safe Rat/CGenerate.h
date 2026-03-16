#pragma once


// CGenerate 对话框

class CGenerate : public CDialogEx
{
	DECLARE_DYNAMIC(CGenerate)

public:
	CGenerate(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CGenerate();
	HICON m_hIcon;
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GENERATE_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CButton m_build;
	CString m_host;
	CString m_port;
	afx_msg void OnBnClickedButton1();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
