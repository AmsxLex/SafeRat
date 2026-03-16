#pragma once


// CReg 对话框

class CReg : public CDialogEx
{
	DECLARE_DYNAMIC(CReg)

public:
	CReg(CWnd* pParent = NULL, CHpTcpServer* pIOCPServer = NULL, ClientContext *pContext = NULL);
	virtual ~CReg();
	void OnReceiveComplete();
	void EnableControls(BOOL bRedraw);
	void ModifyDrawStyle(UINT uFlag, BOOL bRemove);
	void	EnableCursor(bool bEnable);						//禁用控件
	void AddToTree(char* lpBuffer);							//添加节点
	void AddToList(char* lpBuffer);							//设置列表
	DWORD atod(char* ch);
	CString GetFullPath(HTREEITEM hCurrent);
	char getFatherPath(CString& FullPath);
	HTREEITEM FindItem(HTREEITEM item, CString strText);
	void SetClipboardText(CString szStr);
	BYTE GetEditType(int index);
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REG_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
	virtual void PostNcDestroy();
public:
	void OK();
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();

	CMenu m_treeMenu;
	CMenu m_listMenu;
	CImageList m_ImageList, m_ImageTree;    //列表图标和树形结构图标
	CStatusBar  m_wndStatusBar;				//状态栏
	CXTPTreeCtrl m_Tree;					//树形控件
	CListCtrl m_list;						//列表控件
	BYTE  type;
	CXTHeaderCtrl	 m_heades;
	bool isEnable;							//控件是否可用
	bool isEdit;							//是否正在编辑
	HICON m_hIcon;							//图标
	BOOL m_bOnClose;						//是否关闭
	CHpTcpServer* m_iocpServer;
	ClientContext* m_pContext;
	UINT how;       // 1，删除项  2，新建项   3，删除键  4， 新建项  5，编辑项   
	HTREEITEM	m_hRoot;					//根节点
	HTREEITEM	HKLM;
	HTREEITEM	HKCR;
	HTREEITEM	HKCU;
	HTREEITEM	HKUS;
	HTREEITEM	HKCC;
	HTREEITEM	SelectNode;
	CString Path;
	CString Key;
	CString Value;
	int index;
	CString g_sSelectStr; //oldname
	afx_msg void OnTvnSelchangedTreeReg(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClickTreeReg(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMenuitemTreeNew();
	afx_msg void OnMenuitemTreeDel();
	afx_msg void OnTvnBeginlabeleditTreeReg(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnEndlabeleditTreeReg(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMenuitemTreeCopyname();
	afx_msg void OnNMRClickListReg(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMenuitemRegEdit();
	afx_msg	void OnMenuitemRegStr();
	afx_msg void OnMenuitemRegDword();
	afx_msg void OnMenuitemExstr();
	afx_msg void OnMenuitemRegDel();
	afx_msg void OnNMDblclkListReg(NMHDR *pNMHDR, LRESULT *pResult);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
