#pragma once
#include "SEU_QQwry.h"

// CHostDlg 对话框

class CHostDlg : public CXTPResizeDialog
{
	DECLARE_DYNAMIC(CHostDlg)

public:
	CHostDlg(CWnd* pParent = NULL, CHpTcpServer* pIOCPServer = NULL, ClientContext *pContext = NULL);
	virtual ~CHostDlg();
	void OnReceiveComplete();

	void SortColumn(int iCol, bool bAsc);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

	CString __MakePriority(DWORD dwPriClass);
	void DeleteList();
	void reflush();
	CString UnicodeToAnsi(WCHAR* szStr);

	void ShowInfoList();
	void ShowProcessList();
	void ShowWindowsList();
	void ShowSoftWareList();
	void ShowIEHistoryList();
	void ShowHostsList();
	void ShowMstscList();
	void ShowStartup();
	void ShownNetList();
	void ShowUserInfoList();
	void ShowUserList();

	void ShowInfoList_menu();
	void ShowProcessList_menu();
	void ShowNetList_menu();
	void ShowWndowsList_menu();
	void ShowUserinfoList_menu();
	void ShowUserList_menu();
	void ShowSoftwareList_menu();
	void ShowHtmlList_menu();
	void ShowMstscList_menu();
	void ShowStartupList_menu();
	void ShowHostsList_menu();

	void SetClipboardText(CString& Data);


	CImageList				m_ImgList;					//图标
	CXTHeaderCtrl			m_heades;
	int						m_nSortedCol;
	bool					m_bAscending;
	HICON					m_hIcon;					//图标
	BOOL					m_bOnClose;					//是否关闭
	CHpTcpServer*			m_iocpServer;
	ClientContext*			m_pContext;
	SEU_QQwry				m_SQQwry;					//ip位置
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_HOST_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CXTPTabCtrl m_tab;
	CXTPListCtrl m_list;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	virtual void PostNcDestroy();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnClose();
	afx_msg void OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTcnSelchangingTab1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClickList1(NMHDR *pNMHDR, LRESULT *pResult);
};
