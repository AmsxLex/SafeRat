#pragma once
typedef struct
{
	CString strSerName;
	CString strSerDisPlayname;
	CString strSerDescription;
	CString strFilePath;
	CString strSerRunway;
	CString strSerState;

}SERVICEINFO;

// CServerInfoDlg 对话框

class CServerInfoDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CServerInfoDlg)

public:
	CServerInfoDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CServerInfoDlg();
	void SendToken(BYTE bToken);
	CHpTcpServer* m_iocpServer;
	ClientContext* m_pContext;
	SERVICEINFO m_ServiceInfo;
	HICON m_hIcon;

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SERVERINFO_DLG};
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CComboBox m_combox_runway;
	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton8();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
