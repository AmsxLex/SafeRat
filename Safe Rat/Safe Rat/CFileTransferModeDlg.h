#pragma once


// CFileTransferModeDlg 对话框

class CFileTransferModeDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CFileTransferModeDlg)

public:
	CFileTransferModeDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CFileTransferModeDlg();
	CString m_strFileName;
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TRANSFERMODE_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg	void OnEndDialog(UINT id);
	CButton m_o;
	CButton m_p;
	CButton m_q;
	CButton m_r;
	CButton m_s;
	CButton m_t;
	CButton m_z;
	afx_msg void OnBnClickedOverwrite();
	afx_msg void OnBnClickedOverwriteAll();
	afx_msg void OnBnClickedAddition();
	afx_msg void OnBnClickedAdditionAll();
	afx_msg void OnBnClickedJump();
	afx_msg void OnBnClickedJumpAll();
	afx_msg void OnBnClickedCancel();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
