#pragma once
#include "afxwin.h"

// CClipboardDlg 对话框

class CClipboardDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CClipboardDlg)

public:
	CClipboardDlg(CWnd* pParent = NULL, CHpTcpServer* pIOCPServer = NULL, ClientContext *pContext = NULL);
	virtual ~CClipboardDlg();
	void OnReceiveComplete();
	HICON					m_hIcon;						//图标
	BOOL					m_bOnClose;						//是否关闭
	CHpTcpServer*			m_iocpServer;
	ClientContext*			m_pContext;
	void ReceiveClipboardData();
// 对话框数据
	enum { IDD = IDD_ClipboardDlg };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	virtual void PostNcDestroy();
	virtual BOOL OnInitDialog();
	CEdit m_Return_Data;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();

};
