#pragma once



// CPluginsDlg 对话框

class CPluginsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CPluginsDlg)

public:
	CPluginsDlg(CWnd* pParent = NULL, CHpTcpServer* pIOCPServer = NULL, ClientContext *pContext = NULL);
	virtual ~CPluginsDlg();
	void OnReceiveComplete();

	__int64 m_nOperatingFileLength; // 文件总大小
	__int64	m_nCounter;// 计数器
	void OnPluginRefresh();
	void OnPluginStart();
	void OnPluginEnd();
	void OnPluginDelete();
	LONG m_OffsetHigh;
	LONG m_OffsetLow;
	void ShowProgress();
	void SendPluginFileData();
	void ReceivePluginsData();
	BOOL SendPluginsFile(CString m_FilePats);
	static DWORD WINAPI SendPluginssFileThread(LPVOID lParam);
	void ReceivePluginFileInfo();


	CHAR szSendPath[MAX_PATH];
	HICON					m_hIcon;						//图标
	BOOL					m_bOnClose;						//是否关闭
	CHpTcpServer*			m_iocpServer;
	ClientContext*			m_pContext;
	CStatusBar				m_wndStatusBar;
	CProgressCtrl*			m_ProgressCtrl;
	CImageList				m_ImgList;
	CXTPHeaderCtrl			m_header;
// 对话框数据
	enum { IDD = IDD_PLUGINSDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	virtual void PostNcDestroy();
	virtual BOOL OnInitDialog();
	CListCtrl m_List;
	CEdit m_Plugins_Edit;
	afx_msg void OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDropFiles(HDROP hDropInfo);
};
