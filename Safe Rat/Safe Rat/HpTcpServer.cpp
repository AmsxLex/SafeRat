#include "pch.h"
#include "HpTcpServer.h"
#include "Safe RatDlg.h"
#include "LogView.h"
#include "../../common/zlib.h"
#pragma comment(lib, "zlib.lib")
#ifdef _DEBUG
#pragma comment(lib, "HPSocket_D.lib")
#else
#pragma comment(lib, "HPSocket.lib")
#endif
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


 extern CSafeRatDlg* g_pFrame;			//全局主窗口指针

CHpTcpServer::CHpTcpServer(void) :m_TcpServer(this)
{

	m_bIsOver = false;
	//赋值自己的指针
	m_spThis = this;
}

CHpTcpServer::~CHpTcpServer(void)
{
	m_bIsOver = true;

	if (m_TcpServer->GetState() != SS_STOPPED)
		m_TcpServer->Stop();

	m_TcpServer->Wait(-1);

}

BOOL CHpTcpServer::Initialize(NOTIFYPROC pNotifyProc, CSafeRatDlg* pFrame, int nMaxConnections, int nPort,BOOL ipv6)
{
	m_pNotifyProc = pNotifyProc;
	m_pFrame = pFrame;


	//最大连接数
	m_TcpServer->SetMaxConnectionCount(nMaxConnections);

	//最大包 hp最大4m
	m_TcpServer->SetMaxPackSize(4194303);

	m_TcpServer->SetPackHeaderFlag(1023);

	//无延迟
	m_TcpServer->SetSendPolicy(SP_DIRECT);
	m_TcpServer->SetNoDelay(TRUE);


	//开始监听
	return m_TcpServer->Start(_T("0.0.0.0"), nPort);
}

//准备监听 绑定监听地址前触发
EnHandleResult CHpTcpServer::OnPrepareListen(ITcpServer* pSender, SOCKET soListen)
{

	return HR_OK;
}



//接受连接 客户端连接请求到达时触发
EnHandleResult CHpTcpServer::OnAccept(ITcpServer* pSender, CONNID dwConnID, UINT_PTR soClient)
{


	//创建一个结构
	ClientContext* pContext = new ClientContext;
	ASSERT(pContext);
	//初始化为0
	if (pContext != NULL) ZeroMemory(pContext, sizeof(ClientContext));

	if (pContext == NULL)
		return HR_OK;

	//赋值 dwConnID 唯一ID
	pContext->m_dwConnID = dwConnID;
	//赋值IP
	int len = sizeof(pContext->m_RemoteIP);
	USHORT usPort;
	m_TcpServer->GetRemoteAddress(dwConnID, pContext->m_RemoteIP, len, usPort);

	//绑定附加数据
	m_TcpServer->SetConnectionExtra(dwConnID, pContext);

	return HR_OK;

}

//数据发送成功后触发
EnHandleResult CHpTcpServer::OnSend(ITcpServer* pSender, CONNID dwConnID, const BYTE* pData, int iLength)
{

	return HR_OK;
}


// 数据到达（PUSH/PACK） 接收到数据时触发
EnHandleResult CHpTcpServer::OnReceive(ITcpServer* pSender, CONNID dwConnID, const BYTE* pData, int iLength)
{

	try
	{

		ClientContext* pContext = NULL;
		m_TcpServer->GetConnectionExtra(dwConnID, (PVOID*)&pContext);

		if (pContext == nullptr)
			return HR_ERROR;

		pContext->m_CompressionBuffer.Write((PBYTE)pData, iLength);

		// 检测数据大小
		if (pContext->m_CompressionBuffer.GetBufferLen() > 8)
		{

			//获取数据大小
			int nSize = 0;
			CopyMemory(&nSize, pContext->m_CompressionBuffer.GetBuffer(0), sizeof(int));

			//判断数据是否全部接受了
			if (nSize && ((int)pContext->m_CompressionBuffer.GetBufferLen()) >= nSize)
			{
				int nUnCompressLength = 0;
				//读取总长度
				pContext->m_CompressionBuffer.Read((PBYTE)&nSize, 4);
				//读取解压后的数据
				pContext->m_CompressionBuffer.Read((PBYTE)&nUnCompressLength, 4);

				//生成空间用于保存解压数据
				PBYTE _pData = new BYTE[nUnCompressLength];
				if(_pData == NULL)
				{
					pContext->m_CompressionBuffer.ClearBuffer();
					return HR_OK;;
				}
				//解压
				if (uncompress(_pData, (uLongf*)&nUnCompressLength, pContext->m_CompressionBuffer.GetBuffer(), pContext->m_CompressionBuffer.GetBufferLen()) == Z_OK)
				{
					//清空解压缓冲区
					pContext->m_DeCompressionBuffer.ClearBuffer();
					//写到解压缓冲区
					pContext->m_DeCompressionBuffer.Write(_pData, nUnCompressLength);
					//回调
					m_spThis->m_pNotifyProc(m_spThis->m_pFrame, pContext, NC_RECEIVE_COMPLETE);
				}
				
				//清空压缩数据
				pContext->m_CompressionBuffer.ClearBuffer();

				SAFE_DELETE_AR(_pData);
			}
		}
	}
	catch (...)
	{
		
	}
	return HR_OK;
}

// 连接关闭
EnHandleResult CHpTcpServer::OnClose(ITcpServer* pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode)
{

	ClientContext* pContext = NULL;
	//获得附加数据
	m_TcpServer->GetConnectionExtra(dwConnID, (PVOID*)&pContext);
	if(!pContext)  return HR_ERROR;
	//调用回调
	m_spThis->m_pNotifyProc(m_spThis->m_pFrame, pContext, NC_CLIENT_DISCONNECT);

	Sleep(30);

	//清除附加数据
	SAFE_DELETE(pContext);

	return HR_OK;
}

//关闭通信组件 通信组件停止后触发
EnHandleResult CHpTcpServer::OnShutdown(ITcpServer* pSender)
{
	return HR_OK;
}

BOOL CHpTcpServer::GeRemoteIP(CONNID id, char *szAddress, int iAddressLen, USHORT uPort)
{
	return  m_TcpServer->GetRemoteAddress(id, szAddress, iAddressLen, uPort);
}

int CHpTcpServer::Send(ClientContext* pContext, LPBYTE lpData, UINT nSize)
{

	if (pContext == NULL || nSize <= 0 || lpData == NULL || m_bIsOver) return -1;

	//计算最大空间
	unsigned long	destLen = (unsigned long)((double)nSize * 1.001 + 12);

	//申请保存压缩的数据
	LPBYTE			pDest = new BYTE[destLen];
	if (pDest == NULL)
		return 0;

	//压缩数据
	if (compress(pDest, &destLen, lpData, nSize) != Z_OK)
	{
		delete[] pDest;
		return 0;
	}

	//清空内存
	pContext->m_WriteBuffer.ClearBuffer();

	//计算数据包
	UINT nBufLen = destLen + 8;

	//写总长度
	pContext->m_WriteBuffer.Write((PBYTE)&nBufLen,sizeof(nBufLen));

	//写入原始长度
	pContext->m_WriteBuffer.Write((PBYTE)&nSize,sizeof(nSize));

	//写入数据
	pContext->m_WriteBuffer.Write((PBYTE)pDest,destLen);

	//分块发送
	int nRet = SendWithSplit(pContext->m_dwConnID, pContext->m_WriteBuffer.GetBuffer(), pContext->m_WriteBuffer.GetBufferLen(), MAX_SEND_BUFFER);

	return nRet;
}

void CHpTcpServer::Shutdown()
{
	m_TcpServer->Stop();
}

BOOL CHpTcpServer::Disconnect(ClientContext* p_ClientContext)
{
	return m_TcpServer->Disconnect(p_ClientContext->m_dwConnID, TRUE);
}

BOOL CHpTcpServer::SendWithSplit(CONNID dwConnID, LPBYTE lpData, UINT nSize, UINT nSplitSize)
{
	int			nSend = 0;
	UINT		nSendRetry = 0;

	if (nSize >= nSplitSize)
	{
		UINT i = 0;
		nSendRetry = nSize / nSplitSize;
		for (i = 0; i < nSendRetry; i++)
		{
			m_TcpServer->Send(dwConnID, lpData, nSplitSize);
			lpData += nSplitSize;
			nSend += nSplitSize;
		}
		if (nSize - nSend < nSplitSize)
		{
			if (nSize - nSend > 0)
				m_TcpServer->Send(dwConnID, lpData, nSize - nSend);
		}
	}
	else
		m_TcpServer->Send(dwConnID, lpData, nSize);

	return  TRUE;
}



