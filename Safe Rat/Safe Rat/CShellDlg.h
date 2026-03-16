#pragma once
#include"HpTcpServer.h"

// CShellDlg 对话框

class CShellDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CShellDlg)

public:
	void OnReceiveComplete();
	CShellDlg(CWnd* pParent = NULL, CHpTcpServer* pIOCPServer = NULL, ClientContext *pContext = NULL);
	virtual ~CShellDlg();

	void AddKeyBoardData();
	void ResizeEdit();

	
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SHELL_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_edit;
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	virtual void PostNcDestroy();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnEnChangeEdit1();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
private:

	UINT m_nCurSel;						//游标位置
	UINT m_nReceiveLength;		
	HICON m_hIcon;						//图标
	BOOL m_bOnClose;					//是否关闭
	CHpTcpServer* m_iocpServer;
	ClientContext* m_pContext;
	CFont			font;
};
