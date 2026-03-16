#pragma once


// CServerDlg 对话框

class CServerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CServerDlg)

public:
	CServerDlg(CWnd* pParent = NULL, CHpTcpServer* pIOCPServer = NULL, ClientContext *pContext = NULL);
	virtual ~CServerDlg();
	void OnReceiveComplete();
	void InitView();
	void AdjustList();
	VOID ShowServiceList();
	void SendToken(BYTE bToken);
	void OpenInfoDlg();
	void SortColumn(int iCol, bool bAsc);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	void CreateSer();
	CString strMsgShow;
	HICON m_hIcon;						//图标
	BOOL m_bOnClose;					//是否关闭
	CHpTcpServer* m_iocpServer;
	ClientContext* m_pContext;
	CImageList     m_ImgList;
	// NOTE: the ClassWizard will add data members here
//}}AFX_DATA
	CXTHeaderCtrl   m_heades;
	CXTPStatusBar m_wndStatusBar;
	CXTPComboBox* m_wndComBox_search;
	UINT nServiceType;
	int             m_nSortedCol;
	bool            m_bAscending;

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SERVER_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CXTPListCtrl m_list;
	virtual BOOL OnInitDialog();
	virtual void PostNcDestroy();
	afx_msg void OnClose();
	afx_msg void OnNMRClickListServer(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMenuitemRefreshService();
	afx_msg LRESULT OnShowMessage(WPARAM wParam, LPARAM lParam); // 自定义消息
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNMDblclkListServer(NMHDR *pNMHDR, LRESULT *pResult);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CXTPComboBox m_;
	afx_msg void OnCbnSelchangeCombo1();
};
