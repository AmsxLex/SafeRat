#pragma once

#include "AudioRender.h"
// CSpeakerDlg 对话框

class CSpeakerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CSpeakerDlg)

public:
	CSpeakerDlg(CWnd* pParent = NULL, CHpTcpServer* pIOCPServer = NULL, ClientContext *pContext = NULL);
	virtual ~CSpeakerDlg();
	void OnReceiveComplete();
	DWORD         m_nTotalRecvBytes;
	HICON m_hIcon;							//图标
	BOOL m_bOnClose;						//是否关闭
	CHpTcpServer* m_iocpServer;
	ClientContext* m_pContext;
	CAudioRenderImpl SetSpeakerDate;
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SPEAKER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual void PostNcDestroy();
	afx_msg void OnClose();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	CButton m_start;
	CButton m_m_stop;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
