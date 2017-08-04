#include "TCPClientSocket.h"
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#include "HNRC4.h"
#include "Log.h"

CTCPClientSocket::CTCPClientSocket()
{
	m_nRecvMaxNum = 5;
	m_bExit = false;
	for (int i = 0; i < m_nRecvMaxNum; i++)
	{
		char* pData = new char[MaxBuffLen];
		struct PackageRecv* pRecv = new struct PackageRecv;
		pRecv->pData = pData;
		pRecv->nLen = 0;
		pRecv->bRet = false;
		m_vectPackageRecv.push_back(pRecv);
	}
}


CTCPClientSocket::~CTCPClientSocket()
{
	for (int i = 0; i < m_nRecvMaxNum; i++)
	{
		auto pRecv = m_vectPackageRecv[i];
		delete[] pRecv->pData;
		delete pRecv;
		pRecv = nullptr;
	}
}

const char* CTCPClientSocket::GetErrorMsg()
{
	return m_strErrorMsg.c_str();
}

void CTCPClientSocket::CleanUpSocket()
{
	closesocket(m_sTCPSocket);
	closesocket(m_sUDPSocket);
	WSACleanup();
	m_sTCPSocket = INVALID_SOCKET;
	m_sUDPSocket = INVALID_SOCKET;
}

void CTCPClientSocket::SetError()
{
	char Buf[256];
	strerror_s(Buf, sizeof(Buf), GetLastError());
	m_strErrorMsg = Buf;
	CleanUpSocket();
}

void CTCPClientSocket::SetSecretKey(int nSecretKey)
{
	m_nSecretKey = nSecretKey;
}

int CTCPClientSocket::GetSecretKey()
{
	return m_nSecretKey;
}

void CTCPClientSocket::SetEncrypt(bool bEncrypt)
{
	m_bEncrypt = bEncrypt;
}

bool CTCPClientSocket::GetEncrypt()
{
	return m_bEncrypt;
}

void CTCPClientSocket::SetReserve(int nReserve)
{
	m_nReserve = nReserve;
}

int CTCPClientSocket::GetReserve()
{
	return m_nReserve;
}

bool CTCPClientSocket::Init()
{
	WSADATA wsaData;
	int nRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (nRet != 0)
	{
		SetError();
		return false;
	}

	m_sTCPSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_sTCPSocket == INVALID_SOCKET)
	{
		SetError();
		return false;
	}

	// 将socket设置为非阻塞模式
	unsigned long u1 = 1;
	nRet = ioctlsocket(m_sTCPSocket, FIONBIO, &u1);
	if (nRet != 0)
		return false;

	m_sUDPSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_sUDPSocket == INVALID_SOCKET)
	{
		SetError();
		return false;
	}

	return true;
}

bool CTCPClientSocket::Connect(std::string strIPAddr, int nPort)
{

	m_strIP = strIPAddr;
	m_nPort = nPort;

	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET;
	int nRet = inet_pton(AF_INET, strIPAddr.c_str(), &serverAddr.sin_addr.S_un.S_addr); 
	if (nRet < 0)
	{
		SetError();
		return false;
	}
	serverAddr.sin_port = htons(nPort);
	while (1)
	{
		nRet = connect(m_sTCPSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)); // 非阻塞模式 
		if (nRet == SOCKET_ERROR)
		{
			int r = WSAGetLastError();
			if (r == WSAEWOULDBLOCK || r == WSAEINVAL)
			{
				Sleep(100); // 延迟20ms
				continue;
			}
			else if (r == WSAEISCONN)//套接字已经连接
				break;
			else
			{
				DWORD err = GetLastError();
				std::cout << "connect server " << m_strIP.c_str() << " " << m_nPort << " error " << strerror(WSAGetLastError()) << std::endl;
				CLog::GetInstance()->Write("connect server: %s %d error: %s", m_strIP.c_str(), m_nPort, strerror(WSAGetLastError()));
				return false;
			}

		}
		else if (nRet == 0)
			break; // connect成功
	}
	m_bConnect = true;
	return true;
}

int CTCPClientSocket::SendData(void* pData, int nSize, int nMainID, int nAssistID)
{
	if (!m_bConnect) // 未登录时不发送消息
		return 0;
	struct PackageSend* pSend = new struct PackageSend;
	pSend->pData = new char[nSize + sizeof(NetMessageHead)];
	pSend->nLen = nSize + sizeof(NetMessageHead);
	memcpy(pSend->pData + sizeof(NetMessageHead), pData, nSize);
	NetMessageHead netHead;
	netHead.bMainID = nMainID;
	netHead.bAssistantID = nAssistID;
	netHead.bReserver = m_nReserve;
	netHead.bHandleCode = 0;
	netHead.uMessageSize = nSize + sizeof(NetMessageHead);
	memcpy(pSend->pData, &netHead, sizeof(NetMessageHead));

	if (m_bEncrypt) // 加密发送
	{
		HNRC4 rc4;
		rc4.init(g_chSecretKey, 64);
		if (pSend->nLen > sizeof(NetMessageHead))
			rc4.encrpyt((unsigned char*)pSend->pData + sizeof(NetMessageHead), pSend->nLen - sizeof(NetMessageHead));
	}

	Task task;
	task.func = std::bind(&CTCPClientSocket::SendDataPackage, this, std::placeholders::_1);
	task.arg = pSend;
	g_ThreadPool.AddTask(task);

	return 0;
}

void CTCPClientSocket::SendDataPackage(void* pData)
{
	struct PackageSend* pSend = (PackageSend*)pData;
	NetMessageHead* pHead = (NetMessageHead*)(pSend->pData);
	if (!pSend)
		return; // 发送失败
	int nRet = send(m_sTCPSocket, pSend->pData, pSend->nLen, 0);
	if (nRet == SOCKET_ERROR) // 非阻塞模式 发送失败 将任务再次发送
	{
		if (WSAGetLastError() == WSAEWOULDBLOCK) // 发送成功 已将数据发到内核区，但发送窗口已满，之后仍会从内核区发送到发送窗口区
		{
			if (pHead->bMainID != 1) // 过滤心跳消息的发送
				CLog::GetInstance()->Write("userid: %d send data mainid: %d, assistid: %d success", m_nUserID, pHead->bMainID, pHead->bAssistantID);
			delete[] pSend->pData;
			delete pSend;
			return;
		}
		else
		{
			// 发送失败
			m_bConnect = false;
			std::cout << "send error: " << strerror(WSAGetLastError());
			CLog::GetInstance()->Write("userid: %d send data mainid: %d, assistid: %d faild", m_nUserID, pHead->bMainID, pHead->bAssistantID);
		}
	}
	else
		if (pHead->bMainID != 1) // 过滤心跳消息的发送
			CLog::GetInstance()->Write("userid: %d send data mainid: %d, assistid: %d success", m_nUserID, pHead->bMainID, pHead->bAssistantID);
	delete[] pSend->pData; // 发送成功或错误都清除缓冲区
	delete pSend;
	return;
}

void CTCPClientSocket::RecvData(struct PackageRecv* pRecv)
{
	pRecv->bRet = true;
	Task task;
	task.func = std::bind(&CTCPClientSocket::RecvDataPackage, this, std::placeholders::_1);
	task.arg = pRecv;
	g_ThreadPool.AddTask(task);
	return;
}

void CTCPClientSocket::UDPRecvData(void* pBuff, int& nLen, struct PackageSend* pSend)
{
	pSend->pData = (char*)pBuff;
	pSend->nLen = 0;
	pSend->bRet = false;

	Task task;
	task.func = std::bind(&CTCPClientSocket::UDPRecvDataPackage, this, std::placeholders::_1);
	task.arg = pSend;
	g_ThreadPool.AddTask(task);
	return;
}

void CTCPClientSocket::RecvDataPackage(void* pData)
{
	if (!m_bConnect) // 未连接
		return;
	struct PackageRecv* pRecv = (struct PackageRecv*)pData;
	int nRecv = recv(m_sTCPSocket, pRecv->pData, MaxBuffLen, 0);
	if (nRecv == SOCKET_ERROR)
	{
		if (WSAGetLastError() == WSAEWOULDBLOCK) // 没有数据 等待再次接收
		{
			Task task;
			task.func = std::bind(&CTCPClientSocket::RecvDataPackage, this, std::placeholders::_1);
			task.arg = pRecv;
			g_ThreadPool.AddTask(task);
			return;
		}
		else // socket 错误
		{
			//std::cout << "socket close" << std::endl;
			CLog::GetInstance()->Write("userid: %d socket close", m_nUserID);
			memset(pRecv->pData, 0, MaxBuffLen);
			pRecv->bRet = true; // 不再投递接受消息
			m_bConnect = false;
			return;
		}
	}
	pRecv->nLen = nRecv; // 实际收到的数据
	// 处理数据
	if (m_bEncrypt) // 加密了数据则先解密
	{
		if (pRecv->nLen > sizeof(NetMessageHead))
		{
			HNRC4 rc4;
			rc4.init(g_chSecretKey, 64);
			rc4.decrypt((unsigned char*)(pRecv->pData)+sizeof(NetMessageHead), nRecv - sizeof(NetMessageHead));
		}
	}
	HandleData(pRecv->pData, nRecv);
	memset(pRecv->pData, 0, MaxBuffLen); // 清空缓冲区数据
	pRecv->bRet = false; // 已经处理了数据
	return;
}

void CTCPClientSocket::UDPRecvDataPackage(void* pData)
{
	struct PackageRecv* pRecv = (struct PackageRecv*)pData;

	char buff[MaxBuffLen] = { 0x00 };
	sockaddr_in sockAddr;
	sockAddr.sin_family = AF_INET;
	int nRet = inet_pton(AF_INET, m_strIP.c_str(), &sockAddr.sin_addr.S_un.S_addr);
	if (nRet < 0)
	{
		SetError();
		return;
	}
	sockAddr.sin_port = htons(m_nPort);
	int nLen = 0;
	recvfrom(m_sUDPSocket, pRecv->pData, MaxBuffLen, 0, (sockaddr*)&sockAddr, &nLen);
	// 收到消息 如房间人数已满
	pRecv->bRet = true;
}

void CTCPClientSocket::HandleData()
{
	if (m_bConnect && !m_bExit) // m_bExit时不再投递
	{
		for (auto& pRecv : m_vectPackageRecv)
		{
			if (!pRecv->bRet) // 空闲的
			{
				if (m_bConnect)
					RecvData(pRecv);
				else
					break; // 不再投递接收请求
			}
		}
	}
}

void CTCPClientSocket::HandleData(void* pData, int nLen) // 处理接收的数据
{
	NetMessageHead* pNetHead = (NetMessageHead*)pData;
	if (pNetHead->bMainID == MDM_CONNECT) // 连接消息
	{
		HandleConnectMessage((char*)pData + sizeof(NetMessageHead), MDM_CONNECT, pNetHead->bAssistantID);
	}
	else
	{
		//std::cout << "recv message: " << pNetHead->bMainID << " " << pNetHead->bAssistantID << std::endl;
		switch (m_Type)
		{
		case LoginMG::LOGIN_M: // M服务器消息
		{
			if (pNetHead->bMainID == MDM_GP_LOGON)
			{
				if (pNetHead->bAssistantID == ASS_GP_LOGON_SUCCESS)
					//std::cout << m_nUserID << " login success" << std::endl;
					CLog::GetInstance()->Write("userid: %d login Mserver success", m_nUserID);
				else
				{
					//std::cout << m_nUserID << " login error" << std::endl;
					CLog::GetInstance()->Write("userid: %d login Mserver faild, mainid: %d, assistiid:%d, handlecode: %d", m_nUserID, pNetHead->bMainID, pNetHead->bAssistantID, pNetHead->bHandleCode);
					m_bConnect = false; // 连接失败时不再投递接收消息
				}
				return;
			}
			else
			{
				if ((pNetHead->bMainID != 119 && pNetHead->bAssistantID != 1) || (pNetHead->bMainID != 1 && pNetHead->bAssistantID != 1)) // 过滤119和心跳消息
					CLog::GetInstance()->Write("userid: %d recv data from Mserver mainid: %d, assistid: %d, handlecode: %d", m_nUserID, pNetHead->bMainID, pNetHead->bAssistantID, pNetHead->bHandleCode);
				return;
			}
		}
		case LoginMG::LOGIN_G: // G服务器消息
		{
			if (pNetHead->bMainID == MDM_GR_LOGON)
			{
				if (pNetHead->bAssistantID == ASS_GR_LOGON_SUCCESS)
					//std::cout << m_nUserID << " login success" << std::endl;
					CLog::GetInstance()->Write("userid: %d login Gserver success", m_nUserID);
				else
					//std::cout << m_nUserID << " login error" << std::endl;
					CLog::GetInstance()->Write("userid: %d login Gserver faild", m_nUserID);
				return;
			}
			else
			{
				if ((pNetHead->bMainID != 119 && pNetHead->bAssistantID != 1) || (pNetHead->bMainID != 1 && pNetHead->bAssistantID != 1)) // 过滤119和心跳消息
					CLog::GetInstance()->Write("userid: %d recv data from Gserver mainid: %d, assistid: %d, handlecode: %d", m_nUserID, pNetHead->bMainID, pNetHead->bAssistantID, pNetHead->bHandleCode);
				return;
			}
		}
		}
	}
}

void CTCPClientSocket::HandleConnectMessage(void * pData, int nMainID, int nAssistID)
{
	if (nAssistID == ASS_CONNECT_SUCCESS) // 连接成功
	{
		MSG_S_ConnectSuccess* pConnect = (MSG_S_ConnectSuccess*)(pData);
		int nReserve = (pConnect->i64CheckCode - m_nSecretKey) / 23;
		m_nReserve = nReserve;

		// 连接成功 发送心跳消息
		SendData(NULL, 0, MDM_CONNECT, ASS_NET_TEST);
		return;
	}
	if (nAssistID == ASS_NET_TEST) // 心跳包
	{
		//std::cout << "recv herat package" << std::endl;
		SendData(NULL, 0, MDM_CONNECT, ASS_NET_TEST); // 发送心跳 保持连接
	}
}

void CTCPClientSocket::SetType(LoginMG mg)
{
	m_Type = mg;
}

LoginMG CTCPClientSocket::GetType()
{
	return m_Type;
}

void CTCPClientSocket::SetUserID(int nUserID)
{
	m_nUserID = nUserID;
}

int CTCPClientSocket::GetUserID()
{
	return m_nUserID;
}

void CTCPClientSocket::CloseSocket()
{
	CleanUpSocket();
	if (m_sTCPSocket == INVALID_SOCKET)
		CLog::GetInstance()->Write("userid: %d socket close", m_nUserID);
	m_bConnect = false;
}

void CTCPClientSocket::SetExit(bool bExit)
{
	m_bExit = bExit;
}