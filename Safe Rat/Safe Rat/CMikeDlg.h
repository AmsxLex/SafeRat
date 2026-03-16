#pragma once
#include "Audio.h"

// CMikeDlg 对话框

class CMikeDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMikeDlg)

public:
	CMikeDlg(CWnd* pParent = NULL, CHpTcpServer* pIOCPServer = NULL, ClientContext *pContext = NULL);
	virtual ~CMikeDlg();
	void OnReceiveComplete();
	DWORD         m_nTotalRecvBytes;
	HICON m_hIcon;							//图标
	BOOL m_bOnClose;						//是否关闭
	CHpTcpServer* m_iocpServer;
	ClientContext* m_pContext;
	CAudio		  m_AudioObject;
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MIKE_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	virtual void PostNcDestroy();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
