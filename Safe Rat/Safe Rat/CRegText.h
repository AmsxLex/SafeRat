#pragma once


// CRegText 对话框

class CRegText : public CDialogEx
{
	DECLARE_DYNAMIC(CRegText)

public:
	CRegText(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CRegText();
	bool EPath;
	bool EKey;
	bool isOK;
	bool isDWORD;             //是否为DWORD值
	void EnablePath();
	void EnableKey();
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REG_EDIT_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString m_key;
	CString m_value;
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnEnChangeEditValue();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
